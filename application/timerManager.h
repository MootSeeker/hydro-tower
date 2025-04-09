#ifndef TIMERMANAGER_H
#define TIMERMANAGER_H

#include "EventBus.h"
#include <vector>
#include <chrono>

class TimerManager {
public:
    struct Timer {
        std::chrono::seconds interval;
        std::chrono::seconds duration;
        std::chrono::steady_clock::time_point lastStart;
        bool active = false;
    };

    static TimerManager& get();

    void addRepeatingTimer(std::chrono::seconds interval, std::chrono::seconds duration);
    void update();

private:
    TimerManager();
    std::vector<Timer> _timers;
};

#endif // TIMERMANAGER_H
