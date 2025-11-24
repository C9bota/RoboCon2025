#include "lib/midi_convert.h"
#include "lib/midi_model.h"
#include <Wire.h>
#include "../test_servo_move/lib/servo_command.h"
#include "../test_servo_move/lib/servo_move.h"

// テスト用: 左腕サーボのチャンネル定義（仮）
#ifndef SERVO_CHANNEL_LEFT_ARM_DEFINED
#define SERVO_CHANNEL_LEFT_ARM_DEFINED
SERVO_CHANNEL LEFT_HAND_CHANNEL = SERVO_CHANNEL::LEFT_ARM; // 実際の値に合わせて修正
#endif

void setup() {
    Serial.begin(115200);
    Wire.setClock(100000);
    Wire.begin();
    delay(1000);
    Serial.println("[TEST] MIDIノートオンイベント→サーボ動作テスト");

    // サーボモータ動作モード設定
    SetServoSyncMode(SYNC_MODE);
    Serial.println("SetServoSyncMode(SYNC_MODE) 呼び出しOK");

    // PWM パルス出力許可
    AllowPwmPulseOutput(LEFT_HAND_CHANNEL, ENABLE);
    Serial.println("AllowPwmPulseOutput(LEFT_HAND_CHANNEL, ENABLE) 呼び出しOK");
    delay(100);

}

void loop() {
    // 何もしない
}
