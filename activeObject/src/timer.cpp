#include "timer.h"

Timer::Timer(std::string Name, bool autoReload, std::function<void(Event*)> Callback, int Identify)
    : _timerName(Name), _identify(Identify), _callback(Callback) {
    _timerHandle = xTimerCreate(_timerName.c_str(), pdMS_TO_TICKS(1000), autoReload ? pdTRUE : pdFALSE, this, timerCallback);
}

Timer::~Timer() 
{
    xTimerStop( _timerHandle, 0 );
    xTimerDelete( _timerHandle, 0 );
    delete _event;
}

void Timer::Start( TickType_t duration ) 
{
    xTimerChangePeriod( _timerHandle, duration, 0 );
    xTimerStart( _timerHandle, 0 );
}

void Timer::Start( TickType_t duration, Event* event ) 
{
    delete _event;
    _event = event;
    Start( duration );
}

void Timer::Stop( ) 
{
    xTimerStop( _timerHandle, 0 );
}

void Timer::Reset( ) 
{
    xTimerReset( _timerHandle, 0 );
}

TimerHandle_t Timer::GetHandle( ) 
{
    return _timerHandle;
}

void Timer::timerCallback( TimerHandle_t xTimer ) 
{
    Timer* timer = static_cast<Timer*>( pvTimerGetTimerID( xTimer ));
    if ( timer && timer->_callback ) 
    {
        timer->_callback( timer->_event );
    }
}
