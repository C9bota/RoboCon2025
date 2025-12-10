#include "../../lib/sd/sd_check.h"

#include <Arduino.h>
#include <SdFat.h>
#include <string.h>

namespace {

constexpr unsigned long kLogIntervalMs = 1000;
constexpr size_t kMaxPathLength = 64;

SdFat gSd;
bool gSdReady = false;
unsigned long gLastLogMillis = 0;
int gSdCsPin = -1;
char gLastFilePath[kMaxPathLength] = {0};
bool gFileStatusKnown = false;
bool gLastFileExists = false;

}  // namespace

namespace sd_check {

void begin(int csPin) {
    gSdCsPin = csPin;
    gLastLogMillis = millis();

    if (!gSd.begin(gSdCsPin, SD_SCK_MHZ(10))) {
        Serial.println("[SD] カードが見つかりません");
        gSdReady = false;
        gFileStatusKnown = false;
        return;
    }

    gSdReady = true;
    Serial.println("[SD] カードを検出しました");
}

void update() {
    const unsigned long now = millis();
    if (now - gLastLogMillis < kLogIntervalMs) {
        delay(10);
        return;
    }

    gLastLogMillis = now;

    if (!gSdReady) {
        if (gSd.begin(gSdCsPin, SD_SCK_MHZ(10))) {
            gSdReady = true;
            Serial.println("[SD] カードを検出しました");
            return;
        }

        Serial.println("[SD] カード未検出です。カードを挿入してからリセットしてください");
        return;
    }

    Serial.println("[SD] カードは利用可能です");

    if (gFileStatusKnown) {
        Serial.printf("[SD] %s は%s\n", gLastFilePath, gLastFileExists ? "存在します" : "存在しません");
    }
}

bool fileExists(const char* path) {
    if (path == nullptr || path[0] == '\0') {
        Serial.println("[SD] パスが指定されていません");
        return false;
    }

    if (!gSdReady) {
        Serial.println("[SD] 初期化が完了していません");
        return false;
    }

    const bool exists = gSd.exists(path);
    Serial.printf("[SD] %s は%s\n", path, exists ? "存在します" : "存在しません");

    strncpy(gLastFilePath, path, kMaxPathLength - 1);
    gLastFilePath[kMaxPathLength - 1] = '\0';
    gLastFileExists = exists;
    gFileStatusKnown = true;

    return exists;
}

bool readFile(const char* path, Print& out, size_t maxBytes) {
    if (path == nullptr || path[0] == '\0') {
        Serial.println("[SD] パスが指定されていません");
        return false;
    }

    if (!gSdReady) {
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

}  // namespace sd_check
