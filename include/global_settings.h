#ifndef GLOBAL_SETTINGS_H_
#define GLOBAL_SETTINGS_H_

// Choose whether or not to print controller data 
#define DEBUG_CONTROLLER_DATA (0)

// #defines for PWM update interval (in milliseconds) and step size
#define RECV_UPDATE_INTERVAL (50) 
#define LEFT_PWM_STEP_SIZE   (2)
#define RIGHT_PWM_STEP_SIZE  (2)
#define TRIM_PWM_STEP_SIZE   (2)

// #defines for motor pinout
#define LEFT_MOTOR_PWM  (23)
#define RIGHT_MOTOR_PWM (22)
#define TRIM_MOTOR_PWM  (13)

// interval to send user inputs to the car (in milliseconds)
#define CONTROLLER_UPDATE_INTERVAL (150) 

// #defines for user input GPIO and deadzone
#define SPEED_PIN         (32)
#define OFFSET_PIN        (33)
#define JOYSTICK_DEADZONE (25)

// #defines for trim motor button input and step size
#define TRIM_MOTOR_PIN           (25)
#define TRIM_MOTOR_PWM_STEP_SIZE (5)

#endif /* GLOBAL_SETTINGS_H_ */