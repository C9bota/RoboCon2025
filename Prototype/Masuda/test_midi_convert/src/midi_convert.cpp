
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

// メタイベントを受け取って処理する関数
void handleMetaEvent(const MidiMetaEventModel& event) {
	// ここにメタイベント発生時の処理を記述
	// 例: シリアル出力やテンポ・曲名の取得など
	// Serial.print("MetaEvent: type=");
	// Serial.print(event.type);
	// Serial.print(", data=");
	// Serial.print(event.data.c_str());
	// Serial.print(", tick=");
	// Serial.println(event.tick);
}
