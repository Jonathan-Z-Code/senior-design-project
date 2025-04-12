#include <Arduino.h>
#include "unity.h"
#include "esp_now_api.h"

#define SPEED_PIN (32)
#define OFFSET_PIN (33)
#define JOYSTICK_DEADZONE (25)

// mac addr for senior design car: a0:a3:b3:96:78:28
const uint8_t car_mac_addr[6] = {0xA0, 0xA3, 0xB3, 0x96, 0x78, 0x28};

// object of EspNowController class
EspNowController controller;

// callback function for esp_now transmitted data
void data_sent_cb(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("\r\nLast Packet Send Status:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "SUCCESS" : "FAILURE");
    return;
}

// callback function for esp_now data reception
void data_receive_cb(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    // store data into recv_message struct derived from EspNowRecv class 
    static EspNowRecv::recv_message debug_data = {0};

    // copy recv data and print out results
    memcpy(&debug_data, data, sizeof(debug_data));
    Serial.println("PACKET RECEIVED: ");
    Serial.println(debug_data.deltaLeft);
    return;
}

void setup(void) {
    delay(3000); // unity overhead delay
    Serial.begin(115200);
    controller.init(car_mac_addr, data_sent_cb, data_receive_cb);
}

void loop(void) {
    
    delay(250); // read ADC vals and print them out every 1 second 

    // variables to keep track of speed and PWM values
    uint16_t l_val = 0;
    uint16_t r_val = 0;
    uint16_t speed_result = 0;
    float kp = 0.9f;

    // variables to keep track of raw ADC readings
    analogReadResolution(8);
    uint16_t joystick_result  = analogRead(SPEED_PIN);
    uint16_t linear_result = analogRead(OFFSET_PIN);
    int16_t  offset_centered = 128 - linear_result; // center the offset around 0
    
    // check if the joystick is outside of the deadzone
    if(joystick_result > (JOYSTICK_DEADZONE + 128) || joystick_result < (-JOYSTICK_DEADZONE + 128)) {
        speed_result = joystick_result;
    }   
    else { /* controller is within deadzone */
        speed_result = 0;
    }

    // Calculate whether the offset (centered at 0) is biased for left or right motor
    if(offset_centered < 0) { // right motor bias
        uint16_t abs_offset = -1 * offset_centered;
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

    Serial.printf("ADC values: JOYSTICK: %d, LINEAR: %d \n", joystick_result, linear_result);
    Serial.printf("Final L/R PWM values: L: %d, R: %d \n", controller.control_data->left_pwm, controller.control_data->right_pwm);

    controller.send(); // send controller values to the ESPNOW receiver
}
