// activeObject.cpp
#include "activeObject.h"
#include <cstring>
#include <stdio.h>
#include "events.h"
#include "esp_log.h"

ActiveObject::ActiveObject(const std::string& name, size_t stackSize, size_t queueSize)
    : _name(name),
      _timer(name + ".timer", false, [this](Event* e) {
          if (e != nullptr) {
              this->Post(e);
          }
      }) {
    _queue = xQueueCreate(queueSize, sizeof(Event*));
    if (_queue == nullptr) {
        ESP_LOGE("ActiveObject", "Failed to create queue for %s", _name.c_str());
    }
    
    BaseType_t result = xTaskCreatePinnedToCore(
        taskDispatcher, 
        _name.c_str(), 
        stackSize, 
        this, 
        1, 
        &_taskHandle, 
        1
    );
    
    if (result != pdPASS) {
        ESP_LOGE("ActiveObject", "Failed to create task for %s", _name.c_str());
        _taskHandle = nullptr;
    }
}

ActiveObject::~ActiveObject() {
    if (_taskHandle != nullptr) {
        vTaskDelete(_taskHandle);
    }
    if (_queue != nullptr) {
        // Free any remaining events in the queue before deleting it
        Event* e = nullptr;
        while (xQueueReceive(_queue, &e, 0) == pdPASS) {
            if (e != nullptr) {
                delete e;
            }
        }
        vQueueDelete(_queue);
    }
}

bool ActiveObject::Start() {
    return _taskHandle != nullptr;
}

BaseType_t ActiveObject::Post(Event* e) {
    if (e == nullptr) return pdFAIL;
    if (_queue == nullptr) return pdFAIL;
    
    BaseType_t result = xQueueSend(_queue, &e, portMAX_DELAY);
    if (result != pdPASS) {
        // If we couldn't post the event, delete it to avoid memory leak
        delete e;
        ESP_LOGE("ActiveObject", "%s: Failed to post event", _name.c_str());
    }
    return result;
}

BaseType_t ActiveObject::PostISR(Event* e) {
    if (e == nullptr) return pdFAIL;
    if (_queue == nullptr) return pdFAIL;
    
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // Ignore return value since we can't handle errors in an ISR
    (void)xQueueSendFromISR(_queue, &e, &xHigherPriorityTaskWoken);
    
    // Note: We can't delete the event here if posting fails, as we're in an ISR
    // The caller must handle cleanup if this method returns pdFAIL
    
    return xHigherPriorityTaskWoken;
}

bool ActiveObject::PeekQueue(Event** e) {
    if (_queue == nullptr) return false;
    return xQueuePeek(_queue, e, 0) == pdPASS;
}

QueueHandle_t& ActiveObject::getQueue() {
    return _queue;
}

void ActiveObject::taskDispatcher(void* data) {
    ActiveObject* object = static_cast<ActiveObject*>(data);
    if (object != nullptr) {
        object->eventLoop();
    } else {
        ESP_LOGE("ActiveObject", "Invalid object pointer in taskDispatcher");
        vTaskDelete(NULL); // Delete current task if object is invalid
    }
}

void ActiveObject::eventLoop() {
    Event* e = nullptr;
    while (true) {
        if (_queue != nullptr && uxQueueMessagesWaiting(_queue) > 0) {
            // Process events by priority (0 = highest, 2 = lowest)
            for (int p = 0; p <= 2; ++p) {
                UBaseType_t count = uxQueueMessagesWaiting(_queue);
                for (UBaseType_t i = 0; i < count; ++i) {
                    if (xQueuePeek(_queue, &e, 0) == pdPASS && 
                        e != nullptr && 
                        e->getPriority() == static_cast<Event::Priority>(p)) {
                        
                        // Remove from queue now that we've verified it matches our criteria
                        xQueueReceive(_queue, &e, 0);
                        
                        ESP_LOGI("ActiveObject", "[%s] Handling Event: %s (Priority: %d)",
                               _name.c_str(), Event::typeToString(e->getType()), static_cast<int>(e->getPriority()));
                        
                        // Handle the event - no exception handling since it's typically 
                        // disabled in ESP32 applications
                        Dispatcher(e);
                        
                        // No error checking via exceptions - would need to be handled differently
                        // in an embedded system, potentially with return codes
                        
                        delete e;
                        break;
                    }
                }
                // Allow other tasks to run between priority levels
                taskYIELD();
            }
        } else {
            // No messages in queue, sleep for a short time
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}