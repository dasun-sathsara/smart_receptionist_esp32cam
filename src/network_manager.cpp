// network_manager.cpp
#include "network_manager.h"
#include "config.h"
#include <ArduinoJson.h>
#include "logger.h"

static const char *TAG = "NetworkManager";

EventDispatcher *NetworkManager::eventDispatcher = nullptr;
WebSocketsClient NetworkManager::webSocket;

void NetworkManager::begin(EventDispatcher &dispatcher) {
    eventDispatcher = &dispatcher;

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    int connectionAttempts = 0;
    while (WiFiClass::status() != WL_CONNECTED && connectionAttempts < 10) {
        delay(500);
        connectionAttempts++;
    }

    if (WiFiClass::status() == WL_CONNECTED) {
        LOG_I(TAG, "Connected to WiFi network");
        eventDispatcher->dispatchEvent({WIFI_CONNECTED, ""});
    } else {
        LOG_E(TAG, "Failed to connect to WiFi network");
        eventDispatcher->dispatchEvent({WIFI_DISCONNECTED, ""});
        return;
    }

//    webSocket.beginSSL(WS_HOST, WS_PORT, nullptr, "", "wss");
    webSocket.begin(WS_SERVER, WS_PORT);
    webSocket.onEvent(webSocketEvent);
}

void NetworkManager::loop() {
    webSocket.loop();
}

void NetworkManager::webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
    JsonDocument doc;

    switch (type) {
        case WStype_DISCONNECTED:
            LOG_I(TAG, "WebSocket disconnected");
            eventDispatcher->dispatchEvent({WEBSOCKET_DISCONNECTED, ""});
            // Handle WebSocket disconnection (e.g., reattempt connection)
            break;
        case WStype_CONNECTED:
            LOG_I(TAG, "WebSocket connected");
            eventDispatcher->dispatchEvent({WEBSOCKET_CONNECTED, ""});
            break;
        case WStype_TEXT: {

            DeserializationError error = deserializeJson(doc, payload, length);

            if (error) {
                LOG_E(TAG, "Failed to parse JSON: %s", error.c_str());
                return;
            }

            const char *event_type = doc["event_type"];
            LOG_I(TAG, "Received event: %s", event_type);

            if (strcmp(event_type, "capture_image") == 0) {
                eventDispatcher->dispatchEvent({CAPTURE_IMAGE, ""});
            }


            break;
        }

    }

}

void NetworkManager::sendImage(const uint8_t *imageData, size_t imageLength) {
    // Calculate the total length with the "IMAGE:" prefix
    size_t totalLength = imageLength + 6; // 6 for "IMAGE:"

    // Create a new buffer to hold the combined data
    auto *combinedData = new uint8_t[totalLength];

    // Copy the "IMAGE:" prefix
    memcpy(combinedData, "IMAGE:", 6);

    // Copy the image data
    memcpy(combinedData + 6, imageData, imageLength);

    // Send the combined data as binary
    webSocket.sendBIN(combinedData, totalLength);

    // Free the dynamically allocated memory
    delete[] combinedData;
}

void NetworkManager::sendInitMessage() {
    webSocket.sendTXT(R"({"event_type":"init","data":{"device":"esp_cam"}})");
    ESP_LOGI(TAG, "Sent init message");
}

