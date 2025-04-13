// timer.cpp
#include "timer.h"
#include <stdio.h>

Timer::Timer(std::string Name, bool autoReload, std::function<void(Event*)> Callback, int Identify)
    : _timerName(Name), _identify(Identify), _callback(Callback) {
    _timerHandle = xTimerCreate(_timerName.c_str(), pdMS_TO_TICKS(1000), autoReload ? pdTRUE : pdFALSE, this, timerCallback);
}

Timer::~Timer() {
    if (_timerHandle) {
        xTimerDelete(_timerHandle, 0);
    }
}

void Timer::Start(TickType_t duration) {
    _event = nullptr;
    if (_timerHandle) {
        xTimerChangePeriod(_timerHandle, duration, 0);
        xTimerStart(_timerHandle, 0);
    }
}

void Timer::Start(TickType_t duration, Event* event) {
    _event = event;
    if (_timerHandle) {
        xTimerChangePeriod(_timerHandle, duration, 0);
        xTimerStart(_timerHandle, 0);
    }
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

void Timer::timerCallback(TimerHandle_t xTimer) {
    Timer* timer = static_cast<Timer*>(pvTimerGetTimerID(xTimer));
    if (timer != nullptr && timer->_callback != nullptr) {
        timer->_callback(timer->_event);  // kann auch nullptr sein
        timer->_event = nullptr;
    }
}