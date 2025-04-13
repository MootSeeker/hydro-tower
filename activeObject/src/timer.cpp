#include "timer.h"
#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

Timer::Timer(std::string Name, bool autoReload, std::function<void(Event*)> Callback, int Identify)
    : _timerName(Name),
      _identify(Identify),
      _timerHandle(nullptr),
      _event(nullptr),
      _callback(Callback)
{
    _timerHandle = xTimerCreate(
        _timerName.c_str(),
        pdMS_TO_TICKS(1000),
        autoReload ? pdTRUE : pdFALSE,
        static_cast<void*>(this),  // Explicitly cast this pointer
        timerCallback
    );

    if (_timerHandle == nullptr) {
        ESP_LOGE("Timer", "Failed to create timer %s", _timerName.c_str());
    }
}

Timer::~Timer() {
    if (_timerHandle) {
        xTimerDelete(_timerHandle, 0);
    }
}

void Timer::Start(TickType_t duration) {
    _event = nullptr;
    if (_timerHandle == nullptr) {
        ESP_LOGE("Timer", "Timer %s not initialized", _timerName.c_str());
        return;
    }

    BaseType_t result = xTimerChangePeriod(_timerHandle, duration, pdMS_TO_TICKS(100));
    if (result != pdPASS) {
        ESP_LOGE("Timer", "Failed to change period for timer %s", _timerName.c_str());
        return;
    }

    result = xTimerStart(_timerHandle, pdMS_TO_TICKS(100));
    if (result != pdPASS) {
        ESP_LOGE("Timer", "Failed to start timer %s", _timerName.c_str());
    }
}

void Timer::Start(TickType_t duration, Event* event) {
    _event = event;
    Start(duration);  // Reuse existing Start method
}

void Timer::Stop() {
    if (_timerHandle) {
        xTimerStop(_timerHandle, 0);
    }
}

void Timer::Reset() {
    if (_timerHandle) {
        xTimerReset(_timerHandle, 0);
    }
}

TimerHandle_t Timer::GetHandle() {
    return _timerHandle;
}


void Timer::timerCallback(TimerHandle_t xTimer) 
{
    Timer* timer = static_cast<Timer*>(pvTimerGetTimerID(xTimer));
    
    if (timer == nullptr) 
    {
        ESP_LOGE("Timer", "Invalid timer pointer in callback");
        return;
    }

    if (timer->_callback == nullptr) 
    {
        ESP_LOGE("Timer", "No callback registered for timer %s", timer->_timerName.c_str());
        return;
    }

    // Execute callback and clear event
    timer->_callback(timer->_event);
    if (timer->_event != nullptr) 
    {
        delete timer->_event;  // Clean up event if it exists
        timer->_event = nullptr;
    }
}