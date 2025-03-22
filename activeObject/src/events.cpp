
#include "events.h"


EventBus& EventBus::get() {
    static EventBus instance;
    return instance;
}

void EventBus::subscribe(Event::Type type, HandlerFunc handler) {
    _handlers[type].push_back(handler);
}

void EventBus::publish(Event* e) {
    auto it = _handlers.find(e->getType());
    if (it != _handlers.end()) {
        for (auto& handler : it->second) {
            handler(e);
        }
    }
    delete e;
}