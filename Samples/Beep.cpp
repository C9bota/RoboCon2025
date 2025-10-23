void setup() {
    // チャンネル 0、キャリア周波数 12kHz、8 ビットレンジ
    ledcSetup(0, 12000, 8);
    // BEEP を出力するピンを設定する
    ledcAttachPin(4, 0);
    }
    void loop() {
    // 440Hz の音を 100msec 出力する
    ledcWriteTone(0, 440);
    delay(62);
    // 880Hz の音を 100msec 出力する
    ledcWriteTone(0, 880);
    delay(62);
    // 音を止める(消音)
    ledcWriteTone(0, 0);
    delay(1000);
}