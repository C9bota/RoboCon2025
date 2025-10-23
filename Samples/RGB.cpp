void setup() {
    pinMode(32, OUTPUT); // RGB-LED(Red)
    pinMode(27, OUTPUT); // RGB-LED(Green)
    pinMode(33, OUTPUT); // RGB-LED(Blue)
}
void loop() {
    // RGB-LED(Red) ON
    digitalWrite(32, HIGH);
    // Wait 200msec
    delay(200);
    // RGB-LED(Red) OFF
    digitalWrite(32, LOW);
    // Wait 200msec
    delay(200);

    // RGB-LED(Green) ON
    digitalWrite(27, HIGH);
    // Wait 200msec
    delay(200);
    // RGB-LED(Green) OFF
    digitalWrite(27, LOW);
    // Wait 200msec
    delay(200);
    // RGB-LED(Blue) ON
    digitalWrite(33, HIGH);
    // Wait 200msec
    delay(200);
    // RGB-LED(Blue) OFF
    digitalWrite(33, LOW);
    // Wait 200msec
    delay(200);
}