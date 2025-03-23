
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
            Event* copy = nullptr;
            switch (e->getType()) {
                case Event::Type::Measurement:
                    copy = new MeasurementEvent(*static_cast<MeasurementEvent*>(e));
                    break;
                case Event::Type::ScreenRefresh:
                    copy = new ScreenRefreshEvent(*static_cast<ScreenRefreshEvent*>(e));
                    break;
                case Event::Type::OnStart:
                    copy = new OnStart(*static_cast<OnStart*>(e));
                    break;
                case Event::Type::ButtonClicked:
                    copy = new ButtonClicked(*static_cast<ButtonClicked*>(e));
                    break;
                default:
                    break;
            }
            if (copy) handler(copy);
        }
    }
    delete e;
}