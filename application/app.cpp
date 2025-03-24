#include "app.h"
#include "activeObject.h"
#include "events.h"
#include <stdio.h>

#include "button.h"
#include "wifi.h"


namespace App {

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
                    _timer.Start(pdMS_TO_TICKS(1500), new OnStart());
                }
                break;

            case State::IDLE:
                if (e->getType() == Event::Type::OnStart) {
                    printf("[Sensor] IDLE -> ACTIVE\n");
                    _state = State::ACTIVE;
                    EventBus::get().publish(new MeasurementEvent(42.0f));
                    _state = State::IDLE;
                    _timer.Start(pdMS_TO_TICKS(1500), new OnStart());
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
    DisplayActor() : ActiveObject("Display", 4096, 10), _state(State::INIT) {}

    void Dispatcher(Event* e) override {
        if (e->getType() == Event::Type::ScreenRefresh) {
            printf("[Display] 🔄 Refresh triggered\n");
            _timer.Start(pdMS_TO_TICKS(2500), new ScreenRefreshEvent());
            return;
        }

        switch (_state) {
            case State::INIT:
                if (e->getType() == Event::Type::OnStart) {
                    printf("[Display] INIT -> IDLE\n");
                    _state = State::IDLE;
                    _timer.Start(pdMS_TO_TICKS(2500), new ScreenRefreshEvent());
                }
                break;

            case State::IDLE:
                if (e->getType() == Event::Type::Measurement) {
                    MeasurementEvent* measurement = static_cast<MeasurementEvent*>(e);
                    printf("[Display] IDLE -> ACTIVE\n");
                    _state = State::ACTIVE;
                    printf("📟 Display: %.2f\n", measurement->getValue());
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
    LoggerActor() : ActiveObject("Logger", 4096, 10) {}

    void Dispatcher(Event* e) override {
        if (e->getType() == Event::Type::Measurement) {
            MeasurementEvent* measurement = static_cast<MeasurementEvent*>(e);
            printf("[Logger] 📝 Value logged: %.2f\n", measurement->getValue());
        }
    }
};



void AppStart() {
    static WiFiActor wifi;
    static SensorActor sensor;
    static DisplayActor display;
    static LoggerActor logger;

    static ButtonActor button1(GPIO_NUM_1);
    static ButtonActor button2(GPIO_NUM_2);
    static ButtonActor button3(GPIO_NUM_3);
    static ButtonActor button4(GPIO_NUM_10);

    wifi.Configure("MySSID", "MyPassword");

    // Subscriptions: Jeder Actor erhält eine geklonte Kopie
    EventBus::get().subscribe(Event::Type::Measurement, [](Event* e) {
        display.Post(e);
    });

    EventBus::get().subscribe(Event::Type::Measurement, [](Event* e) {
        logger.Post(e);
    });

    // Start der Active Objects
    wifi.Start();
    sensor.Start();
    display.Start();
    logger.Start();

    button1.Start();
    button2.Start();
    button3.Start();
    button4.Start();

    // OnStart initialisieren mit Quell-Angabe
    wifi.Post(new OnStart("App"));
    sensor.Post(new OnStart("Sensor"));
    display.Post(new OnStart("Display"));
}

} // namespace App