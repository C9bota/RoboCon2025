// 必要ライブラリ
#include <Arduino.h>
#include <SdFat.h>           // SdFat ライブラリ（SdFat.h）
extern "C" {
  #include "driver/i2s.h"   // ESP32 の i2s ドライバ
}

// ---------- ユーザー指定ピン ----------
const int SD_MISO = 19;
const int SD_MOSI = 23;
const int SD_SCK  = 18;
const int SD_CS   = 5;

const int I2S_BCK_PIN  = 16; // Bit clock (BCLK)
const int I2S_LRCK_PIN = 17; // Word select (LRCLK)
const int I2S_DATA_PIN = 26; // Data out (DIN -> to PCM5102A DIN)

// ---------- WAV / 再生設定 ----------
const char WAV_FILENAME[] = "/music00.wav";
const uint32_t EXPECTED_SAMPLE_RATE = 44100;
const uint16_t EXPECTED_BITS_PER_SAMPLE = 16;
const uint16_t EXPECTED_CHANNELS = 2;

// バッファサイズ（バイト） - 必要に応じて調整
const size_t BUFFER_SIZE = 4096;


// ---------- SdFat オブジェクト ----------
SdFat sd;
SdFile wavFile;

// ---------- ヘルパー関数: リトルエンディアン読み取り ----------
uint32_t readLE32(const uint8_t *buf) {
  return (uint32_t)buf[0] | ((uint32_t)buf[1] << 8) | ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
}
uint16_t readLE16(const uint8_t *buf) {
  return (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
}

// ---------- WAV ヘッダ解析 ----------
bool parseWavHeader(SdFile &file, uint32_t &dataStart, uint32_t &dataSize,
                    uint16_t &channels, uint32_t &sampleRate, uint16_t &bitsPerSample) {
  // ヘッダの最初 12 バイトを読む（RIFF, overall size, WAVE）
  uint8_t header12[12];
  if (file.read(header12, 12) != 12) return false;
  if (memcmp(header12, "RIFF", 4) != 0) {
    Serial.println("Not a RIFF file");
    return false;
  }
  if (memcmp(header12 + 8, "WAVE", 4) != 0) {
    Serial.println("Not a WAVE file");
    return false;
  }

  // 以降のチャンクを順に解析して "fmt " と "data" を見つける
  bool fmtFound = false;
  bool dataFound = false;
  uint8_t chunkHeader[8];

  while (file.read(chunkHeader, 8) == 8) {
    uint32_t chunkId = readLE32(chunkHeader); // 4 bytes (but we'll compare as ASCII)
    uint32_t chunkSize = readLE32(chunkHeader + 4);

    // チャンクIDを ASCII 文字列でチェック
    if (memcmp(chunkHeader, "fmt ", 4) == 0) {
      // fmt チャンク読み出し（size は通常 16 or >16）
      uint8_t *fmtBuf = (uint8_t*)malloc(chunkSize);
      if (!fmtBuf) {
        Serial.println("malloc failed");
        return false;
      }
      if (file.read(fmtBuf, chunkSize) != (int)chunkSize) {
        free(fmtBuf);
        Serial.println("Failed to read fmt chunk");
        return false;
      }
      uint16_t audioFormat = readLE16(fmtBuf + 0);
      channels = readLE16(fmtBuf + 2);
      sampleRate = readLE32(fmtBuf + 4);
      bitsPerSample = readLE16(fmtBuf + 14);

      free(fmtBuf);

      if (audioFormat != 1) { // 1 == PCM (uncompressed)
        Serial.print("Unsupported audio format: ");
        Serial.println(audioFormat);
        return false;
      }
      fmtFound = true;
    } else if (memcmp(chunkHeader, "data", 4) == 0) {
      // data チャンクの開始位置を取得（現在のファイル位置が data 本体直前）
      dataStart = (uint32_t)file.curPosition(); // SdFat の curPosition() を利用
      dataSize = chunkSize;
      // ファイル読み出し位置はすでに data 本体の先頭（open によって curPos が ）
      dataFound = true;
      break; // data は最後に来ることが多い — 実装上は break して再生に移行
    } else {
      // 他のチャンクはスキップ（チャンクサイズが奇数の場合、パディングバイトあり）
      uint32_t skipBytes = chunkSize;
      if (skipBytes & 1) skipBytes++; // WORD アライン
      if (!file.seekCur(skipBytes)) {
        Serial.println("Failed to skip unknown chunk");
        return false;
      }
    }
  }

  return fmtFound && dataFound;
}

// ---------- I2S 初期化 ----------
void i2sInit() {
  i2s_config_t i2s_config;
  memset(&i2s_config, 0, sizeof(i2s_config));
  i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
  i2s_config.sample_rate = EXPECTED_SAMPLE_RATE;
  i2s_config.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
  i2s_config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT; // stereo

  // ★ 修正版 ★
  i2s_config.communication_format = I2S_COMM_FORMAT_STAND_I2S;

  i2s_config.intr_alloc_flags = 0;
  i2s_config.dma_buf_count = 4;
  i2s_config.dma_buf_len = 1024;
  i2s_config.use_apll = false;
  i2s_config.tx_desc_auto_clear = true;

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);

  i2s_pin_config_t pin_config;
  pin_config.bck_io_num = I2S_BCK_PIN;
  pin_config.ws_io_num = I2S_LRCK_PIN;
  pin_config.data_out_num = I2S_DATA_PIN;
  pin_config.data_in_num = I2S_PIN_NO_CHANGE;

  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_set_sample_rates(I2S_NUM_0, EXPECTED_SAMPLE_RATE);
}

