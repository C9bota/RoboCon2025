/**
 * SDカードが利用可能かどうかを確認する簡易テストスケッチ。
 */

#include <Arduino.h>
#include <SPI.h>
#include "./lib/sd/sd_check.h"

namespace {

constexpr int kSdCsPin = 5;  // 環境に合わせてSDのCSピンを設定
constexpr char kTestFilePath[] = "/test.mid";
constexpr size_t kPreviewBytes = 128;

} // namespace

void setup() {
	Serial.begin(115200);
	delay(1000);  // シリアルモニタが接続される猶予を持たせる

	Serial.println("[SETUP] SD presence check start");

	SPI.begin();

	sd_check::begin(kSdCsPin);

	if (sd_check::fileExists(kTestFilePath)) {
		Serial.println("[SETUP] ファイルの先頭データを読み込みます");
		sd_check::readFile(kTestFilePath, Serial, kPreviewBytes);
	} else {
		Serial.println("[SETUP] 指定ファイルが見つかりません");
	}
}

void loop() {
	sd_check::update();
}

