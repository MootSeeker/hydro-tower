// wifi.cpp
#include "wifi.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include <cstring>

static const char* TAG = "WiFiActor";

// Konstruktor
WiFiActor::WiFiActor()
    : ActiveObject("WiFi", 4096, 10), _connected(false) {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT,
                                        ESP_EVENT_ANY_ID,
                                        &WiFiActor::wifiEventHandler,
                                        this,
                                        nullptr);

    esp_event_handler_instance_register(IP_EVENT,
                                        IP_EVENT_STA_GOT_IP,
                                        &WiFiActor::wifiEventHandler,
                                        this,
                                        nullptr);
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
                _state = State::CONNECTING;
                Configure(_ssid, _password);
            }
            break;

        case State::CONNECTING:
            if (e->getType() == Event::Type::WiFiConnected) {
                _state = State::CONNECTED;
                printf("[WiFi] ✅ Connected\n");
            } else if (e->getType() == Event::Type::WiFiDisconnected) {
                printf("[WiFi] ❌ Failed to connect\n");
            }
            break;

        case State::CONNECTED:
            if (e->getType() == Event::Type::WiFiGotIP) {
                WiFiGotIPEvent* ipEvent = static_cast<WiFiGotIPEvent*>(e);
                printf("[WiFi] Got IP: %s\n", ipEvent->getIP().c_str());
            } else if (e->getType() == Event::Type::WiFiDisconnected) {
                _state = State::CONNECTING;
                printf("[WiFi] 🔄 Reconnecting...\n");
                esp_wifi_connect();
            }
            break;
    }
}

// Event-Handler (Callback aus ESP-IDF)
void WiFiActor::wifiEventHandler(void* arg, esp_event_base_t base, int32_t id, void* data) {
    WiFiActor* self = static_cast<WiFiActor*>(arg);

    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WiFi STA Start → Connecting...");
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "WiFi Disconnected → Retrying...");
        esp_wifi_connect();
        self->_connected = false;
        self->Post(new OnStart("WiFiDisconnected")); // Placeholder
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "WiFi Connected → Got IP");
        self->_connected = true;
        self->Post(new OnStart("WiFiConnected")); // Placeholder
    }
}
