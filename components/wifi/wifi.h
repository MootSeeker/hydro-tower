// wifi.h
#ifndef WIFI_ACTOR_H
#define WIFI_ACTOR_H

#include "activeObject.h"
#include "events.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include <string>

class WiFiActor : public ActiveObject {
    


public:
    WiFiActor();

    void Dispatcher(Event* e) override;
    void Configure(const std::string& ssid, const std::string& password);

private:
    enum class State {
        INIT,
        CONNECTING,
        CONNECTED
    };

    static void wifiEventHandler(void* arg, esp_event_base_t event_base,
                             int32_t event_id, void* event_data);

    std::string _ssid;
    std::string _password;
    bool _connected;
    State _state;
};

#endif // WIFI_ACTOR_H
