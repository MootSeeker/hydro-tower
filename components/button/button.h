#ifndef BUTTON_H
#define BUTTON_H

#include "activeObject.h"
#include "events.h"
#include "driver/gpio.h"
#include <array>
#include <memory>

class ButtonActor : public ActiveObject {
public:
    enum class ActionType {
        NONE,
        SINGLE,
        DOUBLE,
        LONG,
        MAX
    };

    ButtonActor(gpio_num_t pin);

    void Dispatcher(Event* e) override;

    // Setze das Event für eine bestimmte Aktion
    void SetActionEvent(ActionType action, Event* evt);

private:
    static void IRAM_ATTR isrHandler(void* arg);

    gpio_num_t _pin;
    volatile TickType_t _pressTick;
    volatile bool _waitingRelease;
    volatile int _clickCount;
    volatile TickType_t _lastClickTick;

    volatile ActionType _deferredAction = ActionType::NONE;

    std::array<std::unique_ptr<Event>, static_cast<size_t>(ActionType::MAX)> _actionMap;        
};

#endif // BUTTON_H