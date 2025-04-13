#include "button.h"
#include "events.h"
#include "eventBus.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define TAG "Button"

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

    _timer.Start(pdMS_TO_TICKS(100), new ButtonTimerEvent());
}

void IRAM_ATTR ButtonActor::isrHandler(void* arg)
{
    ButtonActor* self = static_cast<ButtonActor*>(arg);
    self->_eventPending = true;
}

void ButtonActor::Dispatcher(Event* e)
{
    ESP_LOGI(TAG, "Dispatcher got event from: %s", e->getSource());

    if (std::string(e->getSource()) != "ButtonPoll") {
        ESP_LOGI(TAG, "Skipped: not ButtonPoll");
        return;
    }

    if (!_eventPending) {
        ESP_LOGI(TAG, "Skipped: no eventPending");
        return;
    }

    ESP_LOGI(TAG, "Processing button state...");
    processButtonState();
}

void ButtonActor::processButtonState()
{
    _eventPending = false;

    TickType_t now = xTaskGetTickCount();
    int level = gpio_get_level(_pin);

    ESP_LOGI(TAG, "Button level = %d", gpio_get_level(_pin));

    if (level == 0 && !_buttonPressed) {
        _buttonPressed = true;
        _pressTick = now;
        _waitingRelease = true;
    }
    else if (level == 1 && _buttonPressed) {
        _buttonPressed = false;
        _waitingRelease = false;
        TickType_t pressDuration = now - _pressTick;

        if (pressDuration >= pdMS_TO_TICKS(LONG_PRESS_MS)) {
            EventBus::get().publish(new ButtonClicked(static_cast<int>(_pin), ButtonClicked::ActionType::LONG, "ButtonActor"));
            _clickCount = 0;
        } else {
            if (_clickCount == 0) {
                _lastClickTick = now;
            }
            int tmpClick = _clickCount;
            tmpClick++;
            _clickCount = tmpClick;
        }
    }

    if (_clickCount > 0 && (now - _lastClickTick > pdMS_TO_TICKS(DOUBLE_CLICK_GAP_MS))) {
        if (_clickCount == 1) {
            EventBus::get().publish(new ButtonClicked(static_cast<int>(_pin), ButtonClicked::ActionType::SINGLE, "ButtonActor"));
        } else if (_clickCount == 2) {
            EventBus::get().publish(new ButtonClicked(static_cast<int>(_pin), ButtonClicked::ActionType::DOUBLE, "ButtonActor"));
        }
        _clickCount = 0;
    }
}
