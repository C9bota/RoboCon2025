#include "../lib/servo_move.h"
#include <Wire.h> // I2C通信を使用するために必要

// サーボモータ動作モード設定
void SetServoSyncMode(SERVO_SYNC_MODE mode) {
    uint8_t cmd[2];
    cmd[0] = SET_SERVO_SYNC_MODE; // コマンド番号
    cmd[1] = mode;                // 動作モード

    Wire.beginTransmission(SERVO_IC_I2C_ADDR);
    Wire.write(cmd, 2);
    Wire.endTransmission();
}

// サーボモータ一斉スタート
void StartServoSync() {
    uint8_t cmd[1];
    cmd[0] = START_SERVO_SYNC; // コマンド番号

    Wire.beginTransmission(SERVO_IC_I2C_ADDR);
    Wire.write(cmd, 1);
    Wire.endTransmission();
}

// PWMパルス出力許可
void AllowPwmPulseOutput(SERVO_CHANNEL channel, SERVO_PULSE_OUTPUT output) {
    uint8_t cmd[3];
    cmd[0] = ALLOW_PWM_PULSE_OUTPUT; // コマンド番号
    cmd[1] = channel;                // 対象のチャンネル
    cmd[2] = output;                 // 出力許可/禁止

    Wire.beginTransmission(SERVO_IC_I2C_ADDR);
    Wire.write(cmd, 3);
    Wire.endTransmission();
}

// サーボモータ目標位置設定（角度指定）
void SetServoTargetByAngle(SERVO_CHANNEL channel, int angle, int milliseconds) {
    uint8_t cmd[6];
    cmd[0] = SET_SERVO_TARGET_BY_ANGLE; // コマンド番号
    cmd[1] = channel;                       // 対象のチャンネル
    memcpy(&cmd[2], &angle, sizeof(angle)); // 目標角度（1/10度単位）
    memcpy(&cmd[4], &milliseconds, sizeof(milliseconds)); // 動作時間（ミリ秒）

    Wire.beginTransmission(SERVO_IC_I2C_ADDR);
    Wire.write(cmd, 6);
    Wire.endTransmission();
}

// サーボモータ現在位置取得（角度指定）
int GetServoPositionByAngle(SERVO_CHANNEL channel) {
    uint8_t cmd[2];
    uint8_t reply[2];
    cmd[0] = GET_SERVO_POSITION_BY_ANGLE; // コマンド番号
    cmd[1] = channel;                         // 対象のチャンネル

    Wire.beginTransmission(SERVO_IC_I2C_ADDR);
    Wire.write(cmd, 2);
    Wire.endTransmission();

    Wire.requestFrom(SERVO_IC_I2C_ADDR, 2); // 応答を2バイト受信
    for (int i = 0; i < 2; i++) {
        if (Wire.available()) {
            reply[i] = Wire.read();
        }
    }

    int angle;
    memcpy(&angle, reply, sizeof(angle)); // 受信データを角度に変換
    return angle;
}