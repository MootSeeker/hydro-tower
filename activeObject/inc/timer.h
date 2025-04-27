#ifndef TIMER_H
#define TIMER_H

#include <string>
#include <functional>
#include <utility>

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "events.h"

class Timer 
{
public:
    Timer(const std::string& name, bool autoReload, std::function<void(Event*)> callback, int identify = 0);
    virtual ~Timer();

    // Start the timer with a duration (ms), optionally with an event
    void Start(TickType_t duration);
    void Start(TickType_t duration, Event* event);

    // Stop or reset the timer
    void Stop();
    void Reset();

    // Get the underlying FreeRTOS timer handle
    TimerHandle_t GetHandle() const;

    // Set a new callback
    void SetCallback(std::function<void(Event*)> callback);

    // Set or get the identify value
    void SetIdentify(int identify);
    int GetIdentify() const;

    // Set or get the timer name
    void SetName(const std::string& name);
    const std::string& GetName() const;

private:
    std::string _timerName;
    int _identify;
    TimerHandle_t _timerHandle;
    Event* _event;
    std::function<void(Event*)> _callback;
    bool _autoReload;

    static void timerCallback(TimerHandle_t xTimer);

    // Disallow copy and assignment
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;
};

#endif // TIMER_H