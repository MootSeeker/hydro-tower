
#ifndef LED_H
#define LED_H

#include "activeObject.h"
#include "driver/gpio.h"
#include "events.h"

class LedActor : public ActiveObject {
public:
    enum class Mode {
        OFF,
        ON,
        TOGGLE,
        BLINK_SLOW,
        BLINK_FAST
    };

    explicit LedActor(gpio_num_t pin);
    void Dispatcher(Event* e) override;

private:
    void applyMode();
    gpio_num_t _pin;
    Mode _mode;
    bool _state;
};

#endif // End: LED_H