#include "activeObject.h"
#include <cstring>
#include <stdio.h>
#include <typeinfo>

ActiveObject::ActiveObject(const std::string& name, size_t stackSize, size_t queueSize)
    : _timer(name + ".timer", false, [this](Event* e) { this->Post(e); }), _name(name)  {
    _queue = xQueueCreate(queueSize, sizeof(Event*));
    xTaskCreate(taskDispatcher, _name.c_str(), stackSize, this, 1, &_taskHandle);
}

ActiveObject::~ActiveObject() {
    vTaskDelete(_taskHandle);
    vQueueDelete(_queue);
}

bool ActiveObject::Start() {
    return _taskHandle != nullptr;
}

BaseType_t ActiveObject::Post(Event* e) {
    return xQueueSend(_queue, &e, portMAX_DELAY);
}

BaseType_t ActiveObject::PostISR(Event* e) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(_queue, &e, &xHigherPriorityTaskWoken);
    return xHigherPriorityTaskWoken;
}

void ActiveObject::taskDispatcher(void* data) {
    static_cast<ActiveObject*>(data)->eventLoop();
}

void ActiveObject::eventLoop() {
    Event* e = nullptr;
    while (true) {
        if (uxQueueMessagesWaiting(_queue) > 0) {
            for (int p = 0; p <= 2; ++p) {
                UBaseType_t count = uxQueueMessagesWaiting(_queue);
                for (UBaseType_t i = 0; i < count; ++i) {
                    if (xQueuePeek(_queue, &e, 0) == pdPASS && e->getPriority() == static_cast<Event::Priority>(p)) {
                        xQueueReceive(_queue, &e, 0);
                        printf("[%s] Handling Event: Type %d (Priority: %d)\n",
                               _name.c_str(), static_cast<int>(e->getType()), static_cast<int>(e->getPriority()));
                        Dispatcher(e);
                        delete e;
                        break;
                    }
                }
                taskYIELD();
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}
