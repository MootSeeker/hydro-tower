
#include "app.h"

#include "activeObject.h"
#include "events.h"
#include <stdio.h>

namespace App
{


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
                if (e->getType() == Event::Type::OnStart) {
                    printf("[Sensor] INIT -> IDLE\n");
                    _state = State::IDLE;
                    _timer.Start(pdMS_TO_TICKS(500), new OnStart());
                }
                break;

            case State::IDLE:
                if (e->getType() == Event::Type::OnStart) {
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
    DisplayActor() : ActiveObject("Display", 4096, 10), _state(State::INIT) 
    {
        EventBus::get().subscribe(Event::Type::Measurement, [this](Event* e) 
        {
            this->Post(e);
        });
    }

    void Dispatcher(Event* e) override {
        if (e->getType() == Event::Type::ScreenRefresh) {
            printf("[Display] 🔄 Refresh triggered\n");
            _timer.Start(pdMS_TO_TICKS(750), new ScreenRefreshEvent());
            return;
        }

        switch (_state) {
            case State::INIT:
                if (e->getType() == Event::Type::OnStart) {
                    printf("[Display] INIT -> IDLE\n");
                    _state = State::IDLE;
                    _timer.Start(pdMS_TO_TICKS(750), new ScreenRefreshEvent());
                }
                break;

            case State::IDLE:
                if (e->getType() == Event::Type::Measurement) {
                    auto* m = static_cast<MeasurementEvent*>(e);
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
    LoggerActor() : ActiveObject("Logger", 4096, 10)
    {
        EventBus::get().subscribe(Event::Type::Measurement, [this](Event* e) 
        {
            this->Post(e);
        });
    }

    void Dispatcher(Event* e) override {
        if (e->getType() == Event::Type::Measurement) {
            auto* m = static_cast<MeasurementEvent*>(e);
            printf("[Logger] 📝 Value logged: %.2f\n", m->getValue());
        }
    }
};

void AppStart( void )
{
    static SensorActor sensor;
    static DisplayActor display;
    static LoggerActor logger;

    sensor.Start();
    display.Start();
    logger.Start();

    sensor.Post(new OnStart());
    display.Post(new OnStart());
}

}