#include "app.h"
#include "activeObject.h"
#include "events.h"
#include "eventBus.h"
#include <stdio.h>

#include "esp_log.h"

#include "button.h"
#include "led.h"
#include "wifi.h"



namespace App {

enum class State {
    INIT,
    IDLE,
    ACTIVE
};

void AppStart() 
{
    static WiFiActor wifi;

    static ButtonActor button1(GPIO_NUM_1);
    static ButtonActor button2(GPIO_NUM_2);
    static ButtonActor button3(GPIO_NUM_3);

    static LED::LedActor led1(GPIO_NUM_11);
    static LED::LedActor led2(GPIO_NUM_12);
    static LED::LedActor led3(GPIO_NUM_13);

    wifi.Configure("MySSID", "MyPassword");

    vTaskDelay(pdMS_TO_TICKS(100));

    wifi.Start();     
    vTaskDelay(pdMS_TO_TICKS(5));

    button1.Start();  
    vTaskDelay(pdMS_TO_TICKS(5));
    button2.Start();  
    vTaskDelay(pdMS_TO_TICKS(5));
    button3.Start();  
    vTaskDelay(pdMS_TO_TICKS(5));

    led1.Start();
    led2.Start();
    led3.Start();

    // Demo: LEDs reagieren auf ButtonEvents
    EventBus::get().subscribe(Event::Type::ButtonClicked, [](Event* e) {
        auto* evt = static_cast<ButtonClicked*>(e);
        if (evt->getActionType() == ButtonClicked::ActionType::SINGLE) {
            EventBus::get().publish(new LedControlEvent(LedMode::TOGGLE, "LED-Demo"));
        }
        if (evt->getActionType() == ButtonClicked::ActionType::DOUBLE) {
            EventBus::get().publish(new LedControlEvent(LedMode::BLINK_FAST, "LED-Demo"));
        }
        if (evt->getActionType() == ButtonClicked::ActionType::LONG) {
            EventBus::get().publish(new LedControlEvent(LedMode::OFF, "LED-Demo"));
        }

        printf("[App] ButtonEvent: ID=%d, Type=%d\n", evt->getID(), static_cast<int>(evt->getActionType()));
    });

    EventBus::get().publish(new LedControlEvent(LedMode::TOGGLE, "Test"));

    wifi.Post(new OnStart("App"));
}


} // namespace App