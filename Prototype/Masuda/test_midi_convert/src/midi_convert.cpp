
#include "../lib/midi_convert.h"
#include "../../test_servo_move/lib/servo_move.h"

// ノートオンイベントを受け取って処理する関数
void handleNoteOnEvent(const MidiNoteEventModel& event) {
	// ロボットの左腕サーボチャンネルはLEFT_HAND_CHANNEL
	// 角度は例として、下げる: 30度, 上げる: 120度, 動作時間: 300ms
	SetServoTargetByAngle(LEFT_HAND_CHANNEL, 30, 300);   // 振り下ろす
	delay(1000);
	SetServoTargetByAngle(LEFT_HAND_CHANNEL, 120, 300);  // 振り上げる
}

// 初期化メタイベントを受け取って処理する関数
void handleInitialMetaEvent(const MidiMetaEventModel& event) {
	// 初めのメタイベント発生時には、初期化処理を行う
	Serial.begin(115200);
    Wire.setClock(100000);
    Wire.begin();
    delay(1000);
    //Serial.println("[TEST] servo_move.cpp 関数テスト開始");

    // サーボモータ動作モード設定
    SetServoSyncMode(SYNC_MODE);
    Serial.println("SetServoSyncMode(SYNC_MODE) 呼び出しOK");

    // PWM パルス出力許可
    AllowPwmPulseOutput(LEFT_HAND_CHANNEL, ENABLE);
    Serial.println("AllowPwmPulseOutput(LEFT_HAND_CHANNEL, ENABLE) 呼び出しOK");
    delay(100);
}
