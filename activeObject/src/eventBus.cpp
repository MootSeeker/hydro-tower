// EventBus.cpp
#include "EventBus.h"
#include <cstdio>

#define ENABLE_EVENT_TRACE 1

EventBus& EventBus::get() {
    static EventBus instance;
    return instance;
}

void EventBus::subscribe(Event::Type type, HandlerFunc handler) {
    auto& ptr = _handlers[type];
    if (!ptr) {
        ptr = std::make_shared<HandlerList>();
    } else {
        ptr = std::make_shared<HandlerList>(*ptr);
    }
    ptr->push_back(handler);
}

void EventBus::publish(Event* e) {
#if ENABLE_EVENT_TRACE
    printf("[Event:%05lu] Publish %s from %s\n", e->getId(), Event::typeToString(e->getType()), e->getSource());
#endif
    auto it = _handlers.find(e->getType());
    if (it != _handlers.end() && it->second) {
        auto handlers = it->second;
        for (size_t i = 0; i < handlers->size(); ++i) {
            Event* cloned = e->Clone();
#if ENABLE_EVENT_TRACE
            printf("[Event:%05lu] Clone to handler %zu (%p)\n", cloned->getId(), i, static_cast<void*>(cloned));
#endif
            (*handlers)[i](cloned);
        }
    }
#if ENABLE_EVENT_TRACE
    printf("[Event:%05lu] Delete original (%p)\n", e->getId(), static_cast<void*>(e));
#endif
    delete e;
}