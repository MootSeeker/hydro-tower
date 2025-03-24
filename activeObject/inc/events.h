#ifndef EVENTS_H
#define EVENTS_H

#include <functional>
#include <map>
#include <vector>
#include <cstdint>

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
    virtual Event* clone() const = 0;

    static const char* typeToString(Type type);

    Event(const char* source);
    uint32_t getId() const;
    const char* getSource() const;

private:
    uint32_t _id;
    const char* _source;
};

class OnStart : public Event {
public:
    OnStart(const char* source = "Unknown") : Event(source) {}
    Type getType() const override { return Type::OnStart; }
    Event* clone() const override;
};

class MeasurementEvent : public Event {
public:
    MeasurementEvent(float value, const char* source = "Unknown")
        : Event(source), _value(value) {}
    Type getType() const override { return Type::Measurement; }
    float getValue() const { return _value; }
    Event* clone() const override;

private:
    float _value;
};

class ScreenRefreshEvent : public Event {
public:
    ScreenRefreshEvent(const char* source = "Unknown") : Event(source) {}
    Type getType() const override { return Type::ScreenRefresh; }
    Priority getPriority() const override { return Priority::Low; }
    Event* clone() const override;
};

class ButtonClicked : public Event {
public:
    ButtonClicked(int id, int state, const char* source = "Unknown")
        : Event(source), _buttonID(id), _state(state) {}
    Type getType() const override { return Type::ButtonClicked; }
    int getID() const { return _buttonID; }
    int getState() const { return _state; }
    Event* clone() const override;

private:
    int _buttonID;
    int _state;
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

#endif // EVENTS_H