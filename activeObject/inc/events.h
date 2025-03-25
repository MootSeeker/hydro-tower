#ifndef EVENTS_H
#define EVENTS_H

#include <functional>
#include <map>
#include <vector>
#include <cstdint>
#include <atomic>
#include <string>

class Event {
public:
    enum class Type {
        OnStart,
        Measurement,
        ScreenRefresh,
        ButtonClicked, 
        SystemReset, 
        WiFiConnected,
        WiFiDisconnected,
        WiFiConnecting,
        WiFiFailed,
        WiFiGotIP, 
        WiFiShutdown, 
        WiFiDisconnectedByRequest
    };

    enum class Priority {
        High = 0,
        Normal = 1,
        Low = 2
    };

    virtual ~Event() {}
    virtual Type getType() const = 0;
    virtual Priority getPriority() const { return Priority::Normal; }
    virtual Event* Clone() const = 0;

    static const char* typeToString(Type type);

    Event(const char* source);
    uint32_t getId() const;
    const char* getSource() const;

    protected:
        uint32_t _id;
        const char* _source;
        static std::atomic<uint32_t> _eventIdCounter;
};

/** Initial Event .......................................................................... */
class OnStart : public Event {
public:
    OnStart(const char* source = "Unknown") : Event(source) {}
    Type getType() const override { return Type::OnStart; }
    Event* Clone() const override;
};


/** Sensor Events .......................................................................... */
class MeasurementEvent : public Event {
public:
    MeasurementEvent(float value, const char* source = "Unknown")
        : Event(source), _value(value) {}
    Type getType() const override { return Type::Measurement; }
    float getValue() const { return _value; }
    Event* Clone() const override;

private:
    float _value;
};


/** Display Events ......................................................................... */
class ScreenRefreshEvent : public Event {
public:
    ScreenRefreshEvent(const char* source = "Unknown") : Event(source) {}
    Type getType() const override { return Type::ScreenRefresh; }
    Priority getPriority() const override { return Priority::Low; }
    Event* Clone() const override;
};


/** Button Events .......................................................................... */
class ButtonClicked : public Event {
public:
    ButtonClicked(int id, int state, const char* source = "Unknown")
        : Event(source), _buttonID(id), _state(state) {}
    Type getType() const override { return Type::ButtonClicked; }
    int getID() const { return _buttonID; }
    int getState() const { return _state; }
    Event* Clone() const override;

private:
    int _buttonID;
    int _state;
};

class SystemResetEvent : public Event {
    public:
        SystemResetEvent(const char* source = "Unknown") : Event(source) {}
        Type getType() const override { return Type::SystemReset; }
        Event* Clone() const override { return new SystemResetEvent(_source); }
    };

/** WiFi Events ............................................................................ */
class WiFiConnectedEvent : public Event {
    public:
        WiFiConnectedEvent(const char* source = "WiFi") : Event(source) {}
        Type getType() const override { return Type::WiFiConnected; }
        Event* Clone() const override { return new WiFiConnectedEvent(_source); }
    };
    
    class WiFiDisconnectedEvent : public Event {
    public:
        WiFiDisconnectedEvent(const char* source = "WiFi") : Event(source) {}
        Type getType() const override { return Type::WiFiDisconnected; }
        Event* Clone() const override { return new WiFiDisconnectedEvent(_source); }
    };
    
    class WiFiConnectingEvent : public Event {
    public:
        WiFiConnectingEvent(const char* source = "WiFi") : Event(source) {}
        Type getType() const override { return Type::WiFiConnecting; }
        Event* Clone() const override { return new WiFiConnectingEvent(_source); }
    };
    
    class WiFiFailedEvent : public Event {
    public:
        WiFiFailedEvent(const char* source = "WiFi") : Event(source) {}
        Type getType() const override { return Type::WiFiFailed; }
        Event* Clone() const override { return new WiFiFailedEvent(_source); }
    };
    
    class WiFiGotIPEvent : public Event {
    public:
        WiFiGotIPEvent(const std::string& ip, const char* source = "WiFi")
            : Event(source), _ip(ip) {}
        Type getType() const override { return Type::WiFiGotIP; }
        const std::string& getIP() const { return _ip; }
        Event* Clone() const override { return new WiFiGotIPEvent(_ip, _source); }
    
    private:
        std::string _ip;
    };

    class WiFiDisconnectedByRequestEvent : public Event {
        public:
            WiFiDisconnectedByRequestEvent(const char* source = "WiFi") : Event(source) {}
            Type getType() const override { return Type::WiFiDisconnected; }
            Event* Clone() const override { return new WiFiDisconnectedByRequestEvent(_source); }
        };
        
        class WiFiShutdownEvent : public Event {
        public:
            WiFiShutdownEvent(const char* source = "WiFi") : Event(source) {}
            Type getType() const override { return Type::WiFiFailed; }
            Event* Clone() const override { return new WiFiShutdownEvent(_source); }
        };
   
/** Event Bus ................................................................................ */
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