#include "button.h"
#include "events.h"
#include <cstdio>

#define PRESS_THRESHOLD_MS 500

ButtonActor::ButtonActor(gpio_num_t pin)
    : ActiveObject("Button", 2048, 10), _pin(pin), _pressStartTick(0), _waitingRelease(false) {
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << pin);
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    gpio_isr_handler_add(pin, isrHandler, this);
}

void IRAM_ATTR ButtonActor::isrHandler(void* arg) {
    ButtonActor* self = static_cast<ButtonActor*>(arg);
    int level = gpio_get_level(self->_pin);
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    TickType_t now = xTaskGetTickCountFromISR();
    Event* evt = nullptr;

    if (level == 0 && !self->_waitingRelease) {
        self->_pressStartTick = now;
        self->_waitingRelease = true;
    } else if (level == 1 && self->_waitingRelease) {
        TickType_t duration = now - self->_pressStartTick;
        self->_waitingRelease = false;
        if (duration >= pdMS_TO_TICKS(PRESS_THRESHOLD_MS)) {
            evt = new SystemResetEvent("LongPress");
        } else {
            evt = new ButtonClicked(self->_pin, 1, "ShortPress");
        }
    }

    if (evt) {
        self->PostISR(evt);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void ButtonActor::Dispatcher(Event* e) {
    switch (e->getType()) {
        case Event::Type::ButtonClicked: {
            ButtonClicked* evt = static_cast<ButtonClicked*>(e);
            printf("[Button GPIO%d] Short Press\n", evt->getID());
            break;
        }
        case Event::Type::SystemReset: {
            printf("[Button] Long Press → SYSTEM RESET requested\n");
            break;
        }
        default:
            break;
    }
}
