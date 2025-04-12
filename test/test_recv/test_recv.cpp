#include "control_api.h"
#include "esp_now_api.h"
#include <Arduino.h>

// #defines for PWM update interval and step size
#define UPDATE_INTERVAL (5)
#define LEFT_PWM_STEP_SIZE  (2)
#define RIGHT_PWM_STEP_SIZE (2)

// #defines for motor pinout
#define LEFT_MOTOR_PWM  (23)
#define RIGHT_MOTOR_PWM (22)

// user defined mac addresses
const uint8_t controller_address[MAC_ADDR_LEN] = {0xA0, 0xA3, 0xB3, 0x96, 0x6E, 0x40};
const uint8_t recv_address[MAC_ADDR_LEN] = {0xA0, 0xA3, 0xB3, 0x96, 0x6E, 0xF8};

// variables to keep track of target PWM values
volatile uint16_t target_left_pwm  = 0;
volatile uint16_t target_right_pwm = 0;

// create an object from the EspNowRecv class
EspNowRecv recv;

// callback function for esp_now transmitted data
void IRAM_ATTR data_sent_cb(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("\r\nLast Packet Send Status:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "SUCCESS" : "FAILURE");
    return;
}

// callback function for esp_now data reception
void IRAM_ATTR data_receive_cb(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    // store data into recv_message struct derived from EspNowRecv class 
    static EspNowController::controller_message control_data = {0};

    // print out data from the controller for debug purposes
    memcpy(&control_data, data, sizeof(control_data));
    Serial.println("PACKET RECEIVED: ");
    Serial.println(control_data.left_pwm);
    Serial.println(control_data.right_pwm);

    // update the pwm values for the left and right wheels
    target_left_pwm  = control_data.left_pwm;
    target_right_pwm = control_data.right_pwm;

    return;
}

void setup(void) {
 
    Serial.begin(115200);

    // init the recv object with peer mac addr and callback functions
    recv.init(controller_address, data_sent_cb, data_receive_cb);
    
    // store updated debug_data to the object's struct
    recv.debug_data->deltaLeft = 1;
    recv.debug_data->deltaRight = 1;
    recv.debug_data->leftMotorValue = 1;
    recv.debug_data->rightMotorValue = 1;
    recv.debug_data->targetValue = 1;

    // send the data OTA to peer address stated in recv.init()
    recv.send();

}

void loop(void) {
    // do nothing
    Serial.println("INTO MAIN LOOP ...");
    unsigned long start_time = millis();
    uint16_t actual_left_pwm  = 0;
    uint16_t actual_right_pwm = 0;

    while(1) {
        if(millis() - start_time >= UPDATE_INTERVAL) {

            // adjust the left PWM value
            if(actual_left_pwm < target_left_pwm) actual_left_pwm += LEFT_PWM_STEP_SIZE;
            if(actual_left_pwm > target_left_pwm) actual_left_pwm -= LEFT_PWM_STEP_SIZE;
        
            analogWrite(LEFT_MOTOR_PWM, actual_left_pwm);

            // adjust the right PWM value
            if(actual_right_pwm < target_right_pwm) actual_right_pwm += RIGHT_PWM_STEP_SIZE;
            if(actual_right_pwm > target_right_pwm) actual_right_pwm -= RIGHT_PWM_STEP_SIZE;

            analogWrite(RIGHT_MOTOR_PWM, actual_right_pwm);

            start_time = millis();
        }
    }
}
