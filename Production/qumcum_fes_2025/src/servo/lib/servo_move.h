# ifndef SERVO_MOVE_H
# define SERVO_MOVE_H

#include "servo_channel.h"
#include "servo_command.h"

#define SERVO_IC_I2C_ADDR 8

// サーボモータ動作モード設定
void SetServoSyncMode(SERVO_SYNC_MODE mode = SERVO_SYNC_MODE::SYNC_MODE);

// サーボモータ一斉スタート
void StartServoSync();

// PWMパルス出力許可
void AllowPwmPulseOutput(SERVO_CHANNEL channel, SERVO_PULSE_OUTPUT output = SERVO_PULSE_OUTPUT::DISABLE);

// サーボモータ目標位置設定（角度指定）
void SetServoTargetByAngle(SERVO_CHANNEL channel, int angle, int milliseconds);

// サーボモータ現在位置取得（角度指定）
int GetServoPositionByAngle(SERVO_CHANNEL channel);




#endif