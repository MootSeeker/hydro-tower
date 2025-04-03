#include "button.h"
#include "events.h"
#include "esp_log.h"

#define TAG "Button"

#define DOUBLE_CLICK_GAP_MS 300
#define LONG_PRESS_MS 1000

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

    gpio_isr_handler_add(pin, isrHandler, this);
}

void IRAM_ATTR ButtonActor::isrHandler(void* arg)
{
    ButtonActor* self = static_cast<ButtonActor*>(arg);
    int level = gpio_get_level(self->_pin);
    BaseType_t hpTaskWoken = pdFALSE;
    TickType_t now = xTaskGetTickCountFromISR();

    if (level == 0 && !self->_waitingRelease) {
        self->_pressTick = now;
        self->_waitingRelease = true;
    }
    else if (level == 1 && self->_waitingRelease) {
        TickType_t duration = now - self->_pressTick;
        self->_waitingRelease = false;

        if (duration >= pdMS_TO_TICKS(LONG_PRESS_MS)) {
            self->_deferredAction = ActionType::LONG;
        } else {
            TickType_t gap = now - self->_lastClickTick;
            self->_lastClickTick = now;

            if (gap < pdMS_TO_TICKS(DOUBLE_CLICK_GAP_MS)) {
                self->_clickCount++;
                if (self->_clickCount == 2) {
                    self->_deferredAction = ActionType::DOUBLE;
                    self->_clickCount = 0;
                }
            } else {
                self->_clickCount = 1;
                self->_deferredAction = ActionType::SINGLE;
            }
        }

        // Post Signal Event (nur ein Marker)
        self->PostISR(new ButtonClicked(self->_pin, 99));  // Typisches Dummy-Event
        portYIELD_FROM_ISR(hpTaskWoken);
    }
}


void ButtonActor::Dispatcher(Event* e)
{
    switch (e->getType())
    {
        case Event::Type::ButtonClicked: {
            ButtonClicked* evt = static_cast<ButtonClicked*>(e);
            int id = evt->getID();
            int state = evt->getState();

            if (state == 1) {
                ESP_LOGI("Button", "GPIO%d Clicked (Single)", id);
            } else if (state == 2) {
                ESP_LOGI("Button", "GPIO%d Clicked (Double)", id);
            } else {
                ESP_LOGI("Button", "GPIO%d Clicked (Unknown)", id);
            }

            break;
        }

        case Event::Type::SystemReset: {
            ESP_LOGI("Button", "🛑 Long Press → SYSTEM RESET requested");
            break;
        }

        default:
            ESP_LOGW("Button", "Unhandled event in dispatcher");
            break;
    }
}

void ButtonActor::SetActionEvent(ActionType type, Event* event) {
    _actionMap[static_cast<size_t>(type)].reset(event);
}
