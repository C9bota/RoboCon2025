
#include "../lib/midi_convert.h"
#include "../../test_servo_move/lib/servo_move.h"

// ノートオンイベントを受け取って処理する関数
void handleNoteOnEvent(const MidiNoteEventModel& event) {
	// ロボットの左腕サーボチャンネル（仮: SERVO_CHANNEL::LEFT_ARM とする）
	// 角度は例として、下げる: 30度, 上げる: 120度, 動作時間: 300ms
	SetServoTargetByAngle(SERVO_CHANNEL::LEFT_ARM, 30, 300);   // 振り下ろす
	// ここで少し待つ（本来は非同期制御やタイマーが望ましいが、例として簡易的に）
    delay(1000);
	// 振り下ろし後、すぐに振り上げる
	SetServoTargetByAngle(SERVO_CHANNEL::LEFT_ARM, 120, 300);  // 振り上げる
}
