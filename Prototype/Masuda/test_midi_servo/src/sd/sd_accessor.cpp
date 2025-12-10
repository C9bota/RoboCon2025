#include "../../lib/sd/sd_accessor.h"

#include <Arduino.h>
#include <MD_MIDIFile.h>
#include <SdFat.h>
#include <string.h>
#include "../../lib/midi/midi_convert.h"

namespace {

constexpr unsigned long kLogIntervalMs = 10000;
constexpr unsigned long kStartRetryIntervalMs = 1000;
constexpr size_t kMaxPathLength = 64;
constexpr size_t kMaxNoteLogLines = 4;
constexpr size_t kMaxDebugEvents = 6;
constexpr size_t kMaxEventsPerUpdate = 12;

SdFat gSd;
MD_MIDIFile gMidiFile;
bool gSdReady = false;
bool gMidiBegun = false;
bool gMidiPlaybackStarted = false;
bool gMidiPlaying = false;
bool gMidiPlaybackFinished = false;
unsigned long gLastLogMillis = 0;
unsigned long gLastStartAttemptMillis = 0;
int gSdCsPin = -1;

char gTestFilePath[kMaxPathLength] = {0};
size_t gTestPreviewBytes = 0;
bool gTestFileConfigured = false;

char gLastFilePath[kMaxPathLength] = {0};
bool gFileStatusKnown = false;
bool gLastFileExists = false;

size_t gNoteLogCount = 0;
size_t gNoteOnDispatchCount = 0;
size_t gDebugEventCount = 0;
sd_diag::NoteOnEventCallback gNoteOnCallback = nullptr;
size_t gTotalEventsProcessed = 0;
bool gPreviewDumped = false;

void logCardMissing() {
    Serial.println("[SD] カード未検出です。カードを挿入してからリセットしてください");
}

bool ensureSdReady() {
    if (gSdReady) {
        return true;
    }

    if (gSdCsPin < 0) {
        Serial.println("[SD] CSピンが設定されていません");
        return false;
    }

    if (!gSd.begin(gSdCsPin, SD_SCK_MHZ(10))) {
        Serial.println("[SD] カードが見つかりません");
        gFileStatusKnown = false;
        return false;
    }

    gSdReady = true;
    Serial.println("[SD] カードを検出しました");
    return true;
}

void handleMidiEvent(midi_event* ev) {
    if (ev == nullptr || ev->size < 1) {
        return;
    }

    const uint8_t command = ev->data[0] & 0xF0;
    const bool isNoteOn = (command == 0x90 && ev->size >= 3 && ev->data[2] != 0);
    const bool isNoteOff = (command == 0x80 && ev->size >= 3) || (command == 0x90 && ev->size >= 3 && ev->data[2] == 0);

    if (gDebugEventCount < kMaxDebugEvents) {
        Serial.printf("[MIDI] event track=%u cmd=0x%02X size=%u\n",
                      static_cast<unsigned>(ev->track),
                      static_cast<unsigned>(command),
                      static_cast<unsigned>(ev->size));
        gDebugEventCount++;
    }

    if (isNoteOn) {
        MidiNoteEventModel note{};
        note.channel = ev->channel;
        note.note = ev->data[1];
        note.velocity = ev->data[2];
        note.tick = 0;  // tick 情報は未使用のため 0

        if (gNoteOnCallback != nullptr) {
            gNoteOnCallback(note);
        } else {
            handleNoteOnEvent(note);
        }
        gNoteOnDispatchCount++;

        if (gNoteLogCount < kMaxNoteLogLines) {
            Serial.printf("[MIDI] NoteOn ch=%u note=%u vel=%u\n",
                          static_cast<unsigned>(ev->channel + 1),
                          ev->data[1],
                          ev->data[2]);
            gNoteLogCount++;
        }
        return;
    }

    if (isNoteOff && gNoteLogCount < kMaxNoteLogLines) {
        Serial.printf("[MIDI] NoteOff ch=%u note=%u\n",
                      static_cast<unsigned>(ev->channel + 1),
                      ev->data[1]);
        gNoteLogCount++;
    }
}

void copyPath(char* dest, size_t destSize, const char* src) {
    if (destSize == 0) {
        return;
    }

    if (src == nullptr) {
        dest[0] = '\0';
        return;
    }

    strncpy(dest, src, destSize - 1);
    dest[destSize - 1] = '\0';
}

bool startMidiPlayback(const char* path) {
    if (path == nullptr || path[0] == '\0') {
        Serial.println("[MIDI] パスが指定されていません");
        return false;
    }

    if (!ensureSdReady()) {
        Serial.println("[MIDI] SDカードが初期化されていません");
        return false;
    }

    const bool exists = gSd.exists(path);
    copyPath(gLastFilePath, sizeof(gLastFilePath), path);
    gLastFileExists = exists;
    gFileStatusKnown = true;

    if (!exists) {
        Serial.printf("[MIDI] %s は存在しません\n", path);
        return false;
    }

    if (!gMidiBegun) {
        gMidiFile.begin(&gSd);
        gMidiBegun = true;
    }

    gMidiFile.setMidiHandler(handleMidiEvent);

    gNoteLogCount = 0;
    gNoteOnDispatchCount = 0;
    gDebugEventCount = 0;
    gTotalEventsProcessed = 0;

    const int loadResult = gMidiFile.load(path);
    if (loadResult != MD_MIDIFile::E_OK) {
        Serial.printf("[MIDI] 読み込みに失敗しました (code=%d)\n", loadResult);
        return false;
    }

    gMidiPlaybackStarted = true;
    gMidiPlaying = true;
    gMidiPlaybackFinished = false;
    if (!gPreviewDumped) {
        gPreviewDumped = true;
    }

    Serial.printf("[MIDI] %s の再生を開始します\n", path);
    return true;
}

void pumpMidiPlayback() {
    if (!gMidiPlaying) {
        return;
    }

    size_t eventsThisCycle = 0;
    while (!gMidiFile.isEOF() && gMidiFile.getNextEvent()) {
        eventsThisCycle++;
        gTotalEventsProcessed++;

        if (eventsThisCycle >= kMaxEventsPerUpdate) {
            break;
        }
    }

    if (gMidiFile.isEOF()) {
        gMidiFile.close();
        gMidiPlaying = false;
        gMidiPlaybackFinished = true;
        Serial.printf("[MIDI] 再生が完了しました (events=%lu, noteOn=%lu)\n",
                      static_cast<unsigned long>(gTotalEventsProcessed),
                      static_cast<unsigned long>(gNoteOnDispatchCount));
    }
}

}  // namespace

