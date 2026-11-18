#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define MAGNET_PIN   21
#define ARM_PIN      19     // servo 1: arm that carries the electromagnet
#define DEPOSIT_PIN  20     // servo 2: deposits at the end

#define ARM_DOWN       10   // electromagnet next to the washer
#define ARM_UP        170   // reversed all the way, over the other container
#define DEPOSIT_HOME   90
#define DEPOSIT_OPEN  160

void servo_init(uint pin)
{
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(pin);
    pwm_set_clkdiv(slice, clock_get_hz(clk_sys) / 1000000.0f);  // 1 tick = 1 us
    pwm_set_wrap(slice, 19999);                                 // 20 ms period (50 Hz)
    pwm_set_gpio_level(pin, 0);                                 // start with no pulses
    pwm_set_enabled(slice, true);
}

void servo_write(uint pin, int angle)                           // angle: 0 to 180
{
    pwm_set_gpio_level(pin, 544 + angle * (2400 - 544) / 180);  // pulse width in us
}

void servo_off(uint pin)                                        // stop pulses
{
    pwm_set_gpio_level(pin, 0);
}

int main(void)
{
    gpio_init(MAGNET_PIN);
    gpio_set_dir(MAGNET_PIN, GPIO_OUT);
    gpio_put(MAGNET_PIN, 0);

    servo_init(ARM_PIN);        // both start with no pulses
    servo_init(DEPOSIT_PIN);

    servo_write(ARM_PIN, ARM_UP);               // arm to its start position
    sleep_ms(1000);
    servo_off(ARM_PIN);

    servo_write(DEPOSIT_PIN, DEPOSIT_HOME);     // deposit servo to its start position
    sleep_ms(1000);
    servo_off(DEPOSIT_PIN);

    while (true)
    {
        gpio_put(MAGNET_PIN, 1);                // magnet on
        servo_write(ARM_PIN, ARM_DOWN);         // lower arm next to the washer
        sleep_ms(3000);                         // washer attaches

        servo_write(ARM_PIN, ARM_UP);           // reverse all the way
        sleep_ms(1500);

        gpio_put(MAGNET_PIN, 0);                // washer drops into the other container
        sleep_ms(300);
        servo_off(ARM_PIN);                     // arm servo off BEFORE servo 2 starts

        servo_write(DEPOSIT_PIN, DEPOSIT_OPEN); // servo 2 deposits
        sleep_ms(1000);
        servo_write(DEPOSIT_PIN, DEPOSIT_HOME);
        sleep_ms(1000);
        servo_off(DEPOSIT_PIN);                 // servo 2 off before the next cycle

        sleep_ms(2000);
    }
}
