#include <Arduino.h>
#include <IRremote.h>

#define IR_RECEIVE_PIN 4
#define R_PIN 9
#define G_PIN 6
#define B_PIN 5

int colors[] = {0, 0, 0};
int deltas[] = {1, 1, 1};
constexpr int COLOR_PINS[] = {R_PIN, G_PIN, B_PIN};

int speed = 10;
bool pwmEnabled = false;
bool blinkEnabled = false;
unsigned long nextPwmUpdate;
unsigned long nextBlinkUpdate;

uint16_t getCommand() {
    const IRData data = IrReceiver.decodedIRData;
    return data.flags == 1 ? 0 : data.command;
}

void updateRGB() {
    for (int i = 0; i < 3; i++) {
        analogWrite(COLOR_PINS[i], colors[i]);
    }
}

void invertDelta(const int i) {
    deltas[i] = deltas[i] != 0 ? 0 : 1;
}

void invertLed(const int i) {
    colors[i] = colors[i] > 0 ? 0 : 255;
}

void pwmUpdate(const int i) {
    int c = colors[i];
    c += deltas[i];
    if (c < 0) {
        c = 0;
        deltas[i] = 1;
    }
    else if (c > 255) {
        c = 255;
        deltas[i] = -1;
    }
    colors[i] = c;
}

void blinkUpdate(const int i) {
    if (deltas[i] != 0) {
        invertLed(i);
    }
}

void setup() {
    for (const int pin: COLOR_PINS) {
        pinMode(pin, OUTPUT);
    }

    Serial.begin(9600);
    IrReceiver.begin(IR_RECEIVE_PIN);
    Serial.println(F("Executing XmasTree"));
}

void loop() {
    if (pwmEnabled) {
        if (millis() >= nextPwmUpdate) {
            for (int i = 0; i < 3; i++) {
                pwmUpdate(i);
            }
            nextPwmUpdate = millis() + speed;
        }
    }
    else if (blinkEnabled) {
        if (millis() >= nextBlinkUpdate) {
            for (int i = 0; i < 3; i++) {
                blinkUpdate(i);
            }
            nextBlinkUpdate = millis() + speed * 100;
        }
    }
    if (IrReceiver.decode()) {
        const uint16_t cmd = getCommand();
        switch (cmd) {
            case 0: break;
            case 28: {
                // OK
                Serial.println(F("HALT"));
                pwmEnabled = false;
                blinkEnabled = false;
                for (int i = 0; i < 3; i++) {
                    colors[i] = 0;
                    deltas[i] = 1;
                }
                break;
            }
            case 25: {
                // 0
                Serial.println(F("PWM"));
                pwmEnabled = !pwmEnabled;
                break;
            }
            case 22: {
                // *
                Serial.println(F("RGB OFF"));
                for (int & c : colors) {
                    c = 0;
                }
                break;
            }
            case 13: {
                // #
                Serial.println(F("RGB RNG"));
                for (int & c : colors) {
                    c = static_cast<int>(random(0, 255));
                }
                break;
            }
            case 24: {
                // UP
                Serial.println(F("DELTAS OFF"));
                for (int & d : deltas) {
                    d = 0;
                }
                break;
            }
            case 8: {
                // LEFT
                Serial.println(F("RD SWITCH"));
                invertDelta(0);
                break;
            }
            case 82: {
                // DOWN
                Serial.println(F("GD SWITCH"));
                invertDelta(1);
                break;
            }
            case 90: {
                // RIGHT
                Serial.println(F("BD SWITCH"));
                invertDelta(2);
                break;
            }
            case 7: {
                // 7
                Serial.println(F("R SWITCH"));
                invertLed(0);
                break;
            }
            case 21: {
                // 8
                Serial.println(F("G SWITCH"));
                invertLed(1);
                break;
            }
            case 9: {
                // 9
                Serial.println(F("B SWITCH"));
                invertLed(2);
                break;
            }
            case 69: {
                // 1
                Serial.println(F("SPEED 5"));
                speed = 5;
                break;
            }
            case 70: {
                // 2
                Serial.println(F("SPEED 10"));
                speed = 10;
                break;
            }
            case 71: {
                // 3
                Serial.println(F("SPEED 25"));
                speed = 25;
                break;
            }
            case 64: {
                // 5
                Serial.println(F("BLINK"));
                blinkEnabled = !blinkEnabled;
                break;
            }
            case 68: {
                // 4
                Serial.println(F("SPEED 1"));
                speed = 1;
                break;
            }
            case 67: {
                // 6
                Serial.println(F("SPEED 3"));
                speed = 3;
                break;
            }
            default: {
                Serial.print("UNKNOWN: ");
                Serial.println(cmd);
                break;
            }
        }

        IrReceiver.resume();
    }
    updateRGB();
}
