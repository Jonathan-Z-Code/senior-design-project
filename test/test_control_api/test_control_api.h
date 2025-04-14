#include <string.h>
#include "esp32-hal-adc.h"

typedef struct pwm_config_t {
    uint8_t left_motor_pin;
    uint8_t right_motor_pin;
    uint16_t left_pwm_step_size;
    uint16_t right_pwm_step_size;
} pwm_config_t;


typedef struct updated_targets_t {
    uint16_t new_target_left;
    uint16_t new_target_right;
} updated_targets_t;


// definition of MotorControl class
class MotorControl {

    private:

    // private struct definition so nobody can create duplicate pwm settings 
    typedef struct pwm_settings_t {
        uint16_t actual_left;
        uint16_t actual_right;
        uint16_t target_left;
        uint16_t target_right;
    } pwm_settings;

    pwm_settings_t _pwm = {0};
    pwm_config_t _config = {0};

    public:

    // constructor
    MotorControl::MotorControl(pwm_config_t* user_config) {
        analogReadResolution(8);
        memcpy(&_config, user_config, sizeof(pwm_config_t));
    }

    // public function to update motor values
    void update_target_values(updated_targets_t target) {
        _pwm.target_left  = target.new_target_left;
        _pwm.target_right = target.new_target_right;
    }

    // public function to run control loop (user MUST implement their own time delay for control loop to work properly)
    void control_loop(void) {

        // adjust the left PWM value
        if(_pwm.actual_left < _pwm.target_left) _pwm.actual_left += _config.left_pwm_step_size;
        if(_pwm.actual_left > _pwm.target_left) _pwm.actual_left -= _config.left_pwm_step_size;

        // adjust the right PWM value
        if(_pwm.actual_right < _pwm.target_right) _pwm.actual_right += _config.right_pwm_step_size;
        if(_pwm.actual_right > _pwm.target_right) _pwm.actual_right -= _config.right_pwm_step_size;

        // update both motor PWM values;
        analogWrite(_config.left_motor_pin, _pwm.actual_left);
        analogWrite(_config.right_motor_pin, _pwm.actual_right);

    }

};

// user defined config settings 
typedef struct user_control_config_t {
    uint8_t joystick_pin;
    uint8_t linear_potentiometer_pin;
    uint8_t joystick_deadzone;
    float kp;
} user_control_config_t;

// definition of UserControl class
class UserControl {

    private:

    user_control_config_t _config = {0};

    public:

    // constructor
    UserControl::UserControl(user_control_config_t* user_config) {
        analogReadResolution(8);
        memcpy(&_config, user_config, sizeof(user_control_config_t));
    }

    // parse user input and return new target values 
    updated_targets_t parse_user_input(void) {
        
        // create a struct in order to return the calculated values
        updated_targets_t return_values = {0};

        // store ADC readings in the respective variables
        uint16_t joystick_result  = analogRead(_config.joystick_pin);
        uint16_t linear_result    = analogRead(_config.linear_potentiometer_pin);

        int16_t  offset_centered = 128 - linear_result; // center the offset around 0
        uint16_t speed_result = 0;

        // check if the joystick is outside of the deadzone
        if(joystick_result > (_config.joystick_deadzone + 128) || joystick_result < (-(_config.joystick_deadzone) + 128)) {
            speed_result = joystick_result;
        }   
        else { /* controller is within deadz)one */
            speed_result = 0;
        }
    
        // Calculate whether the offset (centered at 0) is biased for left or right motor
        if(offset_centered < 0) { // right motor bias
            
            uint16_t abs_offset = -1 * offset_centered;
            return_values.new_target_left = speed_result;
    
            // IMPORTANT NOTE:
            // This algorithm basically takes the offset and divides it by max offset (basically a percentage of the offset)
            // Now this offset percent value is multiplied by the speed_result (in order to have a relative value to subtract)
            // Lastly, it is multiplied again by 2 since the range of offset_centered is -127 to 0 for each if statement
            // Last, multiply by a constant of proportionality in order to soften the biasing as desired
            return_values.new_target_right = speed_result - (float)(2 * speed_result * abs_offset / 256) * _config.kp; 
        }
        else {
            // left motor bias
            return_values.new_target_left = speed_result - (float)(2 * speed_result * offset_centered / 256) * _config.kp;
            return_values.new_target_right = speed_result;
        }

        return return_values;
    }
};