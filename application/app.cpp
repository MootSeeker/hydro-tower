
#include "app.h"

#include "activeObject.h"
#include "events.h"
#include <stdio.h>

enum class State {
    INIT,
    IDLE,
    ACTIVE
};

class SensorActor : public ActiveObject {
public:
    SensorActor() : ActiveObject("Sensor", 4096, 10), _state(State::INIT) {}

    void Dispatcher(Event* e) override {
        switch (_state) {
            case State::INIT:
                if (dynamic_cast<OnStart*>(e)) {
                    printf("[Sensor] INIT -> IDLE\n");
                    _state = State::IDLE;
                    _timer.Start(pdMS_TO_TICKS(500), new OnStart());
                }
                break;

            case State::IDLE:
                if (dynamic_cast<OnStart*>(e)) {
                    printf("[Sensor] IDLE -> ACTIVE\n");
                    _state = State::ACTIVE;
                    EventBus::get().publish(new MeasurementEvent(42.0f));
                    _state = State::IDLE;
                    _timer.Start(pdMS_TO_TICKS(500), new OnStart());
                }
                break;

            default:
                break;
        }
    }

private:
    State _state;
};

class DisplayActor : public ActiveObject {
public:
    DisplayActor() : ActiveObject("Display", 4096, 10), _state(State::INIT) {
        EventBus::get().subscribe<MeasurementEvent>([this](MeasurementEvent* e) {
            this->Post(new MeasurementEvent(*e));
        });
    }

    void Dispatcher(Event* e) override {
        if (dynamic_cast<ScreenRefreshEvent*>(e)) {
            printf("[Display] 🔄 Refresh triggered\n");
            _timer.Start(pdMS_TO_TICKS(750), new ScreenRefreshEvent());
            return;
        }

        switch (_state) {
            case State::INIT:
                if (dynamic_cast<OnStart*>(e)) {
                    printf("[Display] INIT -> IDLE\n");
                    _state = State::IDLE;
                    _timer.Start(pdMS_TO_TICKS(750), new ScreenRefreshEvent());
                }
                break;

            case State::IDLE:
                if (auto* m = dynamic_cast<MeasurementEvent*>(e)) {
                    printf("[Display] IDLE -> ACTIVE\n");
                    _state = State::ACTIVE;
                    printf("📟 Display: %.2f\n", m->getValue());
                    _state = State::IDLE;
                }
                break;

            default:
                break;
        }
    }

private:
    State _state;
};

class LoggerActor : public ActiveObject {
public:
    LoggerActor() : ActiveObject("Logger", 4096, 10) {
        EventBus::get().subscribe<MeasurementEvent>([this](MeasurementEvent* e) {
            this->Post(new MeasurementEvent(*e));
        });
    }

    void Dispatcher(Event* e) override {
        if (auto* m = dynamic_cast<MeasurementEvent*>(e)) {
            printf("[Logger] 📝 Value logged: %.2f\n", m->getValue());
        }
    }
};

