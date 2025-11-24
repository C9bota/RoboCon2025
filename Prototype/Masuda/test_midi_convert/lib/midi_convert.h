#ifndef MIDI_CONVERT_H
#define MIDI_CONVERT_H

#include "midi_model.h"

// ノートオンイベントを受け取って処理するインターフェース
void handleNoteOnEvent(const MidiNoteEventModel& event);

#endif
