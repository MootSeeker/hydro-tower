#include "events.h"

EventBus& EventBus::get() {
    static EventBus instance;
    return instance;
}

void EventBus::subscribe(Event::Type type, HandlerFunc handler) {
    _handlers[type].push_back(handler);
}

void EventBus::publish(Event* e) {
    std::map<Event::Type, std::vector<HandlerFunc>>::iterator found = _handlers.find(e->getType());
    if (found != _handlers.end()) {
        std::vector<HandlerFunc>& handlers = found->second;
        for (size_t i = 0; i < handlers.size(); ++i) {
            handlers[i](e);
        }
    }
}

const char* Event::typeToString(Event::Type type) {
    switch (type) {
        case Event::Type::OnStart: return "OnStart";
        case Event::Type::Measurement: return "Measurement";
        case Event::Type::ScreenRefresh: return "ScreenRefresh";
        case Event::Type::ButtonClicked: return "ButtonClicked";
        default: return "Unknown";
    }
}
