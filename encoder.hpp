#ifndef ENCODER_HPP
#define ENCODER_HPP

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"

class Encoder {
public:
    // ticksPerRev: Number of reflective stripes on your wheel
    // timeoutUs: Microseconds without a pulse before speed reads 0 (default 0.5s)
    Encoder(uint pin, uint32_t ticksPerRev = 8, uint32_t debounceUs = 500, uint64_t timeoutUs = 500000);
    
    void init();
    uint32_t getTicks();
    void resetTicks();
    float getRPM(); // New speed calculation method

private:
    uint _pin;
    uint32_t _ticksPerRev;
    uint32_t _debounceUs;
    uint64_t _timeoutUs;
    
    volatile uint32_t _ticks;
    volatile uint64_t _lastPulseTimeUs;
    volatile uint64_t _pulsePeriodUs; // Time elapsed between the last two valid pulses

    void handlePulse();

    static Encoder* _instances[30];
    static void gpioCallback(uint gpio, uint32_t events);
};

#endif
