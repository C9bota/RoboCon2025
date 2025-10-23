#include "driver/i2s.h"
#include "aquestalk.h"
uint32_t workbuf[AQ_SIZE_WORKBUF];
const int i2s_num = 0; // i2s port number
i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
    .sample_rate = 24000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_STAND_MSB,
    .intr_alloc_flags = 0,
    .dma_buf_count = 4,
    .dma_buf_len = 384,
    .use_apll = 0,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
};
void setup() {
    CAqTkPicoF_Init(workbuf, 32, "XXXX-XXXX-XXXX-XXXX");
    AqResample_Reset();
    // DAC Setup
    i2s_driver_install((i2s_port_t)i2s_num, &i2s_config, 0, NULL);
    i2s_set_clk((i2s_port_t)i2s_num, i2s_config.sample_rate, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
    i2s_set_dac_mode(I2S_DAC_CHANNEL_RIGHT_EN); // R だけの出力にする
}
void loop() {
    int iret = CAqTkPicoF_SetKoe((const uint8_t*)"ohayo--", 120, 0xffffU); //Set speaking speed 120
    or (;;) {
        int16_t wav[32];
        uint16_t len;
        iret = CAqTkPicoF_SyntheFrame(wav, &len);
        if (iret) {
            // I2S の最終バッファをクリアさせる
            vTaskDelay(100);
            //i2s_zero_dma_buffer((i2s_port_t)i2s_num);
            break;
        }

        int i;
        size_t transBytes = 0;
        uint16_t sample[2];
        for (i = 0; i < len; i++) {
            esp_err_t ret = ESP_OK;
            int16_t wav3[3];
            AqResample_Conv(wav[i], wav3);
            // I2S の DMA バッファへ書込み
            for (int k = 0; k < 3; k++) {
                sample[0] = sample[1] = ((uint16_t)wav3[k]) ^ 0x8000U; // モノラルをステレオデータに変換(単純多重化)
                ret = i2s_write((i2s_port_t)i2s_num, (const char*)sample, sizeof(uint16_t) * 1, &transBytes, portMAX_DELAY);
                if (ret != ESP_OK) {
                    // エラー
                    break;
                }
                if (transBytes < sizeof(uint16_t) * 1) {
                    // タイムアウト
                    ret = ESP_FAIL;
                    break;
                }
            }
            if (ret != ESP_OK) {
                break;
            }
        }
    }
    delay(500);
}