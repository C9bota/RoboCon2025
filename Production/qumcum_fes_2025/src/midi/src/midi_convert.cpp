#include "../lib/midi_convert.h"
#include "../../servo/lib/servo_move.h"

#include <Arduino.h>
#include <Wire.h>

// ノートオンイベントを受け取って処理する関数
void handleNoteOnEvent(const MidiNoteEventModel& event) {
	// ロボットの左腕サーボチャンネルはLEFT_HAND_CHANNEL
	// 角度：45～60
    // 時間：250ms（bpm120の4つ打ち）
	SetServoTargetByAngle(LEFT_HAND_CHANNEL, 450, 50);   // 振り下ろす
    StartServoSync();
	delay(50);
	SetServoTargetByAngle(LEFT_HAND_CHANNEL, 600, 50);  // 振り上げる
    StartServoSync();
    delay(50);
}

// 開始メタイベントを受け取って初期化処理する関数
void handleInitialMetaEvent(const MidiMetaEventModel& event) {
    // サーボモータ動作モード設定
    SetServoSyncMode(SYNC_MODE);
    Serial.println("SetServoSyncMode(SYNC_MODE) 呼び出しOK");

    // PWM パルス出力許可
    AllowPwmPulseOutput(LEFT_HAND_CHANNEL, ENABLE);
    Serial.println("AllowPwmPulseOutput(LEFT_HAND_CHANNEL, ENABLE) 呼び出しOK");
    delay(100);
}
