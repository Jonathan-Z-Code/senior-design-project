#include <Arduino.h>
#include "unity.h"
#include "esp_int_wdt.h"

#define IN1_PIN (15)
#define IN2_PIN (2)

void setup(void) {
    delay(3000);
    Serial.begin(115200);
    pinMode(IN2_PIN, OUTPUT);
}

void loop(void) {
    // force PWM on a 8-bit scale   
    analogReadResolution(8);
    digitalWrite(IN2_PIN, LOW);

    while(1) {
    
    for(int i = 0 ; i <= 255 ; i++) {
        analogWrite(IN1_PIN, i); // 50% on
        delay(20);
    }

    for(int j = 255 ; j >= 0 ; j--) {
        analogWrite(IN1_PIN, j);
        delay(20);
    }

    }
    
}