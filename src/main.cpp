#include <Arduino.h>
#include <IRremote.h>

#define IR_RECEIVE_PIN 4
#define R_PIN 9
#define G_PIN 6
#define B_PIN 5

int r, g, b, rd = 1, gd = 1, bd = 1;
constexpr int OUTPUT_PINS[] = {R_PIN, G_PIN, B_PIN};

int speed = 10;
bool pwmEnabled = false;
bool blinkEnabled = false;
unsigned long nextPwmUpdate;
unsigned long nextBlinkUpdate;

uint16_t getCommand() {
    const IRData data = IrReceiver.decodedIRData;
    return data.flags == 1 ? 0 : data.command;
}

int irandom(const int a, const int b) {
    return static_cast<int>(random(a, b));
}

void updateRGB() {
    analogWrite(R_PIN, r);
    analogWrite(G_PIN, g);
    analogWrite(B_PIN, b);
}

void invertDelta(int&d) {
    d != 0 ? d = 0 : d = 1;
}

void invertLed(int&led) {
    led > 0 ? led = 0 : led = 255;
}

void pwmUpdate(int&v, int&d) {
    v += d;
    if (v < 0) {
        v = 0;
        d = 1;
    }
    else if (v > 255) {
        v = 255;
        d = -1;
    }
}

void blinkUpdate(int&v, const int d) {
    if (d != 0) {
        invertLed(v);
    }
}

void setup() {
    for (const int pin: OUTPUT_PINS) {
        pinMode(pin, OUTPUT);
    }

    Serial.begin(9600);
    IrReceiver.begin(IR_RECEIVE_PIN);
    Serial.println(F("Executing XmasTree"));
}

void loop() {
    if (pwmEnabled) {
        if (millis() >= nextPwmUpdate) {
            pwmUpdate(r, rd);
            pwmUpdate(g, gd);
            pwmUpdate(b, bd);
            nextPwmUpdate = millis() + speed;
        }
    }
    else if (blinkEnabled) {
        if (millis() >= nextBlinkUpdate) {
            blinkUpdate(r, rd);
            blinkUpdate(g, gd);
            blinkUpdate(b, bd);
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
                r = g = b = 0;
                rd = gd = bd = 1;
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
                r = g = b = 0;
                break;
            }
            case 13: {
                // #
                Serial.println(F("RGB RNG"));
                r = irandom(0, 255);
                g = irandom(0, 255);
                b = irandom(0, 255);
                break;
            }
            case 24: {
                // UP
                Serial.println(F("DELTAS OFF"));
                rd = gd = bd = 0;
                break;
            }
            case 8: {
                // LEFT
                Serial.println(F("RD SWITCH"));
                invertDelta(rd);
                break;
            }
            case 82: {
                // DOWN
                Serial.println(F("GD SWITCH"));
                invertDelta(gd);
                break;
            }
            case 90: {
                // RIGHT
                Serial.println(F("BD SWITCH"));
                invertDelta(bd);
                break;
            }
            case 7: {
                // 7
                Serial.println(F("R SWITCH"));
                invertLed(r);
                break;
            }
            case 21: {
                // 8
                Serial.println(F("G SWITCH"));
                invertLed(g);
                break;
            }
            case 9: {
                // 9
                Serial.println(F("B SWITCH"));
                invertLed(b);
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
