#ifndef EVENTBUS_H
#define EVENTBUS_H

#include <map>
#include <memory>
#include <vector>
#include <functional>
#include "events.h"

class EventBus {
public:
    using HandlerFunc = std::function<void(Event*)>;

    static EventBus& get();
    void subscribe(Event::Type type, HandlerFunc handler);
    void publish(Event* e);

private:
    EventBus() = default;

    using HandlerList = std::vector<HandlerFunc>;
    std::map<Event::Type, std::shared_ptr<HandlerList>> _handlers;
};

#endif // EVENTBUS_H