// ---------- WAV 再生ルーチン ----------
bool playWavFile(SdFile &file) {
  // 先頭に戻す
  file.seekSet(0);

  uint32_t dataStart = 0;
  uint32_t dataSize = 0;
  uint16_t channels = 0;
  uint32_t sampleRate = 0;
  uint16_t bitsPerSample = 0;

  if (!parseWavHeader(file, dataStart, dataSize, channels, sampleRate, bitsPerSample)) {
    Serial.println("Failed to parse WAV header or unsupported format");
    return false;
  }

  // 仕様チェック（要求されたものと一致するか）
  Serial.print("WAV channels: "); Serial.println(channels);
  Serial.print("WAV sampleRate: "); Serial.println(sampleRate);
  Serial.print("WAV bitsPerSample: "); Serial.println(bitsPerSample);
  Serial.print("Data size bytes: "); Serial.println(dataSize);
  if (channels != EXPECTED_CHANNELS || sampleRate != EXPECTED_SAMPLE_RATE || bitsPerSample != EXPECTED_BITS_PER_SAMPLE) {
    Serial.println("WAV file format does not match expected 16bit/44.1kHz/stereo");
    // ここではエラーとして止めるが、必要なら resampling/format conversion を実装可能
    // 続行するならコメントアウトして再生を試みる
    return false;
  }

  // 再生位置を dataStart にセット
  if (!file.seekSet(dataStart)) {
    Serial.println("Failed to seek to data chunk");
    return false;
  }

  // バッファを確保
  uint8_t *buffer = (uint8_t*)malloc(BUFFER_SIZE);
  if (!buffer) {
    Serial.println("Failed to allocate buffer");
    return false;
  }

  size_t bytesRemaining = dataSize;
  while (bytesRemaining > 0) {
    size_t toRead = (bytesRemaining > BUFFER_SIZE) ? BUFFER_SIZE : bytesRemaining;
    int32_t r = file.read(buffer, toRead);
    if (r <= 0) {
      Serial.println("Read error or EOF");
      break;
    }
    bytesRemaining -= r;

    // i2s_write に渡す
    size_t bytesWritten = 0;
    esp_err_t ret = i2s_write(I2S_NUM_0, buffer, r, &bytesWritten, portMAX_DELAY);
    if (ret != ESP_OK) {
      Serial.print("i2s_write error: "); Serial.println((int)ret);
      free(buffer);
      return false;
    }
    // bytesWritten が読み込んだ r と等しいかを確認する場合はここでチェック可能
  }

  free(buffer);
  return true;
}

// ---------- setup / loop ----------
void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("Starting SD + I2S WAV player...");
  // SPI 初期化（ユーザー指定のピンで）
  // SPI.begin(SCK, MISO, MOSI, SS) : 第4引数は CS の初期ピン（使う場合）
  // ① SPIピンを設定して初期化
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  // ② SdFat を初期化（2引数版）
  if (!sd.begin(SD_CS, SD_SCK_MHZ(10))) {   // 20MHzにしておくのが安全
      Serial.println("SdFat begin failed. Check SD wiring and card.");
      Serial.println(sd.card()->errorCode());
      while (1) { delay(500); }
  }
  Serial.println("SdFat initialized");

  // WAV ファイルを開く
  if (!wavFile.open(WAV_FILENAME, O_READ)) {
    Serial.print("Failed to open ");
    Serial.println(WAV_FILENAME);
    while (1) { delay(500); }
  }
  Serial.print("Opened "); Serial.println(WAV_FILENAME);

  // I2S 初期化
  i2sInit();
  Serial.println("I2S initialized");

  // 再生を開始（setup 内で完了するまでブロック）
  Serial.println("Starting playback...");
  bool ok = playWavFile(wavFile);
  if (!ok) {
    Serial.println("Playback failed or stopped due to error.");
  } else {
    Serial.println("Playback finished.");
  }

  // 再生後はファイルを閉じる
  wavFile.close();
  Serial.println("WAV file closed.");
  // ここで必要なら i2s_driver_uninstall(I2S_NUM_0);
}

void loop() {
  // 再生は setup() で行っているため loop() は空でも良い
  delay(1000);
}