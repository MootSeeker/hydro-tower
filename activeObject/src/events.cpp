// events.cpp
#include "events.h"
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

const char* Event::typeToString(Event::Type type) {
    switch (type) {
        case Type::OnStart: return "OnStart";
        case Type::Measurement: return "Measurement";
        case Type::ScreenRefresh: return "ScreenRefresh";
        case Type::ButtonClicked: return "ButtonClicked";
        case Type::SystemReset: return "SystemReset";
        case Type::WiFiConnected: return "WiFiConnected";
        case Type::WiFiDisconnected: return "WiFiDisconnected";
        case Type::WiFiConnecting: return "WiFiConnecting";
        case Type::WiFiReconnect: return "WiFiReconnect";
        case Type::WiFiRestored: return "WiFiRestored";
        case Type::WiFiFailed: return "WiFiFailed";
        case Type::WiFiGotIP: return "WiFiGotIP";
        case Type::WiFiShutdown: return "WiFiShutdown";
        case Type::WiFiDisconnectedByRequest: return "WiFiDisconnectedByRequest";
        case Type::LedControl: return "LedControl";
        case Type::LedStop: return "LedStop";
        default: return "Unknown";
    }
}
