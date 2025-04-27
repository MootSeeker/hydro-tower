#include "button.h"
#include "events.h"
#include "eventBus.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <string.h> // Using C-style header instead of cstring

static const char* TAG = "Button";

ButtonActor::ButtonActor(gpio_num_t pin)
    : ActiveObject("Button", 4096, 10),
      _pin(pin),
      _pressTick(0),
      _waitingRelease(false),
      _clickCount(0),
      _lastClickTick(0),
      _deferredAction(ActionType::NONE)
{
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << pin);
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(pin, isrHandler, this);

    // Start the timer to check button state every 10ms
    _timer.Start(10, new ButtonTimerEvent());
}

void IRAM_ATTR ButtonActor::isrHandler(void* arg)
{
    ButtonActor* self = static_cast<ButtonActor*>(arg);
    // In ISR we only set a flag to indicate state change
    // The actual level reading is done in the timer handler
    self->_eventPending = true;
}

void ButtonActor::Dispatcher(Event* e)
{
    if (e == nullptr) {
        ESP_LOGW(TAG, "Received null event");
        return;
    }

    // Only process timer events for button polling
    if (e->getType() == Event::Type::TimerTick && 
        strcmp(e->getSource(), "ButtonPoll") == 0) {
        
        // Process button state at regular intervals
        processButtonState();
        
        // Restart the timer for next polling interval
        _timer.Start(10, new ButtonTimerEvent());
    }
}

void ButtonActor::processButtonState()
{
    TickType_t now = xTaskGetTickCount();
    int level = gpio_get_level(_pin);

    // Debug if state change was flagged by ISR
    if (_eventPending) {
        ESP_LOGD(TAG, "Button level = %d", level);
        _eventPending = false; // Clear the flag
    }

    // Handle button press
    if (level == 0 && !_buttonPressed) {
        _buttonPressed = true;
        _pressTick = now;
        _waitingRelease = true;
        ESP_LOGD(TAG, "Button pressed");
    }
    // Handle button release
    else if (level == 1 && _buttonPressed) {
        _buttonPressed = false;
        _waitingRelease = false;
        TickType_t pressDuration = now - _pressTick;

        if (pressDuration >= pdMS_TO_TICKS(LONG_PRESS_MS)) {
            // Long press detected
            ESP_LOGI(TAG, "Long press detected");
            // Convert enum to equivalent ButtonClicked::ActionType
            ButtonClicked::ActionType action = ButtonClicked::ActionType::LONG;
            EventBus::get().publish(new ButtonClicked(static_cast<int>(_pin), action, "ButtonActor"));
            _clickCount = 0; // Reset click count after long press
        } else {
            // Short press - could be part of a single or double click
            if (_clickCount == 0) {
                _lastClickTick = now;
            }
            // Handle volatile variable increment safely
            int currentCount = _clickCount;
            _clickCount = currentCount + 1;
            ESP_LOGD(TAG, "Click count: %d", _clickCount);
        }
    }

    // Check for click completion after a timeout
    if (_clickCount > 0 && (now - _lastClickTick > pdMS_TO_TICKS(DOUBLE_CLICK_GAP_MS))) {
        int currentClickCount = _clickCount;
        
        if (currentClickCount == 1) {
            ESP_LOGI(TAG, "Single click detected");
            ButtonClicked::ActionType action = ButtonClicked::ActionType::SINGLE;
            EventBus::get().publish(new ButtonClicked(static_cast<int>(_pin), action, "ButtonActor"));
        } else if (currentClickCount == 2) {
            ESP_LOGI(TAG, "Double click detected");
            ButtonClicked::ActionType action = ButtonClicked::ActionType::DOUBLE;
            EventBus::get().publish(new ButtonClicked(static_cast<int>(_pin), action, "ButtonActor"));
        } else {
            ESP_LOGI(TAG, "Multiple clicks (%d) detected", currentClickCount);
        }
        _clickCount = 0; // Reset click count after event
    }

    // Check for long press while button is still held down
    if (_buttonPressed && _waitingRelease && (now - _pressTick >= pdMS_TO_TICKS(LONG_PRESS_MS))) {
        // Button has been held long enough for a long press and is still down
        _waitingRelease = false; // Prevent repeated long-press events
        ESP_LOGI(TAG, "Long press while held detected");
        ButtonClicked::ActionType action = ButtonClicked::ActionType::LONG;
        EventBus::get().publish(new ButtonClicked(static_cast<int>(_pin), action, "ButtonActor"));
        _clickCount = 0;
    }
}
