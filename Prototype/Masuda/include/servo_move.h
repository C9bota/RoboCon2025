# ifdef SERVO_MOVE_H
# define SERVO_MOVE_H

#include "servo_channel.h"
#include "servo_command.h"

// サーボモータ動作モード設定
public void SetServoSyncMode(int mode);

// サーボモータ一斉スタート
public void StartServoSync();

// PWMパルス出力許可
public void AllowPwmPulseOutput(SERVO_CHANNEL channel, bool allow);

// サーボモータ目標位置設定（角度指定）
public void SetServoTargetByAngle(SERVO_CHANNEL channel, int ANGLE);

// サーボモータ現在位置取得（角度指定）
public int GetServoPositionByAngle(SERVO_CHANNEL channel);




#endif