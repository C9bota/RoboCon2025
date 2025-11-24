
#include "../lib/midi_convert.h"
#include <cstring>

// 可変長数量のデコード
static uint32_t read_varlen(const uint8_t*& p, const uint8_t* end) {
	uint32_t value = 0;
	while (p < end) {
		uint8_t c = *p++;
		value = (value << 7) | (c & 0x7F);
		if (!(c & 0x80)) break;
	}
	return value;
}

// 4バイトビッグエンディアン整数
static uint32_t read_be32(const uint8_t* p) {
	return (p[0]<<24)|(p[1]<<16)|(p[2]<<8)|p[3];
}

// 2バイトビッグエンディアン整数
static uint16_t read_be16(const uint8_t* p) {
	return (p[0]<<8)|p[1];
}

bool parse_midi(const uint8_t* data, size_t size, MidiSong& out_song) {
	const uint8_t* p = data;
	const uint8_t* end = data + size;
	if (size < 14 || std::memcmp(p, "MThd", 4) != 0) return false;
	uint32_t hdr_len = read_be32(p+4);
	if (hdr_len < 6) return false;
	out_song.format_type = read_be16(p+8);
	out_song.track_count = read_be16(p+10);
	out_song.ticks_per_quarter = read_be16(p+12);
	p += 8 + hdr_len;
	out_song.tracks.clear();

	for (int t = 0; t < out_song.track_count && p+8 <= end; ++t) {
		if (std::memcmp(p, "MTrk", 4) != 0) return false;
		uint32_t trk_len = read_be32(p+4);
		p += 8;
		if (p + trk_len > end) return false;
		const uint8_t* trk_end = p + trk_len;
		MidiTrack track;
		uint32_t tick = 0;
		uint8_t running_status = 0;
		std::vector<MidiNoteEvent> open_notes[128];

		while (p < trk_end) {
			uint32_t delta = read_varlen(p, trk_end);
			tick += delta;
			if (p >= trk_end) break;
			uint8_t status = *p;
			if (status < 0x80) {
				// running status
				if (!running_status) break;
				status = running_status;
			} else {
				++p;
				running_status = status;
			}
			if ((status & 0xF0) == 0x90) {
				// Note On
				uint8_t note = *p++;
				uint8_t vel = *p++;
				if (vel > 0) {
					MidiNoteEvent ev{note, vel, tick, 0};
					open_notes[note].push_back(ev);
				} else {
					// Note On (vel=0) == Note Off
					if (!open_notes[note].empty()) {
						MidiNoteEvent ev = open_notes[note].back();
						open_notes[note].pop_back();
						ev.duration = tick - ev.start_tick;
						track.notes.push_back(ev);
					}
				}
			} else if ((status & 0xF0) == 0x80) {
				// Note Off
				uint8_t note = *p++;
				uint8_t vel = *p++;
				if (!open_notes[note].empty()) {
					MidiNoteEvent ev = open_notes[note].back();
					open_notes[note].pop_back();
					ev.duration = tick - ev.start_tick;
					track.notes.push_back(ev);
				}
			} else if (status == 0xFF) {
				// Meta event
				uint8_t type = *p++;
				uint32_t len = read_varlen(p, trk_end);
				if (type == 0x03 && len > 0 && p+len <= trk_end) {
					track.name = std::string((const char*)p, len);
				}
				p += len;
			} else if ((status & 0xF0) == 0xC0 || (status & 0xF0) == 0xD0) {
				// Program Change / Channel Pressure
				++p;
			} else {
				// 2 data bytes
				p += 2;
			}
		}
		// 残ったNoteOnをクローズ
		for (int n = 0; n < 128; ++n) {
			for (auto& ev : open_notes[n]) {
				ev.duration = tick - ev.start_tick;
				track.notes.push_back(ev);
			}
		}
		out_song.tracks.push_back(track);
		p = trk_end;
	}
	return true;
}
