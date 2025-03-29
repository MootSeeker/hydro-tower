// wifi.h
#ifndef WIFI_ACTOR_H
#define WIFI_ACTOR_H

#include "activeObject.h"
#include "events.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include <string>

/**
 * @brief   WiFi communication class 
 * 
 */
class WiFiComm 
{
    public:
        WiFiComm();
        ~WiFiComm() = default;
    
        void Start();                                  // startet interne Task
        void Send(const std::string& payload);         // fügt Nachricht zur Queue hinzu
    
    private:
        static void taskLoop(void* arg);               // loop zum Senden
        QueueHandle_t _txQueue;
        TaskHandle_t _task;
};

/**
 * @brief   WiFi Actor - Active Object class 
 */
class WiFiActor : public ActiveObject {
public:
    WiFiActor();

    void Dispatcher(Event* e) override;
    void Configure(const std::string& ssid, const std::string& password);

    void Disconnect( void ); 
    void Shutdown( void ); 

    void SendPayload( const std::string& json ) 
    {
        _comm.Send( json );
    }

private:
    static constexpr int MAX_RETRIES = 5;
    
    enum class State {
        INIT,
        CONNECTING,
        CONNECTED,
        FAILED  // ➕ Neuer Zustand
    };




    static void wifiEventHandler(void* arg, esp_event_base_t event_base,
                             int32_t event_id, void* event_data);

    std::string _ssid;
    std::string _password;
    bool _connected;
    State _state;
    int _retries = 0;

    WiFiComm _comm; 
};

#endif // WIFI_ACTOR_H
