#include "led.h"
#include "esp_log.h"

namespace LED
{
static const char* TAG = "LED";

LedActor::LedActor(gpio_num_t pin)
    : ActiveObject("LED", 4096, 10),
      _pin(pin),
      _mode(LedMode::OFF),
      _state(false),
      _timer("led.timer", false, [this](Event* e) { Post(e); })
{
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << pin,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    gpio_set_level(_pin, 0);
}

void LedActor::Dispatcher(Event* e)
{
    if (e->getType() == Event::Type::LedStop) {
        _mode = LedMode::OFF;
        _blinkMode = LedMode::OFF;
        _timer.Stop();
        gpio_set_level(_pin, 0);
        ESP_LOGI("LED", "🛑 Stopped blinking on GPIO %d", _pin);
        return;
    }
    
    if (e->getType() != Event::Type::LedControl)
        return;

    LedControlEvent* ledEvent = static_cast<LedControlEvent*>(e);
    LedMode mode = ledEvent->getMode();

    switch (mode)
    {
        case LedMode::ON:
            _mode = LedMode::ON;
            _blinkMode = LedMode::OFF;
            _timer.Stop();
            gpio_set_level(_pin, 1);
            break;

        case LedMode::OFF:
            _mode = LedMode::OFF;
            _blinkMode = LedMode::OFF;
            _timer.Stop();
            gpio_set_level(_pin, 0);
            break;

        case LedMode::TOGGLE:
            _state = !_state;
            gpio_set_level(_pin, _state ? 1 : 0);
            ESP_LOGI("LED", "Toggled GPIO %d → %d", _pin, _state);

            if (_blinkMode == LedMode::BLINK_FAST)
                _timer.Start(pdMS_TO_TICKS(250), new LedControlEvent(LedMode::TOGGLE, "blink_fast"));
            else if (_blinkMode == LedMode::BLINK_SLOW)
                _timer.Start(pdMS_TO_TICKS(1000), new LedControlEvent(LedMode::TOGGLE, "blink_slow"));
            break;

        case LedMode::BLINK_FAST:
            _mode = LedMode::TOGGLE;
            _blinkMode = LedMode::BLINK_FAST;
            _timer.Stop();
            _timer.Start(pdMS_TO_TICKS(250), new LedControlEvent(LedMode::TOGGLE, "blink_fast"));
            ESP_LOGI("LED", "Starting BLINK_FAST on GPIO %d", _pin);
            break;

        case LedMode::BLINK_SLOW:
            _mode = LedMode::TOGGLE;
            _blinkMode = LedMode::BLINK_SLOW;
            _timer.Stop();
            _timer.Start(pdMS_TO_TICKS(1000), new LedControlEvent(LedMode::TOGGLE, "blink_slow"));
            ESP_LOGI("LED", "Starting BLINK_SLOW on GPIO %d", _pin);
            break;
    }
}



}