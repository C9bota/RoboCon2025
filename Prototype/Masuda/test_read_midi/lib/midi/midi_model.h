
#ifndef MIDI_MODEL_H
#define MIDI_MODEL_H

#include <cstdint>
#include <string>

// ノートオン/ノートオフイベント
struct MidiNoteEventModel {
	uint8_t channel;    // チャンネル番号 (0-15)
	uint8_t note;       // ノート番号 (0-127)
	uint8_t velocity;   // ベロシティ (0-127)
	uint32_t tick;      // 発生tick（任意: 必要に応じて）
};

// コントロールチェンジイベント
struct MidiControlChangeModel {
	uint8_t channel;    // チャンネル番号 (0-15)
	uint8_t control;    // コントロール番号
	uint8_t value;      // 値
	uint32_t tick;      // 発生tick（任意）
};

// プログラムチェンジイベント
struct MidiProgramChangeModel {
	uint8_t channel;    // チャンネル番号 (0-15)
	uint8_t program;    // プログラム番号（楽器番号）
	uint32_t tick;      // 発生tick（任意）
};

// ピッチベンドイベント
struct MidiPitchBendModel {
	uint8_t channel;    // チャンネル番号 (0-15)
	int16_t value;      // ピッチベンド値（-8192～+8191）
	uint32_t tick;      // 発生tick（任意）
};

// メタイベント（例: テンポ変更、トラック名など）
struct MidiMetaEventModel {
	uint8_t type;           // メタイベントタイプ
	std::string data;      // データ内容
	uint32_t tick;         // 発生tick（任意）
};

#endif // MIDI_MODEL_H
