/**
 * SD上のMIDIファイルを読み込み、ノートオンイベントで midi_convert インターフェースを呼び出すテストスケッチ。
 */

#include <Arduino.h>
#include <SPI.h>
#include <SdFat.h>
#include <MD_MIDIFile.h>

#include "./lib/midi/midi_convert.h"

namespace {

constexpr int kSdCsPin = 5;            // 環境に合わせてSDのCSピンを設定
constexpr char kMidiFilePath[] = "/test.mid";  // 読み込むMIDIファイルパス

SdFat gSd;
MD_MIDIFile gMidiFile;

bool gMidiReady = false;
bool gInitialMetaHandled = false;
bool gPlaybackFinished = false;

void handleMetaEvent(const meta_event* mev) {
	if (gInitialMetaHandled || mev == nullptr) {
		return;
	}

	MidiMetaEventModel meta{};
	meta.type = mev->type;
	meta.data.assign(reinterpret_cast<const char*>(mev->data), reinterpret_cast<const char*>(mev->data + mev->size));
	meta.tick = 0;

	handleInitialMetaEvent(meta);
	gInitialMetaHandled = true;
}

void handleMidiEvent(midi_event* ev) {
	if (ev == nullptr || ev->size < 3) {
		return;
	}

	const uint8_t status = ev->data[0];
	if (status != 0x90) {
		return;
	}

	const uint8_t note = ev->data[1];
	const uint8_t velocity = ev->data[2];
	if (velocity == 0) {
		return;  // ノートオフ相当は無視
	}

	MidiNoteEventModel noteEvent{};
	noteEvent.channel = ev->channel;
	noteEvent.note = note;
	noteEvent.velocity = velocity;
	noteEvent.tick = 0;

	handleNoteOnEvent(noteEvent);
}

} // namespace

void setup() {
	Serial.begin(115200);
	while (!Serial && millis() < 2000) {
		delay(10);
	}

	Serial.println("[SETUP] test_read_midi start");

	SPI.begin();

	if (!gSd.begin(kSdCsPin, SD_SCK_MHZ(10))) {
		Serial.println("[SD] 初期化に失敗しました");
		return;
	}

	gMidiFile.begin(&gSd);
	gMidiFile.setMidiHandler(handleMidiEvent);
	gMidiFile.setMetaHandler(handleMetaEvent);

	const int loadResult = gMidiFile.load(kMidiFilePath);
	if (loadResult != MD_MIDIFile::E_OK) {
		Serial.printf("[MIDI] 読み込みに失敗しました (code=%d)\n", loadResult);
		return;
	}

	gMidiReady = true;
	Serial.println("[MIDI] 再生準備完了");
}

void loop() {
	if (!gMidiReady || gPlaybackFinished) {
		return;
	}

	if (!gMidiFile.isEOF()) {
		gMidiFile.getNextEvent();
		return;
	}

	gMidiFile.close();
	gPlaybackFinished = true;
	Serial.println("[MIDI] 再生終了");
}

