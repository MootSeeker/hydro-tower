#include "TimerManager.h"
#include "events.h"
#include <chrono>

TimerManager::TimerManager() {}

TimerManager& TimerManager::get() {
    static TimerManager instance;
    return instance;
}

void TimerManager::addRepeatingTimer(std::chrono::seconds interval, std::chrono::seconds duration) {
    Timer timer;
    timer.interval = interval;
    timer.duration = duration;
    timer.lastStart = std::chrono::steady_clock::now() - interval; // Start sofort gültig
    _timers.push_back(timer);
}

void TimerManager::update() {
    auto now = std::chrono::steady_clock::now();
    for (auto& timer : _timers) {
        if (!timer.active && now - timer.lastStart >= timer.interval) {
            timer.active = true;
            timer.lastStart = now;
            EventBus::get().publish(new LedControlEvent(LedMode::ON, "TimerManager"));
        }
        if (timer.active && now - timer.lastStart >= timer.duration) {
            timer.active = false;
            EventBus::get().publish(new LedControlEvent(LedMode::OFF, "TimerManager"));
        }
    }
}
