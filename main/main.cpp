
#include <cstdio>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_event.h"
#include "esp_log.h"

#include "nvs_flash.h"

#include "activeObject.h"
#include "events.h"
#include <stdio.h>

static const char* TAG = "MAIN"; 

extern "C" void app_main(void)
{
    // Initialize Non-Volatile storgae (NVS)
    esp_err_t state = nvs_flash_init( ); 
    if( state == ESP_ERR_NVS_NO_FREE_PAGES || 
        state == ESP_ERR_NVS_NEW_VERSION_FOUND )
    {
        ESP_ERROR_CHECK( nvs_flash_erase( )); 
        ESP_ERROR_CHECK( nvs_flash_init( )); 
    }

    ESP_ERROR_CHECK( state ); 

    // Initialize the event loop system
    ESP_ERROR_CHECK( esp_event_loop_create_default( )); 

    ESP_LOGI( TAG, "System initialized"); 


    static SensorActor sensor;
    static DisplayActor display;
    static LoggerActor logger;

    sensor.Start();
    display.Start();
    logger.Start();

    sensor.Post(new OnStart());
    display.Post(new OnStart());
}