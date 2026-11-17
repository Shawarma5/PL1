#include "encoder.hpp"

Encoder* Encoder::_instances[30] = { nullptr };
Encoder::Encoder(uint pin, uint32_t ticksPerRev, uint32_t debounceUs, uint64_t timeoutUs)
    : _pin(pin), _ticksPerRev(ticksPerRev), _debounceUs(debounceUs), 
      _timeoutUs(timeoutUs), _ticks(0), _lastPulseTimeUs(0), _pulsePeriodUs(0) {}

void Encoder::init() {
    if (_pin >= 30) return;

    _instances[_pin] = this;

    gpio_init(_pin);
    gpio_set_dir(_pin, GPIO_IN);
    gpio_pull_up(_pin);

    gpio_set_irq_enabled_with_callback(_pin, GPIO_IRQ_EDGE_RISE, true, &gpioCallback);
}

uint32_t Encoder::getTicks() {
    uint32_t status = save_and_disable_interrupts();
    uint32_t currentTicks = _ticks;
    restore_interrupts(status);
    
    return currentTicks;
}

void Encoder::resetTicks() {
    uint32_t status = save_and_disable_interrupts();
    _ticks = 0;
    _pulsePeriodUs = 0;
    restore_interrupts(status);
}

float Encoder::getRPM() {
    uint32_t status = save_and_disable_interrupts();
    uint64_t period = _pulsePeriodUs;
    uint64_t lastTime = _lastPulseTimeUs;
    restore_interrupts(status);

    uint64_t now = time_us_64();

    // 1. If no pulses have occurred yet, or the wheel stopped spinning, return 0
    if (period == 0 || (now - lastTime) > _timeoutUs) {
        return 0.0f;
    }

    // 2. Calculate RPM: (60,000,000 us per minute) / (period_us * ticks_per_rev)
    return 60000000.0f / (static_cast<float>(period) * static_cast<float>(_ticksPerRev));
}

void Encoder::handlePulse() {
    uint64_t now = time_us_64();
    uint64_t dt = now - _lastPulseTimeUs;
    
    if (dt >= _debounceUs) {
        // Ignore the very first pulse after boot/reset since we don't have a valid delta yet
        if (_lastPulseTimeUs != 0) {
            _pulsePeriodUs = dt;
        }
        _ticks++;
        _lastPulseTimeUs = now;
    }
}

void Encoder::gpioCallback(uint gpio, uint32_t events) {
    if (gpio < 30 && _instances[gpio] != nullptr) {
        _instances[gpio]->handlePulse();
    }
}