namespace sd_diag {

void begin(int csPin) {
    gSdCsPin = csPin;
    gLastLogMillis = millis() - kLogIntervalMs;
    gSdReady = false;
    gMidiBegun = false;
    gFileStatusKnown = false;
    gMidiPlaybackStarted = false;
    gMidiPlaying = false;
    gMidiPlaybackFinished = false;
    gPreviewDumped = false;
    gLastStartAttemptMillis = 0;
    gNoteLogCount = 0;
    gNoteOnDispatchCount = 0;
    gDebugEventCount = 0;
    gTotalEventsProcessed = 0;
}

void configureTestFile(const char* path, size_t previewBytes) {
    copyPath(gTestFilePath, sizeof(gTestFilePath), path);
    gTestPreviewBytes = previewBytes;
    gTestFileConfigured = (gTestFilePath[0] != '\0');
    gPreviewDumped = false;
    gMidiPlaybackStarted = false;
    gMidiPlaying = false;
    gMidiPlaybackFinished = false;
    gLastStartAttemptMillis = 0;
    gNoteLogCount = 0;
    gNoteOnDispatchCount = 0;
    gDebugEventCount = 0;
    gTotalEventsProcessed = 0;
}

void setNoteOnHandler(NoteOnEventCallback onNoteOn) {
    gNoteOnCallback = onNoteOn;
}

bool isReady() {
    return gSdReady;
}

SdFat* getSdFat() {
    return &gSd;
}

bool fileExists(const char* path) {
    if (path == nullptr || path[0] == '\0') {
        Serial.println("[SD] パスが指定されていません");
        return false;
    }

    if (!ensureSdReady()) {
        Serial.println("[SD] 初期化が完了していません");
        return false;
    }

    const bool exists = gSd.exists(path);
    Serial.printf("[SD] %s は%s\n", path, exists ? "存在します" : "存在しません");

    copyPath(gLastFilePath, sizeof(gLastFilePath), path);
    gLastFileExists = exists;
    gFileStatusKnown = true;

    return exists;
}

bool readFile(const char* path, Print& out, size_t maxBytes) {
    if (path == nullptr || path[0] == '\0') {
        Serial.println("[SD] パスが指定されていません");
        return false;
    }

    if (!ensureSdReady()) {
        Serial.println("[SD] 初期化が完了していません");
        return false;
    }

    File file = gSd.open(path, FILE_READ);
    if (!file) {
        Serial.printf("[SD] %s のオープンに失敗しました\n", path);
        return false;
    }

    Serial.printf("[SD] %s の読み込みを開始します (size=%lu bytes)\n", path, static_cast<unsigned long>(file.size()));

    char buffer[128];
    size_t totalRead = 0;

    while (file.available()) {
        if (maxBytes > 0 && totalRead >= maxBytes) {
            break;
        }

        size_t toRead = sizeof(buffer);
        if (maxBytes > 0) {
            const size_t remaining = maxBytes - totalRead;
            if (remaining < toRead) {
                toRead = remaining;
            }
        }

        if (toRead == 0) {
            break;
        }

        const size_t bytesRead = file.read(buffer, toRead);
        if (bytesRead == 0) {
            break;
        }

        out.write(reinterpret_cast<const uint8_t*>(buffer), bytesRead);
        totalRead += bytesRead;
    }

    file.close();
    if (totalRead > 0) {
        out.println();
    }

    Serial.printf("[SD] %s の読み込みが完了しました (read=%lu bytes)\n", path, static_cast<unsigned long>(totalRead));
    return true;
}

bool testMidiLoad(const char* path) {
    return startMidiPlayback(path);
}

void update() {
    const unsigned long now = millis();

    if (!gSdReady) {
        if (!ensureSdReady()) {
            if (now - gLastLogMillis >= kLogIntervalMs) {
                gLastLogMillis = now;
                logCardMissing();
            }
            delay(10);
            return;
        }
    }

    if (gTestFileConfigured && !gPreviewDumped && gTestPreviewBytes > 0) {
        if (readFile(gTestFilePath, Serial, gTestPreviewBytes)) {
            gPreviewDumped = true;
        }
    }

    if (gTestFileConfigured && !gMidiPlaybackStarted && !gMidiPlaybackFinished) {
        if (now - gLastStartAttemptMillis >= kStartRetryIntervalMs) {
            gLastStartAttemptMillis = now;
            startMidiPlayback(gTestFilePath);
        }
    }

    pumpMidiPlayback();

    if (now - gLastLogMillis >= kLogIntervalMs) {
        gLastLogMillis = now;
        Serial.println("[DIAG] ----");
        Serial.println("[SD] カードは利用可能です");

        if (gTestFileConfigured) {
            Serial.printf("[MIDI] started=%s playing=%s finished=%s events=%lu noteOn=%lu\n",
                          gMidiPlaybackStarted ? "yes" : "no",
                          gMidiPlaying ? "yes" : "no",
                          gMidiPlaybackFinished ? "yes" : "no",
                          static_cast<unsigned long>(gTotalEventsProcessed),
                          static_cast<unsigned long>(gNoteOnDispatchCount));
        } else if (gFileStatusKnown) {
            Serial.printf("[SD] %s は%s\n", gLastFilePath, gLastFileExists ? "存在します" : "存在しません");
        }
    }
}

}  // namespace sd_diag
