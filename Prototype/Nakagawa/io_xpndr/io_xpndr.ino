#include <Wire.h>

/*
*   定数定義
*/
// I2Cスレーブアドレス
constexpr uint8_t I2C_SLV_ADR_IOEX  = 0x20;

// レジスタアドレス
constexpr uint8_t REG_IOEX_IODIR    = 0x00;
constexpr uint8_t REG_IOEX_IPOL     = 0x01;
constexpr uint8_t REG_IOEX_GPINTEN  = 0x02;
constexpr uint8_t REG_IOEX_DEFVAL   = 0x03;
constexpr uint8_t REG_IOEX_INTCON   = 0x04;
constexpr uint8_t REG_IOEX_IOCON    = 0x05;
constexpr uint8_t REG_IOEX_IOCON_B0 = 0x0A; // BANK=0 (default on MCP23017)
constexpr uint8_t REG_IOEX_GPPU     = 0x06;
constexpr uint8_t REG_IOEX_INTF     = 0x07;
constexpr uint8_t REG_IOEX_INTCAP   = 0x08;
constexpr uint8_t REG_IOEX_GPIO     = 0x09;
constexpr uint8_t REG_IOEX_OLAT     = 0x0A;

constexpr uint8_t RC25_IOEX_IOCON   = 0b10100000;   // [7] BANK=1, [5] SEQOP=1
constexpr uint8_t RC25_IOEX_PORTDIR = 0b00000000;   // IO0とIO1は入力、それ以外は出力

typedef enum {
    DEV_MCP23008 = 0,
    DEV_MCP23017
} IoExpanderDev;

// プロトタイプ宣言
uint8_t InitIoExpander(IoExpanderDev);
uint8_t WriteIoExpanderReg(uint8_t, uint8_t, bool);
uint8_t ReadIoExpanderReg(uint8_t);
void WriteIoExpanderPin(int, uint8_t);  // 1ピン版
void WriteIoExpanderPins(uint8_t, uint8_t); // 多ピン版
uint8_t ReadIoExpanderPin(int);         // 1ピン版
uint8_t ReadIoExpanderPins();            // 多ピン版


/*
*   IOエキスパンダの初期化
*   前提: Wire.begin()されている
*   戻り値: ベリファイして一致していれば0、不一致なら1
*/
uint8_t InitIoExpander(IoExpanderDev dev_type) {
    uint8_t ret_val = 1;

    // 制御レジスタ設定
    // BANK設定する前は、MCP23017とMCP23008でIOCONレジスタのアドレスが違うため分ける
    if (dev_type == DEV_MCP23017) {
        WriteIoExpanderReg(REG_IOEX_IOCON_B0, RC25_IOEX_IOCON, false);
    } else {
        WriteIoExpanderReg(REG_IOEX_IOCON, RC25_IOEX_IOCON, false);
    }

    // ポート方向を基板に合わせて初期化
    WriteIoExpanderReg(REG_IOEX_IODIR, RC25_IOEX_PORTDIR, false);

    ret_val =   (RC25_IOEX_IOCON != ReadIoExpanderReg(REG_IOEX_IOCON)) ||
                (RC25_IOEX_PORTDIR != ReadIoExpanderReg(REG_IOEX_IODIR));

    return ret_val;
}

uint8_t WriteIoExpanderReg(uint8_t addr, uint8_t data, bool verify) {
    uint8_t tx_buf[2] = {addr, data};
    uint8_t ret_val = 0;

    Wire.beginTransmission(I2C_SLV_ADR_IOEX);
    Wire.write(tx_buf, 2);
    Wire.endTransmission(true); // STOP condition

    if (verify) {
        uint8_t read_data = ReadIoExpanderReg(addr);
        ret_val = (read_data != data);
    }

    return ret_val;
}

uint8_t ReadIoExpanderReg(uint8_t addr) {
    uint8_t bytes_received = 0;
    uint8_t rx_buf = 0;

    Wire.beginTransmission(I2C_SLV_ADR_IOEX);
    Wire.write(addr);
    Wire.endTransmission(false); // RESTART condition
    bytes_received = Wire.requestFrom((uint8_t)I2C_SLV_ADR_IOEX, (uint8_t)1, (uint8_t)1); // set STOP Condition
    if (bytes_received > 0) {
        rx_buf = Wire.read();
    }

    return rx_buf;
}

void WriteIoExpanderPin(int pin_no, uint8_t value) {
    uint8_t bitmask = (1 << pin_no);

    // Read-Modify-Writeをする
    WriteIoExpanderPins(bitmask, (value << pin_no));

    return;
}

void WriteIoExpanderPins(uint8_t mask, uint8_t value) {
    uint8_t current_olat_value = 0;
    uint8_t next_olat_value = 0;

    // 現在のOLATレジスタの値を読み出して、Read-Modify-Writeをする
    current_olat_value = ReadIoExpanderReg(REG_IOEX_OLAT);
    next_olat_value = (~mask & current_olat_value) | (mask & value);
    WriteIoExpanderReg(REG_IOEX_OLAT, next_olat_value, false);

    return;
}

uint8_t ReadIoExpanderPin(int pin_no) {
    uint8_t all_pin_value = ReadIoExpanderPins();

    return ((all_pin_value >> pin_no) & 0x01);
}

uint8_t ReadIoExpanderPins(void) {
    return ReadIoExpanderReg(REG_IOEX_GPIO);
}

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    
    Wire.begin();
    Wire.setClock(100000);
    if (!InitIoExpander(DEV_MCP23017)) {
        Serial.println("Success!");
    } else {
        Serial.println("Failed!");
    }
}

void loop() {
    // put your main code here, to run repeatedly:
    WriteIoExpanderPin(0, 1);
    delay(100);
    WriteIoExpanderPin(0, 0);
    delay(400);
}
