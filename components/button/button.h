#ifndef BUTTON_H
#define BUTTON_H

#include "activeObject.h"
#include "events.h"
#include "driver/gpio.h"
#include <array>
#include <memory>

class ButtonActor : public ActiveObject {
public:
    enum class ActionType : int { NONE, SINGLE, DOUBLE, LONG, MAX };

    ButtonActor(gpio_num_t pin);

    void Dispatcher(Event* e) override;

private:
    static void IRAM_ATTR isrHandler(void* arg);

    gpio_num_t _pin;
    volatile TickType_t _pressTick;
    volatile bool _waitingRelease;
    volatile int _clickCount;
    volatile TickType_t _lastClickTick;
    volatile bool _eventPending = false;
    
    volatile ActionType _deferredAction = ActionType::NONE;
};

class ButtonClicked : public Event {
    public:
        enum class ActionType {
            NONE,
            SINGLE,
            DOUBLE,
            LONG,
            MAX
        };

    
        ButtonClicked(int id, ActionType action, const char* source = "Unknown")
            : Event(source), _buttonID(id), _action(action) {}
    
        Type getType() const override { return Type::ButtonClicked; }
        int getID() const { return _buttonID; }
        ActionType getActionType() const { return _action; }
    
        Event* Clone() const override {
            return new ButtonClicked(_buttonID, _action, _source);
        }
    
    private:
        int _buttonID;
        ActionType _action;
    };

#endif // BUTTON_H