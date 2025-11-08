#include <Wire.h>
#include "./lib/servo_command.h"
#include "./lib/servo_move.h"

void setup() {
    Serial.begin(115200);
    Wire.setClock(100000);
    Wire.begin();
    delay(1000);
    Serial.println("[TEST] servo_move.cpp 関数テスト開始");

    // サーボモータ動作モード設定
    SetServoSyncMode(SYNC_MODE);
    Serial.println("SetServoSyncMode(SYNC_MODE) 呼び出しOK");

    // PWM パルス出力許可
    AllowPwmPulseOutput(LEFT_HAND_CHANNEL, ENABLE);
    Serial.println("AllowPwmPulseOutput(LEFT_HAND_CHANNEL, ENABLE) 呼び出しOK");
    delay(100);
}

void loop() {
    static int phase = 0;
    static int j = 0;
    int pos;
    switch (phase%5) {
        case 0:
            // モータの角度を0度にセット
            SetServoTargetByAngle(LEFT_HAND_CHANNEL, 0, 1000);
            Serial.println("SetServoTargetByAngle(LEFT_HAND_CHANNEL, 0, 1000)");
            StartServoSync();
            Serial.println("StartServoSync()");
            delay(1000);
            phase++;
            break;
        case 1:
            // モータの現在座標を取得
            pos = GetServoPositionByAngle(LEFT_HAND_CHANNEL);
            Serial.print("Angle: ");
            Serial.println(pos);
            phase++;
            break;
        case 2:
            // モータの角度を180度にセット
            SetServoTargetByAngle(LEFT_HAND_CHANNEL, 1800, 1000);
            Serial.println("SetServoTargetByAngle(LEFT_HAND_CHANNEL, 1800, 1000)");
            StartServoSync();
            Serial.println("StartServoSync()");
            j = 0;
            phase++;
            break;
        case 3:
            if (j < 10) {
                delay(100);
                pos = GetServoPositionByAngle(LEFT_HAND_CHANNEL);
                Serial.print("Angle: ");
                Serial.println(pos);
                j++;
            } else {
                phase++;
            }
            break;
        case 4:
            delay(400);
            // モータの角度を90度にセット
            SetServoTargetByAngle(LEFT_HAND_CHANNEL, 900, 1000);
            Serial.println("SetServoTargetByAngle(LEFT_HAND_CHANNEL, 900, 1000)");
            StartServoSync();
            Serial.println("StartServoSync()");
            delay(1000);
            phase++;
            break;
		default:
            // 停止
            break;
    }
    delay(100);
}
