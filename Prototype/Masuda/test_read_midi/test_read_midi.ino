/**
 * SDカードおよびMIDIファイルを1秒おきに診断するテストスケッチ。
 */

#include <Arduino.h>
#include <SPI.h>

#include "./lib/sd/sd_accessor.h"

namespace {

constexpr int kSdCsPin = 5;  // 環境に合わせてSDのCSピンを設定
//constexpr char kTestFilePath[] = "/test.mid";
constexpr char kTestFilePath[] = "/test-1track.mid";
constexpr size_t kPreviewBytes = 128;

} // namespace

void setup() {
	Serial.begin(115200);
	delay(1000);  // シリアルモニタが接続される猶予を持たせる

	Serial.println("[SETUP] SD accessor start");

	SPI.begin();

	sd_diag::begin(kSdCsPin);
	sd_diag::configureTestFile(kTestFilePath, kPreviewBytes);
}

void loop() {
	sd_diag::update();
}

