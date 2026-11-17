
#include "tape.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "pico/stdlib.h"

namespace tape {
namespace {

constexpr unsigned int kPins[kCount] = {17, 18, 19, 20};

//order bl br fl fr
constexpr uint32_t kThresholdUs[kCount] = {1150, 1150, 1150, 1150};

constexpr uint32_t kTimeoutUs = 3000;  //no read
constexpr uint32_t kChargeUs = 10;     // cap charge
constexpr int kConfirm = 2;            // readings in a row before on_tape() changes

uint32_t pin_mask = 0;                 

volatile bool measuring = false;
volatile uint32_t start_us = 0;
volatile uint32_t fall_us[kCount];
volatile uint32_t fallen = 0;          // set bit = fallen


uint32_t readings[kCount];
bool on[kCount];
int streak[kCount];
uint32_t count = 0;

//interrupt
void fall_irq() {
    const uint32_t now = time_us_32();
    for (int i = 0; i < kCount; i++) {
        if (gpio_get_irq_event_mask(kPins[i]) & GPIO_IRQ_EDGE_FALL) {
            gpio_acknowledge_irq(kPins[i], GPIO_IRQ_EDGE_FALL);
            if (measuring && !(fallen & (1u << i))) {
                fall_us[i] = now;
                fallen |= 1u << i;
            }
        }
    }
}

void start_reading() {
    //cap charge
    gpio_put_masked(pin_mask, pin_mask);
    gpio_set_dir_out_masked(pin_mask);
    busy_wait_us(kChargeUs);

    
    for (int i = 0; i < kCount; i++) gpio_acknowledge_irq(kPins[i], GPIO_IRQ_EDGE_FALL);
    fallen = 0;
    measuring = true;
    start_us = time_us_32();

    gpio_set_dir_in_masked(pin_mask);
}

void finish_reading() {
    measuring = false;  

    for (int i = 0; i < kCount; i++) {
        uint32_t t = (fallen & (1u << i)) ? fall_us[i] - start_us : kTimeoutUs;
        if (t > kTimeoutUs) t = kTimeoutUs;
        readings[i] = t;

        const bool looks_like_tape = t < kThresholdUs[i];
        if (looks_like_tape == on[i]) {
            streak[i] = 0;
        } else if (++streak[i] >= kConfirm) {
            on[i] = looks_like_tape;
            streak[i] = 0;
        }
    }
    count++;
}

} 

void init() {
    for (int i = 0; i < kCount; i++) {
        gpio_init(kPins[i]);
        gpio_disable_pulls(kPins[i]); 
        pin_mask |= 1u << kPins[i];
        readings[i] = kTimeoutUs;
        on[i] = false;
        streak[i] = 0;
    }

    gpio_add_raw_irq_handler_masked(pin_mask, fall_irq);
    for (int i = 0; i < kCount; i++) {
        gpio_set_irq_enabled(kPins[i], GPIO_IRQ_EDGE_FALL, true);
    }
    irq_set_enabled(IO_IRQ_BANK0, true);

    start_reading();
}

void update() {
    if (!measuring) {
        start_reading();
        return;
    }
    const bool all_fell = fallen == (1u << kCount) - 1;
    const bool timed_out = time_us_32() - start_us >= kTimeoutUs;
    if (all_fell || timed_out) {
        finish_reading();
        start_reading();
    }
}

uint32_t raw(int sensor) { return readings[sensor]; }
bool on_tape(int sensor) { return on[sensor]; }
bool any_on_tape() { return on[BL] || on[BR] || on[FL] || on[FR]; }
uint32_t reading_count() { return count; }

}  
