#ifndef EVENTS_H
#define EVENTS_H

#include <functional>
#include <map>
#include <vector>

class Event {
public:
    enum class Type {
        OnStart,
        Measurement,
        ScreenRefresh,
        ButtonClicked
    };

    enum class Priority {
        High = 0,
        Normal = 1,
        Low = 2
    };

    virtual ~Event() {}
    virtual Type getType() const = 0;
    virtual Priority getPriority() const { return Priority::Normal; }
    virtual Event* Clone( ) const = 0; 

    static const char* typeToString(Type type);
};

// Event types

class OnStart : public Event {
public:
    Type getType() const override { return Type::OnStart; }
    Event* Clone( ) const override { return new OnStart(*this); }
};

class MeasurementEvent : public Event {
public:
    MeasurementEvent(float value) : _value(value) {}
    Type getType() const override { return Type::Measurement; }
    float getValue() const { return _value; }

    Event* Clone( ) const override { return new MeasurementEvent(_value); }

private:
    float _value;
};

class ScreenRefreshEvent : public Event {
public:
    Type getType() const override { return Type::ScreenRefresh; }
    Priority getPriority() const override { return Priority::Low; }
    Event* Clone( ) const override { return new ScreenRefreshEvent(*this); }
};

class ButtonClicked : public Event {
public:
    ButtonClicked(int id, int state) : _buttonID(id), _state(state) {}
    Type getType() const override { return Type::ButtonClicked; }
    int getID() const { return _buttonID; }
    int getState() const { return _state; }

    Event* Clone( ) const override { return new ButtonClicked(*this); }

private:
    int _buttonID;
    int _state;
};

// EventBus

class EventBus {
public:
    using HandlerFunc = std::function<void(Event*)>;

    static EventBus& get();
    void subscribe(Event::Type type, HandlerFunc handler);
    void publish(Event* e);

private:
    std::map<Event::Type, std::vector<HandlerFunc>> _handlers;
};

#endif // EVENTS_H
