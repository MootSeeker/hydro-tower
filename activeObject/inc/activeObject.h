#ifndef ACTIVE_OBJECT_H
#define ACTIVE_OBJECT_H

#include <functional>
#include <vector>
#include <memory>
#include <string>

#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "timer.h"
#include "events.h"

class ActiveObject {
    public:
        ActiveObject(const std::string& name, size_t stackSize, size_t queueSize);
        virtual ~ActiveObject();
    
        bool Start();
        BaseType_t Post(Event* e);
        BaseType_t PostISR(Event* e);
        virtual void Dispatcher(Event* e) = 0;
    
        bool PeekQueue(Event** e);
        QueueHandle_t& getQueue();
        inline Timer* getTimer() { return &_timer; }
    
    protected:
        std::string _name;
        Timer _timer;
    
    private:
        static void taskDispatcher(void* data);
        void eventLoop();
    
        TaskHandle_t _taskHandle;
        QueueHandle_t _queue;
    };

#endif // End: Active Object