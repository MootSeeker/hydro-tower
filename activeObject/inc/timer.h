
#ifndef TIMER_H
#define TIMER_H

#include <string>
#include <functional>

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "events.h"

class Timer 
{
    public:
        Timer( std::string Name, bool autoReload, std::function<void(Event*)> Callback, int Identify = 0 );
        virtual ~Timer();
    
        void Start( TickType_t duration );
        void Start( TickType_t duration, Event* event );
        void Stop( );
        void Reset( );
        TimerHandle_t GetHandle( );
    
    private:
        std::string _timerName;
        int _identify;
        TimerHandle_t _timerHandle;
        Event* _event;
        std::function<void(Event*)> _callback;
    
        static void timerCallback( TimerHandle_t xTimer );
    };

#endif // End: TIMER_H