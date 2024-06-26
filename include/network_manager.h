// network_manager.h
#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <cstdint>
#include "WebSocketsClient.h"
#include "events.h"

class NetworkManager {
public:
    static void begin(EventDispatcher &dispatcher);

    static void loop();

    static void sendImage(const uint8_t *data, size_t length);

    static void sendInitMessage();

private:
    static EventDispatcher *eventDispatcher;
    static WebSocketsClient webSocket;

    static void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);

};

#endif // NETWORK_MANAGER_H