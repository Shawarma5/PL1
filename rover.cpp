#include <stdio.h>
#include <cstdlib>
#include "pico/stdlib.h"
#include <hardware/pwm.h>
#include "overcurrent.h"  
#include "tape.h"
#include "encoder.hpp"
#include <math.h> 

uint fr_a = 8, fr_b = 9, //front right motor pins
     fl_a = 6, fl_b = 7, //front left motor pins
     br_a = 4, br_b = 5, //back right motor pins
     bl_a = 2, bl_b = 3; //back left motor pins

//put other pin definitions here
uint fl_enc = 12, fr_enc = 13, //front encoder pins
     bl_enc = 21, br_enc = 22; //back encoder pins

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

    Motor front_right_motor(fr_a, fr_b);
    Motor front_left_motor(fl_a, fl_b);
    Motor back_right_motor(br_a, br_b);
    Motor back_left_motor(bl_a, bl_b);

    // start oc before motors
    oc::init();
    bool oc_was_ok = true;

    tape::init();

    Encoder fl_encoder(fl_enc), fr_encoder(fr_enc), bl_encoder(bl_enc), br_encoder(br_enc);
    Encoder* encoders[4] = {&fl_encoder, &fr_encoder, &bl_encoder, &br_encoder};
    for (Encoder* e : encoders) e->init();

    Motor* motors[4] = {&front_left_motor, &front_right_motor, &back_left_motor, &back_right_motor};
    int test_speed = 40000;

    //stop and pause first so a direction change never switches straight over
    auto drive_all = [&](int speed) {
        for (Motor* m : motors) m->setSpeed(0);
        sleep_ms(20);
        if (speed != 0) {
            for (Motor* m : motors) m->setSpeed(speed);
        }
    };

    bool show_tape = false, show_enc = false;
    uint32_t last_print = 0;
    const char* help = "commands: f forward, b back, s stop, t tape, e encoders, o oc status, h help\n";
    sleep_ms(2000);
    printf("%s", help);

    while (true) {
        // check oc latch every loop
        bool oc_ok = oc::motors_allowed();
        if (!oc_ok && oc_was_ok) {
            printf("trip #%lu, motors stopped\n", (unsigned long)oc::trip_count());
            drive_all(0);
        }
        if (oc_ok && !oc_was_ok) {
            printf("clear\n");
        }
        oc_was_ok = oc_ok;

        tape::update();

        //serial test commands
        switch (getchar_timeout_us(0)) {
            case 'f': drive_all(test_speed); break;
            case 'b': drive_all(-test_speed); break;
            case 's': drive_all(0); break;
            case 't': show_tape = !show_tape; break;
            case 'e': show_enc = !show_enc; break;
            case 'o': printf("oc %s, trips %lu\n", oc_ok ? "ok" : "LATCHED", (unsigned long)oc::trip_count()); break;
            case 'h': printf("%s", help); break;
            default: break;
        }

        uint32_t now = to_ms_since_boot(get_absolute_time());
        if ((show_tape || show_enc) && now - last_print >= 200) {
            last_print = now;
            if (show_tape) {
                printf("tape BL %4lu%s BR %4lu%s FL %4lu%s FR %4lu%s\n",
                       (unsigned long)tape::raw(tape::BL), tape::on_tape(tape::BL) ? "*" : " ",
                       (unsigned long)tape::raw(tape::BR), tape::on_tape(tape::BR) ? "*" : " ",
                       (unsigned long)tape::raw(tape::FL), tape::on_tape(tape::FL) ? "*" : " ",
                       (unsigned long)tape::raw(tape::FR), tape::on_tape(tape::FR) ? "*" : " ");
            }
            if (show_enc) {
                printf("enc FL %lu %.0frpm  FR %lu %.0frpm  BL %lu %.0frpm  BR %lu %.0frpm\n",
                       (unsigned long)fl_encoder.getTicks(), (double)fl_encoder.getRPM(),
                       (unsigned long)fr_encoder.getTicks(), (double)fr_encoder.getRPM(),
                       (unsigned long)bl_encoder.getTicks(), (double)bl_encoder.getRPM(),
                       (unsigned long)br_encoder.getTicks(), (double)br_encoder.getRPM());
            }
        }

        sleep_ms(5);
    }
}
