
#ifndef LED_H
#define LED_H

#include "activeObject.h"
#include "events.h"
#include "driver/gpio.h"

namespace LED
{

    class LedActor : public ActiveObject {
        public:
            LedActor(gpio_num_t pin);
            void Dispatcher(Event* e) override;
        
        private:
        
            gpio_num_t _pin;
            LedMode _mode;
            LedMode _blinkMode = LedMode::OFF;
            bool _state;
            Timer _timer;
        };
}

#endif // End: LED_H