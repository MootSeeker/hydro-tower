
#ifndef APP_H
#define APP_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

namespace App
{

    enum class AppEvent 
    {
        NONE, 
        SENSOR_UPDATE, 
        WIFI_CONNECTED, 
        WIDI_DISCONNECTED, 
        UI_REFRESH
    }; 

    struct EventMessage
    {
        AppEvent event; 
        void *data; 
    }; 

    class Application
    {
        public:
            static Application& GetInstance( void );  
            void Start( void ); 
            void PostEvent( AppEvent event, void *data = nullptr ); 

        private: 
            Application( void ); 
            static void eventHandlerTask( void* arg ); 
    
            TaskHandle_t _eventTaskHandle; 
            QueueHandle_t _eventQueue; 
    
    }; // End: class Application    
} // End: namespace App
#endif // End: APP_H