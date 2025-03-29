// wifi.cpp
#include "wifi.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include <cstring>

static const char* TAG = "WiFiActor";

WiFiComm::WiFiComm( ) : _txQueue( xQueueCreate( 10, sizeof( std::string* )))
{

}

void WiFiComm::Start( )
{
    xTaskCreate( taskLoop, "WiFiCommTask", 4096, this, 5, &_task ); 
}

void WiFiComm::Send( const std::string& payload )
{
    std::string* msg = new std::string( payload ); 

    if( xQueueSend( _txQueue, &msg, 0 ) != pdPASS )
    {
        delete msg; 
        ESP_LOGW("WiFiComm", "⚠️ Queue full, message dropped");
    } else {
        ESP_LOGI("WiFiComm", "📥 Queued: %s", msg->c_str());
    }
}

void WiFiComm::taskLoop( void* args )
{
    WiFiComm* self = static_cast<WiFiComm*>( args );
    std::string* msg = nullptr;

    while( pdTRUE ) 
    {
        if ( xQueueReceive( self->_txQueue, &msg, portMAX_DELAY ) == pdPASS ) 
        {
            ESP_LOGI("WiFiComm", "📤 Sending: %s", msg->c_str( ));
            delete msg;
        }
    } 
}




// Konstruktor
WiFiActor::WiFiActor() : ActiveObject( "WiFi", 4096, 10 ), _connected( false ) 
{
    esp_netif_init( );
    esp_event_loop_create_default( );
    esp_netif_create_default_wifi_sta( );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT( );         
    esp_wifi_init( &cfg );

    esp_event_handler_instance_register( WIFI_EVENT,
                                         ESP_EVENT_ANY_ID,
                                         &WiFiActor::wifiEventHandler,
                                         this,
                                         nullptr );

    esp_event_handler_instance_register( IP_EVENT,
                                         IP_EVENT_STA_GOT_IP,
                                         &WiFiActor::wifiEventHandler,
                                         this,
                                         nullptr );

    _comm.Start( ); 
}

// Konfiguration
void WiFiActor::Configure(const std::string& ssid, const std::string& password) {
    _ssid = ssid;
    _password = password;

    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.sta.ssid, ssid.c_str(), sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, password.c_str(), sizeof(wifi_config.sta.password));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
    esp_wifi_connect();

    ESP_LOGI(TAG, "WiFi configured with SSID: %s", _ssid.c_str());
}

// Dispatcher
void WiFiActor::Dispatcher(Event* e) {
    switch (_state) {
        case State::INIT:
            if (e->getType() == Event::Type::OnStart) {
                printf("[WiFi] INIT → CONNECTING\n");
                _state = State::CONNECTING;
                Configure(_ssid, _password);
            }
            break;

        case State::CONNECTING:
            if (e->getType() == Event::Type::WiFiConnected) {
                printf("[WiFi] ✅ Connected\n");
                _state = State::CONNECTED;

                if (_retries > 0) {
                    Post(new WiFiRestoredEvent("WiFi"));
                }

                _retries = 0;
            }
            else if (e->getType() == Event::Type::WiFiDisconnected) {
                printf("[WiFi] ❌ Disconnected while connecting\n");

                if (_retries < MAX_RETRIES) {
                    _retries++;
                    printf("[WiFi] 🔁 Retry #%d...\n", _retries);
                    esp_wifi_connect();
                } else {
                    printf("[WiFi] ❌ Max retries reached → FAILED\n");
                    _state = State::FAILED;
                    _retries = 0;
                    Post(new WiFiFailedEvent("WiFi"));
                }
            }
            else if (e->getType() == Event::Type::WiFiShutdown) {
                printf("[WiFi] 🔻 Shutdown while connecting\n");
                Shutdown();
                _state = State::INIT;
                _retries = 0;
            }
            break;

        case State::CONNECTED:
            if (e->getType() == Event::Type::WiFiGotIP) {
                WiFiGotIPEvent* ipEvent = static_cast<WiFiGotIPEvent*>(e);
                printf("[WiFi] 📡 Got IP: %s\n", ipEvent->getIP().c_str());
            }
            else if (e->getType() == Event::Type::WiFiDisconnected) {
                printf("[WiFi] ⚠️ Connection lost\n");

                if (_retries < MAX_RETRIES) {
                    _retries++;
                    printf("[WiFi] 🔁 Retry #%d...\n", _retries);
                    _state = State::CONNECTING;
                    esp_wifi_connect();
                } else {
                    printf("[WiFi] ❌ Max retries reached → FAILED\n");
                    _state = State::FAILED;
                    _retries = 0;
                    Post(new WiFiFailedEvent("WiFi"));
                }
            }
            else if (e->getType() == Event::Type::WiFiDisconnectedByRequest) {
                printf("[WiFi] 🔌 Disconnected manually\n");
                Disconnect();
                _state = State::INIT;
                _retries = 0;
            }
            else if (e->getType() == Event::Type::WiFiShutdown) {
                printf("[WiFi] 🔻 Shutdown requested\n");
                Shutdown();
                _state = State::INIT;
                _retries = 0;
            }
            break;

        case State::FAILED:
            printf("[WiFi] ⛔ Ignoring event in FAILED state: %s\n", Event::typeToString(e->getType()));
            break;
    }
}



// Event-Handler (Callback aus ESP-IDF)
void WiFiActor::wifiEventHandler(void* arg, esp_event_base_t base, int32_t id, void* data) {
    WiFiActor* self = static_cast<WiFiActor*>(arg);

    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WiFi STA Start → waiting for connect...");
        
    } 
    else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "WiFi Disconnected → Posting Event");
        self->_connected = false;
        self->Post(new WiFiDisconnectedEvent());  // Dispatcher übernimmt Retry-Logik
    } 
    else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "WiFi Connected → Got IP");
        self->_connected = true;
        self->Post(new WiFiConnectedEvent());
    }
}


void WiFiActor::Disconnect( void ) {
    esp_wifi_disconnect();
    ESP_LOGI("WiFi", "Disconnected manually");
}

void WiFiActor::Shutdown( void ) {
    esp_wifi_stop();
    esp_wifi_deinit();
    ESP_LOGI("WiFi", "WiFi shut down");
}
