
#ifndef EVENTS_H
#define EVENTS_H

#include <string>
#include <functional>
#include <vector>
#include <map>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <queue>

// Base Event class
class Event {
public:
    enum class Priority {
        HIGH = 0,
        NORMAL = 1,
        LOW = 2
    };

    Event(Priority priority = Priority::NORMAL) : _priority(priority) {}
    virtual ~Event() {}

    Priority getPriority() const { return _priority; }

private:
    Priority _priority;
};

// Example event types
class OnStart : public Event {
public:
    OnStart(Priority p = Priority::NORMAL) : Event(p) {}
};

class ButtonClicked : public Event {
public:
    ButtonClicked(int id, int state, Priority p = Priority::HIGH) : Event(p), _buttonID(id), _state(state) {}
    inline int getID() const { return _buttonID; }
    inline int getState() const { return _state; }

private:
    int _buttonID;
    int _state;
};

class MeasurementEvent : public Event {
public:
    MeasurementEvent(float value, Priority p = Priority::NORMAL) : Event(p), _value(value) {}
    float getValue() const { return _value; }

private:
    float _value;
};

class ScreenRefreshEvent : public Event {
public:
    ScreenRefreshEvent(Priority p = Priority::LOW) : Event(p) {}
};

class EventBus {
public:
    using HandlerFunc = std::function<void(Event*)>;

    static EventBus& get();

    void subscribe(const std::string& eventType, HandlerFunc handler);
    void publish(Event* e);

    template<typename T>
    void subscribe(std::function<void(T*)> handler) {
        std::string type = typeid(T).name();
        _handlers[type].push_back([handler](Event* e) {
            if (auto casted = dynamic_cast<T*>(e)) {
                handler(casted);
            }
        });
    }

private:
    EventBus() = default;
    std::map<std::string, std::vector<HandlerFunc>> _handlers;
};

#endif // End: EVENTS_H