
#ifndef EVENTS_H
#define EVENTS_H

#include <string>
#include <functional>
#include <vector>
#include <map>

class Event {
    public:
        enum class Priority {
            HIGH = 0,
            NORMAL = 1,
            LOW = 2
        };
    
        enum class Type {
            OnStart,
            ButtonClicked,
            Measurement,
            ScreenRefresh
        };
    
        Event(Type type, Priority priority = Priority::NORMAL)
            : _type(type), _priority(priority) {}
        virtual ~Event() {}
    
        Type getType() const { return _type; }
        Priority getPriority() const { return _priority; }
    
    private:
        Type _type;
        Priority _priority;
    };
    
    class OnStart : public Event {
    public:
        OnStart(Priority p = Priority::NORMAL) : Event(Type::OnStart, p) {}
    };
    
    class ButtonClicked : public Event {
    public:
        ButtonClicked(int id, int state, Priority p = Priority::HIGH)
            : Event(Type::ButtonClicked, p), _buttonID(id), _state(state) {}
    
        int getID() const { return _buttonID; }
        int getState() const { return _state; }
    
    private:
        int _buttonID;
        int _state;
    };
    
    class MeasurementEvent : public Event {
    public:
        MeasurementEvent(float value, Priority p = Priority::NORMAL)
            : Event(Type::Measurement, p), _value(value) {}
    
        float getValue() const { return _value; }
    
    private:
        float _value;
    };
    
    class ScreenRefreshEvent : public Event {
    public:
        ScreenRefreshEvent(Priority p = Priority::LOW)
            : Event(Type::ScreenRefresh, p) {}
    };
    
    class EventBus {
    public:
        using HandlerFunc = std::function<void(Event*)>;
    
        static EventBus& get();
    
        void subscribe(Event::Type type, HandlerFunc handler);
        void publish(Event* e);
    
    private:
        std::map<Event::Type, std::vector<HandlerFunc>> _handlers;
    };
    

#endif // End: EVENTS_H