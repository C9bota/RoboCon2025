#include "../../lib/midi/midi_convert.h"
#include "../../lib/servo/servo_move.h"

#include <Arduino.h>
#include <Wire.h>

#define ACOUSTIC_SNARE 38
#define BASS_DRUM_1 36
#define CLOSED_HI_HAT 42

//constexpr uint8_t kRedLedPin = 32;

static void initServo();
static void ringBell();
static void hitBassDrum();
static void hitClosedHiHat();
static void defaultAction();
//static void setRedLed(bool on);

// 開始メタイベントを受け取って初期化処理する関数
void handleInitialMetaEvent(const MidiMetaEventModel& event) {
    //setRedLed(true); // 初期状態は点灯
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

// ノートオンイベントを受け取って処理する関数
void handleNoteOnEvent(const MidiNoteEventModel& event) {
    Serial.println("event.note = " + String(event.note));
    
    if (event.note == ACOUSTIC_SNARE) {
        ringBell();
        return;
    }
    if (event.note == BASS_DRUM_1) {
        hitBassDrum();
        return;
    }
    if (event.note == CLOSED_HI_HAT) {
        hitClosedHiHat();
        return;
    }

    // 未対応のノート番号の場合はデフォルト動作
    defaultAction();
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
    /*
    setRedLed(false); // バスドラムのノートで赤色LEDを消灯
    delay(100);
    setRedLed(true);  // 元に戻す
    */
    SetServoTargetByAngle(RIGHT_HAND_CHANNEL, 450, 50);   // 振り下ろす
    StartServoSync();
	delay(50);
	SetServoTargetByAngle(RIGHT_HAND_CHANNEL, 600, 50);  // 振り上げる
    StartServoSync();
    delay(50);
}

// クローズドハイハット演奏
static void hitClosedHiHat() {
    /*
    setRedLed(false);
    delay(100);
    setRedLed(true);  // 元に戻す
    */
    SetServoTargetByAngle(RIGHT_HAND_CHANNEL, 450, 50);   // 振り下ろす
    StartServoSync();
	delay(50);
	SetServoTargetByAngle(RIGHT_HAND_CHANNEL, 600, 50);  // 振り上げる
    StartServoSync();
    delay(50);
}

// デフォルト動作
static void defaultAction() {
    // ロボットの左腕サーボチャンネルはLEFT_HAND_CHANNEL
	// 角度：45～60
    // 時間：250ms（bpm120の4つ打ち）
    /*
	SetServoTargetByAngle(LEFT_HAND_CHANNEL, 450, 50);   // 振り下ろす
    StartServoSync();
	delay(50);
	SetServoTargetByAngle(LEFT_HAND_CHANNEL, 600, 50);  // 振り上げる
    StartServoSync();
    delay(50);
    */
}

/*
static void setRedLed(bool on) {
    digitalWrite(kRedLedPin, on ? HIGH : LOW);
}
*/