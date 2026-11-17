#include <stdio.h>
#include <cstdlib>
#include "tape.h"
#include "pico/stdlib.h"
#include <hardware/pwm.h>
#include "overcurrent.h"  
#include <math.h> 

uint fr_a = 8, fr_b = 9, //front right motor pins
     fl_a = 6, fl_b = 7, //front left motor pins
     br_a = 4, br_b = 5, //back right motor pins
     bl_a = 2, bl_b = 3; //back left motor pins

//put other pin definitions here

//MIN Speed 20000

class Motor {
    public:
        Motor(uint pin_positive, uint pin_negative) 
        :   pin_positive(pin_positive), 
            pin_negative(pin_negative)
        {
            gpio_set_function(pin_positive, GPIO_FUNC_PWM);
            gpio_set_function(pin_negative, GPIO_FUNC_PWM);

            //get the pwm slice for each pin, 8 slices in total, each slice has 2 channels, A and B
            pwm_slice = pwm_gpio_to_slice_num(pin_positive);

            //enable the pwm for each slice, this will start the pwm signal on the pins
            pwm_set_enabled(pwm_slice, true);

            //set the pwm frequency to 1kHz, this will set the period of the pwm signal to 1ms
            pwm_set_clkdiv(pwm_slice, 2.0f);
            pwm_set_wrap(pwm_slice, 62499);
        }

        //set the speed of the motor using PWM
        void setSpeed(int speed) {

            /* this will probably be the class and function that handles the endocing, we will need to define how fast we want our
             speed to be (some arbitrary value) figure out how many ticks that is per second and use a basic error 
             correction calculation to accomodate. */

            switch (speed) {
                case 0: //no current or voltage accross the motor
                    pwm_set_chan_level(pwm_slice, 0, 0);
                    pwm_set_chan_level(pwm_slice, 1, 0);
                    break;
                default:
                    switch (speed > 0){
                        case true: //forward
                            pwm_set_chan_level(pwm_slice, 0, std::abs(speed));
                            pwm_set_chan_level(pwm_slice, 1, 0);
                            break;
                    
                        case false: //reverse
                            pwm_set_chan_level(pwm_slice, 0, 0);
                            pwm_set_chan_level(pwm_slice, 1, std::abs(speed));
                            break;
                    }
                    break;
            }
           printf("speed: %d\n", speed);
        }

        //set the motor to brake mode, voltage and no current accross the motor
        void setBrake() {
            pwm_set_chan_level(pwm_slice, 0, 62500);
            pwm_set_chan_level(pwm_slice, 1, 62500);
        }

        //int error_correction(int desired_speed, int actual_speed) {}

    private:
        uint pin_positive;        
        uint pin_negative;

        uint pwm_slice;
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
        constexpr double pi = 3.14159265358979323846;
        front_left_speed = magnitude * cos(direction + pi/4);
        front_right_speed = magnitude * cos(direction - pi/4);
        back_left_speed = magnitude * cos(direction - pi/4);
        back_right_speed = magnitude * cos(direction + pi/4);

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

int main() {
    //init stdio
    stdio_init_all();
    int count = 0;

    Motor front_right_motor(fr_a, fr_b);
    Motor front_left_motor(fl_a, fl_b);
    Motor back_right_motor(br_a, br_b);
    Motor back_left_motor(bl_a, bl_b);

    tape::init(); //initialize tape sensors

    // start oc before motors
    //oc::init();
    //bool oc_was_ok = true;

    while (count < 10) {
        // check oc latch every loop
        // bool oc_ok = oc::motors_allowed();
        // if (!oc_ok && oc_was_ok) {
        //     printf("trip #%lu\n", (unsigned long)oc::trip_count());
        // }
        // if (oc_ok && !oc_was_ok) {
        //     printf("clear\n");
        // }
        // oc_was_ok = oc_ok;

        // if (oc_ok) {
        //     printf("hello world\n"); //placeholder for motor control code
        // } else {
            
        // }

        // front_right_motor.setSpeed(62500); //placeholder for motor control code
        // front_left_motor.setSpeed(60000); //placeholder for motor control code
        // back_right_motor.setSpeed(62500); //placeholder for motor control code
        // back_left_motor.setSpeed(60000); //placeholder for motor control code

        sleep_ms(1000); 

        //  count++;
        //  printf("count: %d\n", count);

        tape::update(); //update tape readings
    
        if (tape::on_tape(tape::FL) || tape::on_tape(tape::FR)) {
        //chassis.stop(); 
        printf(" front edge hit the boundary\n");
        }
        uint32_t t = tape::raw(tape::BL); // raw read
        printf("troubleshooting\n");
        printf("raw reading: %d\n", t);

    }

    // front_right_motor.setSpeed(0); //placeholder for motor control code
    // front_left_motor.setSpeed(0); //placeholder for motor control code
    // back_right_motor.setSpeed(0); //placeholder for motor control code
    // back_left_motor.setSpeed(0); //placeholder for motor control code

    // while (1) {
    //     sleep_ms(1000);
    // }
    
}
