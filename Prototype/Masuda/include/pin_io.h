# ifdef PIN_IO
# define PIN_IO

/*
    参照：「Qumcum R321J Main Board Product Manua」p.39
    https://qumcum.com/wp-content/uploads/2022/08/r321j_board_manual.pdf
*/

enum PIN_IO {
    SERIAL_RTS = 0,
    USB_SERIAL_RXD = 1,
    // NOT_CONNECTED : 2,
    USB_SERIAL_TXD = 3,
    BEEP = 4,
    ULTRASONIC_TRIGGER = 5,
    // NOT_CONNECTED : 6,
    // NOT_CONNECTED : 7,
    // NOT_CONNECTED : 8,
    // NOT_CONNECTED : 9,
    PAD_SPI_SCK = 10,
    PAD_SPI_MISO = 11,
    // NOT_CONNECTED : 12,
    // NOT_CONNECTED : 13,
    // NOT_CONNECTED : 14,
    PAD_SPI_MOSI = 15,
    MIC_BCLK = 16,
    MIC_LRCL = 17,
    LED3_FOR_BLUETOOTH_LINK_INDICATOR = 18,
    // NOT_CONNECTED : 19,
    // NOT_DEFINED : 20,
    I2C_SDA = 21,
    I2C_SCL = 22, 
    // NOT_CONNECTED : 23,
    // NOT_DEFINED : 24,
    VOICE = 25,
    MIC_DOUT = 26,
    GREEN = 27,
    // NOT_DEFINED : 28,
    // NOT_DEFINED : 29,
    // NOT_DEFINED : 30,
    // NOT_DEFINED : 31,
    RED = 32,
    BLUE = 33,
    BATTERY_ROWER = 34,
    ULTRASONIC_ECHOR = 35,
    // NOT_CONNECTED : 36,
    // NOT_DEFINED : 37,
    // NOT_DEFINED : 38,
    SS = 39
}


# endif
