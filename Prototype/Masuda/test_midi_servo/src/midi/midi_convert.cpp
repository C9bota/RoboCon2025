#include "../../lib/midi/midi_convert.h"
#include "../../lib/servo/servo_move.h"
#include "../../lib/exio/exio.h"

#include <Arduino.h>
#include <Wire.h>

#define ACOUSTIC_SNARE 38
#define BASS_DRUM_1 36
#define CLOSED_HI_HAT 42

static void initServo();
static void initSolenoid();
static void ringBell();
static void hitBassDrum();
static void hitClosedHiHat();
static void defaultAction();

// 開始メタイベントを受け取って初期化処理する関数
void handleInitialMetaEvent(const MidiMetaEventModel& event) {
    initServo();
}

// サーボモータの初期化
static void initServo() {
    // サーボモータ動作モード設定
    SetServoSyncMode(SYNC_MODE);
    Serial.println("SetServoSyncMode(SYNC_MODE) 呼び出しOK");

    // PWM パルス出力許可
    AllowPwmPulseOutput(LEFT_HAND_CHANNEL, ENABLE);
    Serial.println("AllowPwmPulseOutput(LEFT_HAND_CHANNEL, ENABLE) 呼び出しOK");
    delay(100);
}

// ソレノイドの初期化
static void initSolenoid() {
    // IOエキスパンダ初期化
    if (!InitIoExpander(DEV_MCP23017)) {
        Serial.println("IOエキスパンダ初期化成功");
    } else {
        Serial.println("IOエキスパンダ初期化失敗");
    }
    delay(100);
}

// ノートオンイベントを受け取って処理する関数
void handleNoteOnEvent(const MidiNoteEventModel& event) {
    Serial.println("event.note = " + String(event.note));
    if (event.note == ACOUSTIC_SNARE) // MIDIノート番号38はベル
    {
        ringBell();
    }
    else if (event.note == BASS_DRUM_1) // MIDIノート番号36はバスドラム
    {
        hitBassDrum();
    }
    else if (event.note == CLOSED_HI_HAT) // MIDIノート番号42はクローズドハイハット
    {
        hitClosedHiHat();
    }
    else {
        // 未対応のノート番号の場合はデフォルト動作
        defaultAction();
    }
}

// ベル演奏
static void ringBell() {
    // ロボットの左腕サーボチャンネルはLEFT_HAND_CHANNEL
	// 角度：45～60
    // 時間：250ms（bpm120の4つ打ち）
	SetServoTargetByAngle(LEFT_HAND_CHANNEL, 450, 50);   // 振り下ろす
    StartServoSync();
	delay(50);
	SetServoTargetByAngle(LEFT_HAND_CHANNEL, 600, 50);  // 振り上げる
    StartServoSync();
    delay(50);
}

// バスドラム演奏
static void hitBassDrum() {
    // ソレノイドを動作させてドラムを叩く処理をここに実装
    WriteIoExpanderPin(0, 1);   // GPA0端子をHにする
    delay(50);
    WriteIoExpanderPin(0, 0);   // GPA0端子をLにする
    delay(50);
}

// クローズドハイハット演奏
static void hitClosedHiHat() {
    // ソレノイドを動作させてハイハットを叩く処理をここに実装
    WriteIoExpanderPin(0, 1);   // GPA0端子をHにする
    delay(50);
    WriteIoExpanderPin(0, 0);   // GPA0端子をLにする
    delay(50);
}

// デフォルト動作
static void defaultAction() {
    // ロボットの左腕サーボチャンネルはLEFT_HAND_CHANNEL
	// 角度：45～60
    // 時間：250ms（bpm120の4つ打ち）
	SetServoTargetByAngle(LEFT_HAND_CHANNEL, 450, 50);   // 振り下ろす
    StartServoSync();
	delay(50);
	SetServoTargetByAngle(LEFT_HAND_CHANNEL, 600, 50);  // 振り上げる
    StartServoSync();
    delay(50);
}