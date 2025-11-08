#include <Wire.h>
#include "./lib/servo_command.h"
#include "./lib/servo_move.h"
//#include "./src/servo_move.cpp"

void setup() {
    Serial.begin(115200);
    Wire.begin();
    delay(1000);
    Serial.println("[TEST] servo_move.cpp 関数テスト開始");

    // サーボ動作モード設定
    SetServoSyncMode(SYNC_MODE);
    Serial.println("SetServoSyncMode(SYNC_MODE) 呼び出しOK");

    // サーボ一斉スタート
    StartServoSync();
    Serial.println("StartServoSync() 呼び出しOK");

    // PWMパルス出力許可
    AllowPwmPulseOutput(RIGHT_HAND_CHANNEL, ENABLE);
    Serial.println("AllowPwmPulseOutput(RIGHT_HAND_CHANNEL, ENABLE) 呼び出しOK");

    // サーボ目標位置設定（角度指定）
    SetServoTargetByAngle(RIGHT_HAND_CHANNEL, 900, 500); // 90.0度, 500ms
    Serial.println("SetServoTargetByAngle(RIGHT_HAND_CHANNEL, 900, 500) 呼び出しOK");

    // サーボ現在位置取得（角度指定）
    int pos = GetServoPositionByAngle(RIGHT_HAND_CHANNEL);
    Serial.print("GetServoPositionByAngle(RIGHT_HAND_CHANNEL) = ");
    Serial.println(pos);

    Serial.println("[TEST] 完了");
}

void loop() {
	// 何もしない
}
