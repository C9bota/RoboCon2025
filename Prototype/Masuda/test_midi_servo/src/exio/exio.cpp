#include <Wire.h>
#include "../../lib/exio/exio.h"

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

// IOエキスパンダのレジスタライト(ベリファイ機能付き)
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

// IOエキスパンダのレジスタリード
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

// IOエキスパンダ出力(1ピンだけ設定)
void WriteIoExpanderPin(int pin_no, uint8_t value) {
    uint8_t bitmask = (1 << pin_no);

    // Read-Modify-Writeをする
    WriteIoExpanderPins(bitmask, (value << pin_no));

    return;
}

// IOエキスパンダ出力(複数ピンを同時設定)
void WriteIoExpanderPins(uint8_t mask, uint8_t value) {
    uint8_t current_olat_value = 0;
    uint8_t next_olat_value = 0;

    // 現在のOLATレジスタの値を読み出して、Read-Modify-Writeをする
    current_olat_value = ReadIoExpanderReg(REG_IOEX_OLAT);
    next_olat_value = (~mask & current_olat_value) | (mask & value);
    WriteIoExpanderReg(REG_IOEX_OLAT, next_olat_value, false);

    return;
}

// IOエキスパンダ入力(1ピンだけ読む)
uint8_t ReadIoExpanderPin(int pin_no) {
    uint8_t all_pin_value = ReadIoExpanderPins();

    return ((all_pin_value >> pin_no) & 0x01);
}

// IOエキスパンダ入力(複数ピンをまとめて読む)
uint8_t ReadIoExpanderPins(void) {
    return ReadIoExpanderReg(REG_IOEX_GPIO);
}
