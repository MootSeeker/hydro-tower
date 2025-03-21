#include "events.h"

EventBus& EventBus::get() {
    static EventBus instance;
    return instance;
}

void EventBus::subscribe(const std::string& eventType, HandlerFunc handler) {
    _handlers[eventType].push_back(handler);
}

void EventBus::publish(Event* e) {
    std::string type = typeid(*e).name();
    auto it = _handlers.find(type);
    if (it != _handlers.end()) {
        for (auto& handler : it->second) {
            handler(e);
        }
    }
    delete e;
}