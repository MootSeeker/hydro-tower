#include "app.h"
#include "activeObject.h"
#include "timer.h"
#include "events.h"
#include "eventBus.h"
#include <stdio.h>

#include "esp_log.h"

#include "button.h"
#include "led.h"
#include "wifi.h"

static const char* TAG = "App";

namespace App {

enum class State {
    INIT,
    IDLE,
    ACTIVE
};

// Event-Handler für Button-Events
void handleButtonEvent(ButtonClicked* buttonEvent, LED::LedActor& greenLed, LED::LedActor& blueLed) {
    int buttonId = buttonEvent->getID();
    auto actionType = buttonEvent->getActionType();
    
    // Log Button-Aktion mit mehr Details
    ESP_LOGI(TAG, "Button %d %s pressed (GPIO %d)", buttonId, 
             actionType == ButtonClicked::ActionType::SINGLE ? "SINGLE" : 
             actionType == ButtonClicked::ActionType::DOUBLE ? "DOUBLE" : 
             actionType == ButtonClicked::ActionType::LONG ? "LONG" : "UNKNOWN", 
             buttonId);
    
    // Für einen einfachen Druck (SHORT PRESS) - Grüne LED blinken
    if (actionType == ButtonClicked::ActionType::SINGLE) {
        // Vorherige Blinkmuster stoppen und grün blinken lassen
        blueLed.Post(new LedStopEvent("ButtonHandler"));
        greenLed.Post(new LedControlEvent(LedMode::BLINK_FAST, "ButtonHandler"));
        ESP_LOGI(TAG, "Green LED blinking on button %d SHORT press", buttonId);
    }
    
    // Für einen Doppeldruck (DOUBLE PRESS) - Blaue LED blinken
    else if (actionType == ButtonClicked::ActionType::DOUBLE) {
        // Vorherige Blinkmuster stoppen und blau blinken lassen
        greenLed.Post(new LedStopEvent("ButtonHandler"));
        blueLed.Post(new LedControlEvent(LedMode::BLINK_SLOW, "ButtonHandler"));
        ESP_LOGI(TAG, "Blue LED blinking on button %d DOUBLE press", buttonId);
    }
}

void AppStart() 
{
    static WiFiActor wifi;

    static ButtonActor button1(GPIO_NUM_1);
    static ButtonActor button2(GPIO_NUM_2);
    static ButtonActor button3(GPIO_NUM_3);

    static LED::LedActor led1(GPIO_NUM_11);  // Rot
    static LED::LedActor led2(GPIO_NUM_12);  // Grün 
    static LED::LedActor led3(GPIO_NUM_13);  // Blau

    // Timer für die rote LED (blinkt kontinuierlich)
    static Timer blinkTimer("BlinkTimer", true, [&](Event* e) {
        static bool ledState = false;
        ledState = !ledState;
        if (ledState) {
            led1.Post(new LedControlEvent(LedMode::ON, "BlinkTimer"));
        } else {
            led1.Post(new LedControlEvent(LedMode::OFF, "BlinkTimer"));
        }
    }, 0);

    // Eventbus-Abonnement für Button-Events
    ESP_LOGI(TAG, "Subscribing to ButtonClicked events");
    EventBus::get().subscribe(Event::Type::ButtonClicked, [&](const Event* event) {
        ESP_LOGI(TAG, "Received ButtonClicked event");
        // Button-Event empfangen
        const ButtonClicked* buttonEvent = static_cast<const ButtonClicked*>(event);
        // Event-Handler aufrufen
        handleButtonEvent(const_cast<ButtonClicked*>(buttonEvent), led2, led3);
    });

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

    // Direkt den Wert in Millisekunden übergeben, nicht in Ticks
    blinkTimer.Start(2000);

    wifi.Post(new OnStart("App"));
}

} // namespace App