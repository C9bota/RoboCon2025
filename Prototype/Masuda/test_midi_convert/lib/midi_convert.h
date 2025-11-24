
#ifndef MIDI_CONVERT_H
#define MIDI_CONVERT_H

#include <vector>
#include <cstdint>
#include <string>

// MIDIノートイベントを表現する構造体
struct MidiNoteEvent {
	uint8_t note_number;   // ノート番号 (0-127)
	uint8_t velocity;      // ベロシティ (0-127)
	uint32_t start_tick;   // ノートオンのtick
	uint32_t duration;     // tick単位の長さ
};

// MIDIトラック内のノートイベントリスト
struct MidiTrack {
	std::vector<MidiNoteEvent> notes;
	std::string name;
};

// MIDIファイル全体のモデル
struct MidiSong {
	uint16_t format_type;
	uint16_t track_count;
	uint16_t ticks_per_quarter;
	std::vector<MidiTrack> tracks;
};

// SMFバイナリデータからMidiSongを生成する
bool parse_midi(const uint8_t* data, size_t size, MidiSong& out_song);

#endif
