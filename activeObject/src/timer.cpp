#include "timer.h"
#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

Timer::Timer(const std::string& Name, bool autoReload, std::function<void(Event*)> Callback, int Identify)
    : _timerName(Name),
      _identify(Identify),
      _timerHandle(nullptr),
      _event(nullptr),
      _callback(Callback),
      _autoReload(autoReload)
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
    // Free any remaining event
    if (_event != nullptr) {
        delete _event;
        _event = nullptr;
    }
}

void Timer::Start(TickType_t duration) {
    _event = nullptr;
    if (_timerHandle == nullptr) {
        ESP_LOGE("Timer", "Timer %s not initialized", _timerName.c_str());
        return;
    }

    BaseType_t result = xTimerChangePeriod(_timerHandle, pdMS_TO_TICKS(duration), pdMS_TO_TICKS(100));
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
    // Delete any existing event before assigning the new one
    if (_event != nullptr) {
        delete _event;
    }
    _event = event;
    Start(duration);  // Reuse existing Start method
}

void Timer::Stop() {
    if (_timerHandle) {
        xTimerStop(_timerHandle, pdMS_TO_TICKS(100));
    }
}

void Timer::Reset() {
    if (_timerHandle) {
        xTimerReset(_timerHandle, pdMS_TO_TICKS(100));
    }
}

TimerHandle_t Timer::GetHandle() const {
    return _timerHandle;
}

void Timer::SetCallback(std::function<void(Event*)> callback) {
    _callback = callback;
}

void Timer::SetIdentify(int identify) {
    _identify = identify;
}

int Timer::GetIdentify() const {
    return _identify;
}

void Timer::SetName(const std::string& name) {
    _timerName = name;
}

const std::string& Timer::GetName() const {
    return _timerName;
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

    // Create a local copy of the event pointer to prevent possible race conditions
    Event* eventToProcess = timer->_event;
    timer->_event = nullptr;  // Clear event pointer first for thread safety
    
    // Execute callback with the stored event
    timer->_callback(eventToProcess);
    
    // Clean up event if it exists and timer is not auto-reload
    // For auto-reload timers, the event is already cleared above
    if (eventToProcess != nullptr) 
    {
        delete eventToProcess;
    }
}