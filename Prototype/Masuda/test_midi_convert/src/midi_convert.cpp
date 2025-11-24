
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
