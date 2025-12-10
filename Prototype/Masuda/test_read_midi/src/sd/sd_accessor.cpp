#include "../../lib/sd/sd_accessor.h"

#include <Arduino.h>
#include <MD_MIDIFile.h>
#include <SdFat.h>
#include <string.h>
#include <string>

#include "../../lib/midi/midi_convert.h"

namespace {

constexpr unsigned long kLogIntervalMs = 1000;
constexpr size_t kMaxPathLength = 64;
constexpr size_t kMidiPreviewEvents = 0;  // 0: unlimited until conditions met
constexpr size_t kMaxNoteLogLines = 4;
constexpr size_t kMaxMetaLogLines = 4;
constexpr size_t kMaxDebugEvents = 6;

SdFat gSd;
MD_MIDIFile gMidiFile;
bool gSdReady = false;
bool gMidiBegun = false;
unsigned long gLastLogMillis = 0;
int gSdCsPin = -1;

char gTestFilePath[kMaxPathLength] = {0};
size_t gTestPreviewBytes = 0;
bool gTestFileConfigured = false;

char gLastFilePath[kMaxPathLength] = {0};
bool gFileStatusKnown = false;
bool gLastFileExists = false;

bool gInitialMetaHandled = false;
bool gMetaLogPrinted = false;
size_t gNoteLogCount = 0;
size_t gNoteOnDispatchCount = 0;
size_t gMetaLogCount = 0;
size_t gDebugEventCount = 0;

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

void handleMetaEvent(const meta_event* mev) {
    if (mev == nullptr) {
        return;
    }

    if (!gMetaLogPrinted && gMetaLogCount < kMaxMetaLogLines) {
        Serial.printf("[MIDI] メタイベント track=%u type=0x%02X size=%u\n",
                      static_cast<unsigned>(mev->track),
                      mev->type,
                      static_cast<unsigned>(mev->size));
        gMetaLogCount++;
        gMetaLogPrinted = true;
    }

    if (gInitialMetaHandled) {
        return;
    }

    MidiMetaEventModel meta{};
    meta.type = mev->type;
    meta.tick = 0;  // MD_MIDIFile は tick 情報を提供しないため 0 を設定
    meta.data.assign(reinterpret_cast<const char*>(mev->data),
                     reinterpret_cast<const char*>(mev->data + mev->size));

    handleInitialMetaEvent(meta);
    gInitialMetaHandled = true;
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

        handleNoteOnEvent(note);
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

}  // namespace

namespace sd_diag {

void begin(int csPin) {
    gSdCsPin = csPin;
    gLastLogMillis = millis() - kLogIntervalMs;
    gSdReady = false;
    gMidiBegun = false;
    gFileStatusKnown = false;
}

void configureTestFile(const char* path, size_t previewBytes) {
    copyPath(gTestFilePath, sizeof(gTestFilePath), path);
    gTestPreviewBytes = previewBytes;
    gTestFileConfigured = (gTestFilePath[0] != '\0');
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
    if (path == nullptr || path[0] == '\0') {
        Serial.println("[MIDI] パスが指定されていません");
        return false;
    }

    if (!ensureSdReady()) {
        Serial.println("[MIDI] SDカードが初期化されていません");
        return false;
    }

    if (!gSd.exists(path)) {
        Serial.printf("[MIDI] %s は存在しません\n", path);
        return false;
    }

    if (!gMidiBegun) {
        gMidiFile.begin(&gSd);
        gMidiBegun = true;
    }

    gMidiFile.setMidiHandler(handleMidiEvent);
    gMidiFile.setMetaHandler(handleMetaEvent);

    gInitialMetaHandled = false;
    gMetaLogPrinted = false;
    gNoteLogCount = 0;
    gNoteOnDispatchCount = 0;
    gMetaLogCount = 0;
    gDebugEventCount = 0;

    const int loadResult = gMidiFile.load(path);
    if (loadResult != MD_MIDIFile::E_OK) {
        Serial.printf("[MIDI] 読み込みに失敗しました (code=%d)\n", loadResult);
        return false;
    }

    size_t eventsProcessed = 0;
    while (!gMidiFile.isEOF()) {
        gMidiFile.getNextEvent();
        eventsProcessed++;

        if (gInitialMetaHandled && gNoteOnDispatchCount > 0) {
            break;
        }

        if (kMidiPreviewEvents > 0 && eventsProcessed >= kMidiPreviewEvents) {
            break;
        }
    }

    gMidiFile.close();
    const bool handledMeta = gInitialMetaHandled;
    const bool handledNote = (gNoteOnDispatchCount > 0);
    Serial.printf("[MIDI] 読み込み確認完了 (events=%lu, meta=%s, note=%s)\n",
                  static_cast<unsigned long>(eventsProcessed),
                  handledMeta ? "ok" : "none",
                  handledNote ? "ok" : "none");

    return handledMeta && handledNote;
}

void update() {
    const unsigned long now = millis();
    if (now - gLastLogMillis < kLogIntervalMs) {
        delay(1);
        return;
    }

    gLastLogMillis = now;

    Serial.println("[DIAG] ----");

    if (!ensureSdReady()) {
        logCardMissing();
        return;
    }

    Serial.println("[SD] カードは利用可能です");

    if (gTestFileConfigured) {
        const bool exists = fileExists(gTestFilePath);
        if (exists && gTestPreviewBytes > 0) {
            readFile(gTestFilePath, Serial, gTestPreviewBytes);
        }

        if (exists) {
            testMidiLoad(gTestFilePath);
        }
    } else if (gFileStatusKnown) {
        Serial.printf("[SD] %s は%s\n", gLastFilePath, gLastFileExists ? "存在します" : "存在しません");
    }
}

}  // namespace sd_diag
