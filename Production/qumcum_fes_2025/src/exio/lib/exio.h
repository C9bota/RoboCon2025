#ifndef EXIO_H
#define EXIO_H

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

#endif  // EXIO_H
