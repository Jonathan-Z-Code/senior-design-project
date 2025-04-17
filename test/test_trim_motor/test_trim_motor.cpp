#include <Arduino.h>
#include "unity.h"

// user-defined pin assignment for the trim motor button
#define TRIM_MOTOR_PIN (25)

// period for reading the trim motor button (this is in milliseconds)
#define TRIM_MOTOR_READ_PERIOD (50)

// step size for trim motor PWM signal 
#define TRIM_MOTOR_PWM_STEP_SIZE (5)

void setup(void) {
    delay(3000); // unity framework required delay 
    Serial.begin(115200); // establish connection with UART0 
    pinMode(TRIM_MOTOR_PIN, INPUT); // assign input to the trim motor pin
}

void loop(void) {
    unsigned long currTime = millis();

    // I have chosen a signed 32-bit integer because
    // ESP32 is a 32-bit MCU, so it doesnt need special instructions to load
    // It can handle negative values and all values within a uint8_t (PWM should be within 0-255) 
    int32_t trim_pwm = 0;

    while(1) {
        if(millis() - currTime >= TRIM_MOTOR_READ_PERIOD) {
 
            // read the TRIM_MOTOR_PIN and update trimmer motor accordingly
            if(digitalRead(TRIM_MOTOR_PIN)) {
                trim_pwm += TRIM_MOTOR_PWM_STEP_SIZE;
            }
            else {
                trim_pwm -= TRIM_MOTOR_PWM_STEP_SIZE;
            }

            // bounds checking 
            if(trim_pwm < 0)    trim_pwm = 0;
            if(trim_pwm > 255)  trim_pwm = 255;

            // load next timestamp reference
            currTime = millis();

            // DEBUG: print out the trim_pwm variable to check
            Serial.printf("%u\n", trim_pwm);
        }
    }
    
    return;
}