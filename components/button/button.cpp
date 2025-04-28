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
      _eventPending(false),
      _buttonPressed(false),
      _deferredAction(ActionType::NONE)
{
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << pin);
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    esp_log_level_set(TAG, ESP_LOG_INFO); // Erhöhe Log-Level auf INFO

    // Log Button-Initialisierung
    ESP_LOGI(TAG, "Button initialized on GPIO %d", pin);

    // ISR einrichten
    esp_err_t err = gpio_install_isr_service(0);
    if (err == ESP_OK || err == ESP_ERR_INVALID_STATE) { // INVALID_STATE bedeutet bereits installiert
        ESP_LOGI(TAG, "ISR service setup: %s", (err == ESP_OK) ? "OK" : "Already installed");
        err = gpio_isr_handler_add(pin, isrHandler, this);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to add ISR handler for GPIO %d: %s", pin, esp_err_to_name(err));
        }
    } else {
        ESP_LOGE(TAG, "Failed to install ISR service: %s", esp_err_to_name(err));
    }

    // Konfiguriere den geerbten Timer für Button-Polling
    _timer.SetCallback([this](Event* e) {
        this->Post(e);
    });

    // Timer starten für regelmäßige Button-Abfrage
    ESP_LOGI(TAG, "Starting button polling timer");
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

    // Füge eine periodische Statusprüfung alle 500ms hinzu
    static TickType_t lastStatusCheck = 0;
    TickType_t now = xTaskGetTickCount();
    
    if (now - lastStatusCheck > pdMS_TO_TICKS(500)) {
        lastStatusCheck = now;
        // Direkte Statusprüfung der Buttons
        int level = gpio_get_level(_pin);
        ESP_LOGI(TAG, "Button GPIO %d status: %s", (int)_pin, level ? "HIGH" : "LOW");
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
        ESP_LOGI(TAG, "Button GPIO %d level changed to %d", (int)_pin, level);
        _eventPending = false; // Clear the flag
    }

    // Handle button press
    if (level == 0 && !_buttonPressed) {
        _buttonPressed = true;
        _pressTick = now;
        _waitingRelease = true;
        ESP_LOGI(TAG, "Button GPIO %d pressed", (int)_pin);
    }
    // Handle button release
    else if (level == 1 && _buttonPressed) {
        _buttonPressed = false;
        _waitingRelease = false;
        TickType_t pressDuration = now - _pressTick;

        ESP_LOGI(TAG, "Button GPIO %d released after %d ms", (int)_pin, 
                 (int)(pressDuration * portTICK_PERIOD_MS));

        if (pressDuration >= pdMS_TO_TICKS(LONG_PRESS_MS)) {
            // Long press detected
            ESP_LOGI(TAG, "Long press detected on GPIO %d", (int)_pin);
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
            ESP_LOGI(TAG, "Click count for GPIO %d: %d", (int)_pin, _clickCount);
        }
    }

    // Check for click completion after a timeout
    if (_clickCount > 0 && (now - _lastClickTick > pdMS_TO_TICKS(DOUBLE_CLICK_GAP_MS))) {
        int currentClickCount = _clickCount;
        
        if (currentClickCount == 1) {
            ESP_LOGI(TAG, "Single click detected on GPIO %d, publishing event", (int)_pin);
            ButtonClicked::ActionType action = ButtonClicked::ActionType::SINGLE;
            EventBus::get().publish(new ButtonClicked(static_cast<int>(_pin), action, "ButtonActor"));
        } else if (currentClickCount == 2) {
            ESP_LOGI(TAG, "Double click detected on GPIO %d, publishing event", (int)_pin);
            ButtonClicked::ActionType action = ButtonClicked::ActionType::DOUBLE;
            EventBus::get().publish(new ButtonClicked(static_cast<int>(_pin), action, "ButtonActor"));
        } else {
            ESP_LOGI(TAG, "Multiple clicks (%d) detected on GPIO %d", currentClickCount, (int)_pin);
        }
        _clickCount = 0; // Reset click count after event
    }

    // Check for long press while button is still held down
    if (_buttonPressed && _waitingRelease && (now - _pressTick >= pdMS_TO_TICKS(LONG_PRESS_MS))) {
        // Button has been held long enough for a long press and is still down
        _waitingRelease = false; // Prevent repeated long-press events
        ESP_LOGI(TAG, "Long press while held detected on GPIO %d, publishing event", (int)_pin);
        ButtonClicked::ActionType action = ButtonClicked::ActionType::LONG;
        EventBus::get().publish(new ButtonClicked(static_cast<int>(_pin), action, "ButtonActor"));
        _clickCount = 0;
    }
}
