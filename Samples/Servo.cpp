#include <Wire.h>
#define SERVO_IC_I2C_ADDR 8
void send_servo_driver_cmd(const uint8_t* tx, int size)
{
    Wire.beginTransmission(SERVO_IC_I2C_ADDR);
    while (size-- > 0) {
        Wire.write(*tx++);
    }
    Wire.endTransmission();
}
uint16_t recv_servo_driver_reply(uint8_t* rx, int size)
{
    uint16_t n = 0;
    Wire.requestFrom(SERVO_IC_I2C_ADDR, size); // request len bytes from Slave ID #addr
    while (Wire.available()) {
        *rx++ = Wire.read();
        n++;
    }
    return n;
}
void setup() {
    Serial.begin(115200);

    // I2C のクロックを 100,000Hz に設定
    Wire.setClock(100000);
    Wire.begin();

    uint8_t cmd[10]; // 送信電文格納バッファ
    uint8_t reply[10]; // 応答電文格納バッファ

    // サーボモータ動作モード設定
    cmd[0] = 0; // cmdno
    cmd[1] = 1; // sw

    // コマンドを送信する
    send_servo_driver_cmd(cmd, 2);

    // コマンドバッファはクリアしておく
    memset(cmd, 0x00, sizeof(cmd));

    // PWM パルス出力許可
    cmd[0] = 2; // cmdno
    cmd[1] = 3; // bOut
    *(uint16_t*)(&(cmd[2])) = 1;

    // コマンドを送信する
    send_servo_driver_cmd(cmd, 4);
    delay(100);
}
void loop() {
    uint8_t cmd[10]; // 送信電文格納バッファ
    uint8_t reply[10]; // 応答電文格納バッファ
    uint16_t AngleHD; // 取得した角度(1=1/10 度)

    // コマンドバッファはクリアしておく
    memset(cmd, 0x00, sizeof(cmd));

    // モータの角度を 0 度にセット
    cmd[0] = 4; // cmdno
    cmd[1] = 3; // ch
    *(uint16_t*)(&(cmd[2])) = 0; // AngleHD 角度(1=1/10 度)
    *(uint16_t*)(&(cmd[4])) = 1000; // mTime 動作時間(msec)

    // コマンドを送信する
    send_servo_driver_cmd(cmd, 6);

    // コマンドバッファはクリアしておく
    memset(cmd, 0x00, sizeof(cmd));

    // モータ一斉スタート
    cmd[0] = 1; // cmdno

    // コマンドを送信する
    send_servo_driver_cmd(cmd, 1);
    delay(1000);

    // コマンドバッファはクリアしておく
    memset(cmd, 0x00, sizeof(cmd));

    // モータの現在座標を取得する
    cmd[0] = 6; // cmdno
    cmd[1] = 3; // ch

    // コマンドを送信する
    send_servo_driver_cmd(cmd, 2);

    // 応答を受信する
    recv_servo_driver_reply(reply, 2);
    AngleHD = *(uint16_t*)reply;
    Serial.print("Angle: ");
    Serial.println(AngleHD);

    // コマンドバッファはクリアしておく
    memset(cmd, 0x00, sizeof(cmd));

    // モータの角度を 180 度にセット
    cmd[0] = 4; // cmdno
    cmd[1] = 3; // ch
    *(uint16_t*)(&(cmd[2])) = 1800; // AngleHD 角度(1=1/10 度)
    *(uint16_t*)(&(cmd[4])) = 1000; // mTime 動作時間(msec)

    // コマンドを送信する
    send_servo_driver_cmd(cmd, 6);

    // コマンドバッファはクリアしておく
    memset(cmd, 0x00, sizeof(cmd));

    // モータ一斉スタート
    cmd[0] = 1; // cmdno

    // コマンドを送信する
    send_servo_driver_cmd(cmd, 1);
    for (int j = 0; j < 10; j++) {
        delay(100);

        // コマンドバッファはクリアしておく
        memset(cmd, 0x00, sizeof(cmd));

        // モータの現在座標を取得する
        cmd[0] = 6; // cmdno
        cmd[1] = 3; // ch

        // コマンドを送信する
        send_servo_driver_cmd(cmd, 2);

        // 応答を受信する
        recv_servo_driver_reply(reply, 2);
        AngleHD = *(uint16_t*)reply;
        Serial.print("Angle: ");
        Serial.println(AngleHD);
    }
    delay(400);

    // コマンドバッファはクリアしておく
    memset(cmd, 0x00, sizeof(cmd));

    // モータの角度を 90 度にセット
    cmd[0] = 4; // cmdno
    cmd[1] = 3; // ch
    *(uint16_t*)(&(cmd[2])) = 900; // AngleHD 角度(1=1/10 度)
    *(uint16_t*)(&(cmd[4])) = 1000; // mTime 動作時間(msec)

    // コマンドを送信する
    send_servo_driver_cmd(cmd, 6);

    // コマンドバッファはクリアしておく
    memset(cmd, 0x00, sizeof(cmd));

    // モータ一斉スタート
    cmd[0] = 1; // cmdno
    
    // コマンドを送信する
    send_servo_driver_cmd(cmd, 1);
    delay(1000);
    delay(100);
}
