
#include "../lib/midi_convert.h"

// ノートオンイベントを受け取って処理する関数（例: ログ出力や演奏処理など）
void handleNoteOnEvent(const MidiNoteEventModel& event) {
	// ここにノートオンイベント発生時の処理を記述
	// 例: シリアル出力や音源制御など
	// printf("NoteOn: ch=%d note=%d vel=%d tick=%lu\n", event.channel, event.note, event.velocity, event.tick);
}
