#include "../../lib/sd/sd_accessor.h"

#include <Arduino.h>
#include <MD_MIDIFile.h>
#include <SdFat.h>
#include <string.h>

namespace {

constexpr unsigned long kLogIntervalMs = 1000;
constexpr size_t kMaxPathLength = 64;
constexpr size_t kMidiPreviewEvents = 8;

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

bool gMidiNoteLogged = false;
bool gMidiMetaLogged = false;

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
    if (mev == nullptr || gMidiMetaLogged) {
        return;
    }

    Serial.printf("[MIDI] メタイベント type=0x%02X size=%u\n", mev->type, static_cast<unsigned>(mev->size));
    gMidiMetaLogged = true;
}

void handleMidiEvent(midi_event* ev) {
    if (ev == nullptr || ev->size < 1 || gMidiNoteLogged) {
        return;
    }

    const uint8_t command = ev->data[0] & 0xF0;
    if (command == 0x90 && ev->size >= 3 && ev->data[2] != 0) {
        Serial.printf("[MIDI] NoteOn ch=%u note=%u vel=%u\n",
                      static_cast<unsigned>(ev->channel + 1),
                      ev->data[1],
                      ev->data[2]);
        gMidiNoteLogged = true;
        return;
    }

    if (command == 0x80 && ev->size >= 3) {
        Serial.printf("[MIDI] NoteOff ch=%u note=%u\n",
                      static_cast<unsigned>(ev->channel + 1),
                      ev->data[1]);
        gMidiNoteLogged = true;
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

    gMidiNoteLogged = false;
    gMidiMetaLogged = false;

    const int loadResult = gMidiFile.load(path);
    if (loadResult != MD_MIDIFile::E_OK) {
        Serial.printf("[MIDI] 読み込みに失敗しました (code=%d)\n", loadResult);
        return false;
    }

    size_t eventsProcessed = 0;
    while (!gMidiFile.isEOF() && eventsProcessed < kMidiPreviewEvents) {
        gMidiFile.getNextEvent();
        eventsProcessed++;
    }

    gMidiFile.close();
    Serial.printf("[MIDI] 読み込み確認完了 (events=%lu)\n", static_cast<unsigned long>(eventsProcessed));
    return true;
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
