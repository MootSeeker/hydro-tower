#include "led.h"
#include "esp_log.h"

static const char* TAG = "LED";

LedActor::LedActor(gpio_num_t pin)
    : ActiveObject("LED", 2048, 10), _pin(pin), _mode(Mode::OFF), _state(false)
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
    if (e->getType() == Event::Type::LedControl) {
        LedControlEvent* ledEvent = static_cast<LedControlEvent*>(e);
        _mode = static_cast<Mode>(ledEvent->getMode());
        applyMode();
    }
}

void LedActor::applyMode()
{
    switch (_mode) {
        case Mode::OFF:
            gpio_set_level(_pin, 0);
            break;

        case Mode::ON:
            gpio_set_level(_pin, 1);
            break;

        case Mode::TOGGLE:
            _state = !_state;
            gpio_set_level(_pin, _state ? 1 : 0);
            break;

        case Mode::BLINK_SLOW:
            // Implement blinking logic (e.g. start timer or schedule)
            break;

        case Mode::BLINK_FAST:
            // Implement blinking logic (e.g. start timer or schedule)
            break;
    }
}
