
#include "app.h"

#include "esp_log.h"

namespace App
{
    static const char* TAG = "APP"; 

    Application::Application( void ) : _eventTaskHandle( nullptr )
    {
        _eventQueue = xQueueCreate( 10, sizeof( EventMessage )); 
    }

    Application& Application::GetInstance( void )
    {
        static Application instance; 
        return instance; 
    }

    void Application::Start( void )
    {
        BaseType_t state = pdFALSE; 

        ESP_LOGI( TAG, "Starting application..." ); 

        state = xTaskCreatePinnedToCore(    eventHandlerTask, 
                                            "Event Handler", 
                                            4096, 
                                            nullptr, 
                                            2, 
                                            &_eventTaskHandle, 
                                            1 ); 
        ESP_ERROR_CHECK( state );
    }

    void Application::PostEvent( AppEvent event, void *data )
    {
        BaseType_t state = pdFALSE;

        EventMessage msg = { event, data }; 
        state = xQueueSend( _eventQueue, &msg, portMAX_DELAY ); 

        ESP_ERROR_CHECK( state ); 
    }

    void Application::eventHandlerTask( void* args )
    {
        ESP_LOGI( TAG, "Event Handler Task started on Core %d", xPortGetCoreID( ));

        EventMessage msg; 

        for( ;; )
        {
            if( xQueueReceive( GetInstance( )._eventQueue, &msg, portMAX_DELAY ))
            {
                switch( msg.event )
                {
                    case AppEvent::SENSOR_UPDATE: 
                    {
                        ESP_LOGI( TAG, "Processing SENSOR_UPDATE event" );
                        break; 
                    }

                    case AppEvent::WIFI_CONNECTED: 
                    {
                        ESP_LOGI( TAG, "Processing WIFI_CONNECTED event" );
                        break; 
                    }

                    case AppEvent::WIDI_DISCONNECTED: 
                    {
                        ESP_LOGI( TAG, "Processing WIFI_DISCONNECTED event" );
                        break; 
                    }

                    case AppEvent::UI_REFRESH: 
                    {
                        ESP_LOGI( TAG, "Processing UI_REFRESH event" );
                        break; 
                    }

                    case AppEvent::NONE: 
                    default: 
                    {
                        ESP_LOGI( TAG, "Received wrong Event - do nothing" );
                        break; 
                    }
                }
            }
        }
    }
}