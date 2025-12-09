#include <Wire.h>
#include "src/exio/lib/exio.h"

void setup() {
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
    WriteIoExpanderPin(0, 1);   // GPA0端子をHにする
    delay(100);
    WriteIoExpanderPin(0, 0);   // GPA0端子をLにする
    delay(400);
}
