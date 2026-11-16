#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <math.h>

class Motor {
    public:
        Motor(uint pin_positive, uint pin_negative) 
        :   pin_positive(pin_positive), 
            pin_negative(pin_negative)
            {
            //set pins to pwm
            gpio_set_function(pin_positive, GPIO_FUNC_PWM);
            gpio_set_function(pin_negative, GPIO_FUNC_PWM);

            //get the pwm slice for each pin, 8 slices in total, each slice has 2 channels, A and B
            pwm_slice_positive = pwm_gpio_to_slice_num(pin_positive);
            pwm_slice_negative = pwm_gpio_to_slice_num(pin_negative);

            //enable the pwm for each slice, this will start the pwm signal on the pins
            pwm_set_enabled(pwm_slice_positive, true);
            pwm_set_enabled(pwm_slice_negative, true);

            //set the pwm frequency to 1kHz, this will set the period of the pwm signal to 1ms
            //this doesn't feel right, check later #TODO
            pwm_set_clkdiv(pwm_slice_positive, 125); // 125MHz?
            pwm_set_clkdiv(pwm_slice_negative, 125);
            
            //set the pwm wrap value to 1000, this will set the duty cycle of the pwm signal to 0-100%
            pwm_set_wrap(pwm_slice_positive, 1000); //1kHz
            pwm_set_wrap(pwm_slice_negative, 1000);

        }

        //set the speed of the motor using PWM
        void setSpeed(int speed) {
            speed = speed > 1000 ? 1000 : speed; //cap the speed to 1000, at max dudty cycle

            switch (speed) {
                case 0: //no current or voltage accross the motor
                    pwm_set_gpio_level(pin_positive, 0);
                    pwm_set_gpio_level(pin_negative, 0);
                    break;
                default:
                    switch (speed > 0) {
                        case true: //forward
                            pwm_set_gpio_level(pin_positive, speed);
                            pwm_set_gpio_level(pin_negative, 0);
                            break;
                        case false: //reverse
                            pwm_set_gpio_level(pin_positive, 0);
                            pwm_set_gpio_level(pin_negative, speed);
                            break;
                    }
                    break;
            }
        }

        //set the motor to brake mode, voltage and no current accross the motor
        void setBrake() {
            pwm_set_gpio_level(pin_positive, 1000);
            pwm_set_gpio_level(pin_negative, 1000);
        }

    private:
        uint pin_positive;
        uint pin_negative;

        uint pwm_slice_positive;
        uint pwm_slice_negative;
};

class chassis {
    public:
        chassis(Motor front_left_motor, Motor front_right_motor, 
            Motor back_left_motor, Motor back_right_motor)
        :   front_left_motor(front_left_motor),
            front_right_motor(front_right_motor),
            back_left_motor(back_left_motor),
            back_right_motor(back_right_motor) {}

    void move( int magnitude, int direction) {
        //set the individual motor speeds based on the magnitude and direction of the movement
        front_left_speed = magnitude * cos(direction + M_PI/4);
        front_right_speed = magnitude * cos(direction - M_PI/4);
        back_left_speed = magnitude * cos(direction - M_PI/4);
        back_right_speed = magnitude * cos(direction + M_PI/4);

        front_left_motor.setSpeed(front_left_speed);
        front_right_motor.setSpeed(front_right_speed);
        back_left_motor.setSpeed(back_left_speed);
        back_right_motor.setSpeed(back_right_speed);
    }

    private:
        Motor front_left_motor;
        Motor front_right_motor;
        Motor back_left_motor;
        Motor back_right_motor;

        int front_left_speed;
        int front_right_speed;
        int back_left_speed;
        int back_right_speed;

};

class washer_handler {
    public:
        washer_handler(uint arm_servo_pin, uint basket_servo_pin, uint electromagnet_pin)
        :   arm_servo_pin(arm_servo_pin),
            basket_servo_pin(basket_servo_pin),
            electromagnet_pin(electromagnet_pin) {}

    private:
        uint arm_servo_pin;
        uint basket_servo_pin;
        uint electromagnet_pin;

        //Test all of these, they may need to be made pwm
        int arm_forward_position = 0;
        int arm_backward_position = 180;
        int basket_home = 0;
        
        int deposition_arm_position = 90;
        int deposition_basket_position = 90;

    void trail(bool traveling_forward) {
        //basket_servo.setPosition(basket_home);

        switch(traveling_forward) {
            case true:
                //arm_servo.setPosition(arm_backward_position);
                break;
            case false:
                //arm_servo.setPosition(arm_forward_position);
                break;
        }
    }

    void engage_electromagnet() {
        //digitalWrite(electromagnet_pin, HIGH);
    }
    void disengage_electromagnet() {
        //digitalWrite(electromagnet_pin, LOW);
    }

    void internal_deposit() {
        //arm_servo.setPosition(deposition_arm_position);
        //basket_servo.setPosition(basket_home);
    }

    void external_deposit() {
        //arm_servo.setPosition(arm_backwards_poasition);
        //basket_servo.setPosition(deposition_basket_position);
    }

}

int main() {
    //init stdio
    stdio_init_all();

    Motor front_left_motor(0, 1);
    Motor front_right_motor(2, 3);
    Motor back_left_motor(4, 5);
    Motor back_right_motor(6, 7);


    while (true) {
        printf("hello world\n"); //placeholder for motor control code
        sleep_ms(1000); // Delay for 1 second
    }
}
