#include "events.h"
#include <atomic>
#include <cstdio>

#define ENABLE_EVENT_TRACE 1

std::atomic<uint32_t> Event::_eventIdCounter {1};

Event::Event(const char* source)
    : _id(_eventIdCounter.fetch_add(1)), _source(source) {}

uint32_t Event::getId() const {
    return _id;
}

const char* Event::getSource() const {
    return _source;
}

Event* OnStart::Clone() const {
    return new OnStart(getSource());
}

Event* MeasurementEvent::Clone() const {
    return new MeasurementEvent(_value, getSource());
}

Event* ScreenRefreshEvent::Clone() const {
    return new ScreenRefreshEvent(getSource());
}

Event* ButtonClicked::Clone() const {
    return new ButtonClicked(_buttonID, _state, getSource());
}

const char* Event::typeToString(Event::Type type) {
    switch( type ) 
    {
        case Event::Type::OnStart: return "OnStart";
        case Event::Type::Measurement: return "Measurement";
        case Event::Type::ScreenRefresh: return "ScreenRefresh";
        case Event::Type::ButtonClicked: return "ButtonClicked";
        case Event::Type::SystemReset: return "SystemReset";
        case Event::Type::WiFiConnected: return "WiFiConnected";
        case Event::Type::WiFiDisconnected: return "WiFiDisconnected";
        case Event::Type::WiFiConnecting: return "WiFiConnecting";
        case Event::Type::WiFiReconnect: return "WiFiReconnect";
        case Event::Type::WiFiRestored: return "WiFiRestored";
        case Event::Type::WiFiFailed: return "WiFiFailed";
        case Event::Type::WiFiGotIP: return "WiFiGotIP";
        case Event::Type::WiFiShutdown: return "WiFiShutdown";
        case Event::Type::WiFiDisconnectedByRequest: return "WiFiDisconnectedByRequest";
        case Event::Type::LedControl: return "LedControl";
        default: return "UnknownEventType";
    }
}


EventBus& EventBus::get() {
    static EventBus instance;
    return instance;
}

void EventBus::subscribe(Event::Type type, HandlerFunc handler) {
    _handlers[type].push_back(handler);
}

void EventBus::publish(Event* e) {
#if ENABLE_EVENT_TRACE
    printf("[Event:%05lu] Publish %s from %s\n", e->getId(), Event::typeToString(e->getType()), e->getSource());
#endif
    std::map<Event::Type, std::vector<HandlerFunc>>::iterator found = _handlers.find(e->getType());
    if (found != _handlers.end()) {
        std::vector<HandlerFunc>& handlers = found->second;
        for (size_t i = 0; i < handlers.size(); ++i) {
            Event* cloned = e->Clone();
#if ENABLE_EVENT_TRACE
            printf("[Event:%05lu] Clone to handler %zu (%p)\n", cloned->getId(), i, static_cast<void*>(cloned));
#endif
            handlers[i](cloned);
        }
    }
#if ENABLE_EVENT_TRACE
    printf("[Event:%05lu] Delete original (%p)\n", e->getId(), static_cast<void*>(e));
#endif
    delete e;
}