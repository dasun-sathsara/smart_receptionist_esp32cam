
#ifndef CAM_ESP_NOW_MANAGER_H
#define CAM_ESP_NOW_MANAGER_H

#include <Arduino.h>
#include <esp_now.h>
#include "events.h"

#define ESPNOW_CHANNEL 1

class ESPNow {
public:
    static void begin(EventDispatcher &dispatcher);

private:
    static void onDataReceived(const uint8_t *mac, const uint8_t *data, int len);

    static void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);

    static EventDispatcher *eventDispatcher;
};


#endif //CAM_ESP_NOW_MANAGER_H
