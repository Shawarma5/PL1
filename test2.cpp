#include <stdio.h>
#include "pico/stdlib.h"
#include <hardware/pwm.h>

class Motor {
    public:
        Motor(uint pin_positive, uint pin_negative, bool direction) 
        :   pin_positive(pin_positive), 
            pin_negative(pin_negative), 
            direction(direction) {
            gpio_set_function(pin_positive, GPIO_FUNC_PWM);
            gpio_set_function(pin_negative, GPIO_FUNC_PWM);

            //get the pwm slice for each pin, 8 slices in total, each slice has 2 channels, A and B
            pwm_slice_positive = pwm_gpio_to_slice(pin_positive);
            pwm_slice_negative = pwm_gpio_to_slice(pin_negative);

            //enable the pwm for each slice, this will start the pwm signal on the pins
            pwm_set_enabled(pwm_slice_positive, true);
            pwm_set_enabled(pwm_slice_negative, true);

            //set the pwm frequency to 1kHz, this will set the period of the pwm signal to 1ms
            pwm_set_clkdiv(pwm_slice_positive, 125); // 125MHz /
            pwm_set_clkdiv(pwm_slice_negative, 125); // 125MHz /

            pwm_set_wrap(pwm_slice_positive, 1000); // 1kHz
            pwm_set_wrap(pwm_slice_negative, 1000); // 1kHz

        }

        //set the speed of the motor using PWM
        void setSpeed(int speed) {
            switch (speed) {
                case 0: //no current or voltage accross the motor
                    gpio_put(pin_positive, 0);
                    gpio_put(pin_negative, 0);
                    break;
                default:
                    //smth smth pwm, all this will need to change when we implement pwm library
                    break;
            }
        }

        //set the motor to brake mode, voltage and no current accross the motor
        void setBrake() {
            gpio_put(pin_positive, 1);
            gpio_put(pin_negative, 1);
        }

    private:
        uint pin_positive;
        uint pin_negative;
        bool direction; //true for forward, false for reverse

        uint pwm_slice_positive;
        uint pwm_slice_negative;
};

class chassis {
    public:
        chassis(Motor front_left_motor, Motor front_right_motor, Motor back_left_motor, Motor back_right_motor)
        :   front_left_motor(front_left_motor),
            front_right_motor(front_right_motor),
            back_left_motor(back_left_motor),
            back_right_motor(back_right_motor) {}



    private:
        Motor front_left_motor;
        Motor front_right_motor;
        Motor back_left_motor;
        Motor back_right_motor;


};

int main() {
    //init stdio
    stdio_init_all();

    while (true) {
        printf("hello world\n"); //placeholder for motor control code
        sleep_ms(1000); // Delay for 1 second
    }
}
