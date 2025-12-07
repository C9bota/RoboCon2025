#include "./lib/midi_convert.h"
#include "./lib/midi_model.h"
#include <Wire.h>

void setup() {
    Serial.begin(115200);
    Wire.setClock(100000);
    Wire.begin();
    delay(1000);
    Serial.println("[TEST] midi_convert.cpp 関数テスト開始");

    // 初期化メタイベントのテストデータを作成
    MidiMetaEventModel metaEvent;
    metaEvent.type = 0x51; // 例: テンポ設定
    metaEvent.data = "TestMeta";
    metaEvent.tick = 0;
    handleInitialMetaEvent(metaEvent);
}


void loop() {
    static unsigned long lastCall = 0;
    static int count = 0;
    if (millis() - lastCall > 500) { // 2秒ごとにノートオンイベント発火
        lastCall = millis();
        MidiNoteEventModel noteEvent;
        noteEvent.channel = 0;
        noteEvent.note = 60 + (count % 12); // C4から順に
        noteEvent.velocity = 100;
        noteEvent.tick = count * 480;
        handleNoteOnEvent(noteEvent);
        count++;
    }
}
