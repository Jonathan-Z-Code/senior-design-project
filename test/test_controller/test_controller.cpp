#include <Arduino.h>
#include "unity.h"
#include "global_settings.h"
#include "esp_now_api.h"

// mac addr for senior design car: a0:a3:b3:96:78:28
const uint8_t car_mac_addr[6] = {0xA0, 0xA3, 0xB3, 0x96, 0x78, 0x28};

// object of EspNowController class
EspNowController controller;

// callback function for esp_now transmitted data
void IRAM_ATTR data_sent_cb(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("\r\nLast Packet Send Status:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "SUCCESS" : "FAILURE");
    return;
}

// callback function for esp_now data reception
void IRAM_ATTR data_receive_cb(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    // store data into recv_message struct derived from EspNowRecv class 
    static DRAM_ATTR EspNowRecv::recv_message debug_data = {0};

    // copy recv data and print out results
    memcpy(&debug_data, data, sizeof(debug_data));

    /* TELEMETRY LEGEND: 
    *   tL = target left motor value
    *   tR = target right motor value
    *   tT = target trim motor value
    *   cL = current left motor value
    *   cR = current right motor value
    *   cT = current trim motor value
    */
    Serial.println("TELEMETRY DATA:");
    Serial.printf("tL: %u | ", debug_data.targetLeftValue);
    Serial.printf("tR: %u | ", debug_data.targetRightValue);
    Serial.printf("tT: %u | ", debug_data.targetTrimValue);
    Serial.printf("cL: %u | ", debug_data.leftMotorValue);
    Serial.printf("cR: %u | ", debug_data.rightMotorValue);
    Serial.printf("cT: %u\n" , debug_data.trimMotorValue);

    return;
}

void setup(void) {
    delay(3000);                    // unity overhead delay
    Serial.begin(115200);           // set proper baud rate
    pinMode(TRIM_MOTOR_PIN, INPUT); // The trim motor pin is an input because it is a button that determines the speed of the trim motor
    controller.init(car_mac_addr, data_sent_cb, data_receive_cb); // initialize the controller for ESP_NOW communication with the car
}

void loop(void) {
    
    // variables to keep track of speed and PWM values
    uint8_t l_val = 0;
    uint8_t r_val = 0;
    uint8_t speed_result = 0;
    int16_t trim_pwm = 0;   // variable to store trim_pwm
    float kp = 0.9f;
    unsigned int curr_time = millis();

    while(1) {
        if(millis() - curr_time >= CONTROLLER_UPDATE_INTERVAL) {
            // variables to keep track of raw ADC readings
            analogReadResolution(8);
            uint8_t joystick_result  = analogRead(SPEED_PIN);
            uint8_t linear_result = analogRead(OFFSET_PIN);
            int8_t  offset_centered = 127 - linear_result; // center the offset around 0
            
            // check if the joystick is outside of the deadzone
            if(joystick_result > (JOYSTICK_DEADZONE + 128) || joystick_result < (-JOYSTICK_DEADZONE + 128)) {
                speed_result = joystick_result;
            }   
            else { /* controller is within deadzone */
                speed_result = 0;
            }

            // Calculate whether the offset (centered at 0) is biased for left or right motor
            if(offset_centered < 0) { // right motor bias
                uint8_t abs_offset = (uint8_t)(-1 * offset_centered);
                controller.control_data->left_pwm  = speed_result;

                // IMPORTANT NOTE:
                // This algorithm basically takes the offset and divides it by max offset (basically a percentage of the offset)
                // Now this offset percent value is multiplied by the speed_result (in order to have a relative value to subtract)
                // Lastly, it is multiplied again by 2 since the range of offset_centered is -127 to 0 for each if statement
                // Last, multiply by a constant of proportionality in order to soften the biasing as desired
                controller.control_data->right_pwm = speed_result - (float)(2 * speed_result * abs_offset / 256) * kp; 
            }
            else {
                // left motor bias
                controller.control_data->left_pwm  = speed_result - (float)(2 * speed_result * offset_centered / 256) * kp;
                controller.control_data->right_pwm = speed_result;
            }

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
            
            // assign values to the control data struct
            controller.control_data->trim_pwm = (uint8_t)trim_pwm;

            #if DEBUG_CONTROLLER_DATA
            // DEBUG: print out the trim_pwm variable to check
            Serial.printf("Final TRIM_PWM: %d \n", controller.control_data->trim_pwm);
            Serial.printf("ADC values: JOYSTICK: %u, LINEAR: %u \n", joystick_result, linear_result);
            Serial.printf("Final PWM values: L: %d, R: %d \n", controller.control_data->left_pwm, controller.control_data->right_pwm);
            #endif

            controller.send(); // send controller values to the ESPNOW receiver
            
            curr_time = millis();
        }
    }
}
