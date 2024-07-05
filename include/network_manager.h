// network_manager.h
#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <cstdint>
#include "WebSocketsClient.h"
#include "events.h"
#include "ArduinoJson.h"

class NetworkManager {
public:
    void begin(EventDispatcher &dispatcher);

    [[noreturn]] static void loop(void *pvParameters);

    static void sendImage(const uint8_t *data, size_t length);

    static void sendInitMessage();

    [[noreturn]] static void reconnectTask(void *pvParameters);

    static void sendEvent(const char *eventType, const JsonObject &data);

private:
    static EventDispatcher *eventDispatcher;
    static WebSocketsClient webSocket;

    static void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);

};

#endif // NETWORK_MANAGER_H