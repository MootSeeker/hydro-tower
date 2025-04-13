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
    void processButtonState();

    gpio_num_t _pin;
    volatile TickType_t _pressTick;
    volatile bool _waitingRelease;
    volatile int _clickCount;
    volatile TickType_t _lastClickTick;
    volatile bool _eventPending = false;
    volatile bool _buttonPressed = false;

    static constexpr int DOUBLE_CLICK_GAP_MS = 300;
    static constexpr int LONG_PRESS_MS = 1000;
    static constexpr int DEBOUNCE_MS = 50;

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

class ButtonTimerEvent : public Event {
public:
    ButtonTimerEvent() : Event("ButtonPoll") {}
    Type getType() const override { return Type::TimerTick; }
    Event* Clone() const override { return new ButtonTimerEvent(); }
};

#endif // BUTTON_H