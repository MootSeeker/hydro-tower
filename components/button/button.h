#ifndef BUTTON_H
#define BUTTON_H

#include "activeObject.h"
#include "events.h"
#include "driver/gpio.h"

class ButtonActor : public ActiveObject {
public:
    ButtonActor(gpio_num_t pin);
    void Dispatcher(Event* e) override;

private:
    static void IRAM_ATTR isrHandler(void* arg);
    gpio_num_t _pin;
    volatile TickType_t _pressStartTick;
    volatile bool _waitingRelease;
};

#endif // BUTTON_H