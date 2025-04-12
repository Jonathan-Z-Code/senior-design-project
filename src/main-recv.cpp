/* 
    ---------------------------------------- ESPNOWRECV FUNCTIONALITY ----------------------------------------------------------
    
    Pre-reqs: The only thing the user needs to provide are mac addresses and callback functions! No globals! Nice!
    
    Why Bother? Previous codebase was a mess, time to harness the power of C++ and use encapsulation and abstraction via classes

    Author: Jonathan Zurita (jzuritastem@gmail.com)

    Any suggestions welcome!
    ----------------------------------------------------------------------------------------------------------------------------


    ---------------------------------------- MOTORCONTROL FUNCTIONALITY --------------------------------------------------------

    Pre-reqs: The only thing needed is to provide which pins are used for encoder readings and PWM output

    Why Bother? Previous codebase was a mess, time to harness the power of C++ and use encapsulation

    Author: Jonathan Zurita (jzuritastem@gmail.com)

    Any suggestions welcome!
    ----------------------------------------------------------------------------------------------------------------------------
    
*/

#include "control_api.h"
#include "esp_now_api.h"
#include "gps_api.h"
#include <Arduino.h>

// user defined mac addresses
const uint8_t controller_address[MAC_ADDR_LEN] = {0xA0, 0xA3, 0xB3, 0x96, 0x6E, 0x40};
const uint8_t recv_address[MAC_ADDR_LEN] = {0xA0, 0xA3, 0xB3, 0x96, 0x6E, 0xF8};

// create an object from the EspNowRecv class
EspNowRecv recv;

// create an object from the MotorControl class
MotorControl motor;

// callback function for esp_now transmitted data
void data_sent_cb(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("\r\nLast Packet Send Status:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "SUCCESS" : "FAILURE");
    return;
}

// callback function for esp_now data reception
void data_receive_cb(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    // store data into recv_message struct derived from EspNowRecv class 
    static EspNowController::controller_message control_data = {0};

    memcpy(&control_data, data, sizeof(control_data));
    Serial.println("PACKET RECEIVED: ");
    Serial.println(control_data.left_pwm);
    Serial.println(control_data.right_pwm);

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
    delay(10000);
}
