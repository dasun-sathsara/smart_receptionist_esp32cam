#include "network_manager.h"
#include "config.h"
#include <ArduinoJson.h>
#include "logger.h"

static const char *TAG = "NetworkManager";

EventDispatcher *NetworkManager::eventDispatcher = nullptr;
WebSocketsClient NetworkManager::webSocket;
const char *WS_SERVER = "192.168.17.218";

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
    }

//    webSocket.beginSSL(WS_HOST, WS_PORT, nullptr, "", "wss");
    webSocket.begin(WS_SERVER, WS_PORT);
    webSocket.onEvent(webSocketEvent);

    webSocket.enableHeartbeat(15000, 3000, 2);

    xTaskCreate(NetworkManager::loop, "WiFi Task", 8192, this, 2, nullptr);
    xTaskCreate(NetworkManager::reconnectTask, "WiFi Reconnect Task", 2048, this, 1, nullptr);
}

void NetworkManager::changeWebSocketServer(const char *newServer) {
    webSocket.disconnect();
    WS_SERVER = newServer;
    webSocket.begin(WS_SERVER, WS_PORT);
    LOG_I(TAG, "WebSocket server changed to: %s", WS_SERVER);
}

[[noreturn]] void NetworkManager::loop(void *pvParameters) {
    while (true) {
        webSocket.loop();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

[[noreturn]] void NetworkManager::reconnectTask(void *pvParameters) {
    while (true) {
        if (WiFiClass::status() != WL_CONNECTED) {
            LOG_I(TAG, "Reconnecting to WiFi network...");
            WiFi.reconnect();
            int connectionAttempts = 0;
            while (WiFiClass::status() != WL_CONNECTED && connectionAttempts < 10) {
                delay(500);
                connectionAttempts++;
            }
            if (WiFiClass::status() == WL_CONNECTED) {
                LOG_I(TAG, "Reconnected to WiFi network");
                eventDispatcher->dispatchEvent({WIFI_CONNECTED, ""});
            } else {
                LOG_E(TAG, "Failed to reconnect to WiFi network");
                eventDispatcher->dispatchEvent({WIFI_DISCONNECTED, ""});
            }
        }
        vTaskDelay(pdMS_TO_TICKS(60000)); // Check every 1 minute
    }
}

void NetworkManager::webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
    JsonDocument doc;

    switch (type) {
        case WStype_DISCONNECTED:
            LOG_I(TAG, "WebSocket disconnected");
            break;
        case WStype_CONNECTED:
            LOG_I(TAG, "WebSocket connected");
            eventDispatcher->dispatchEvent({WS_CONNECTED, ""});
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
                eventDispatcher->dispatchEvent({CMD_CAPTURE_IMAGE, ""});
            } else if (strcmp(event_type, "reset_device") == 0) {
                LOG_I(TAG, "Received reset command. Restarting ESP32...");
                vTaskDelay(pdMS_TO_TICKS(1000));
                esp_restart();
            } else if (strcmp(event_type, "change_server") == 0) {
                const char *newServer = doc["data"]["server"];
                if (newServer) {
                    changeWebSocketServer(newServer);
                } else {
                    LOG_E(TAG, "Invalid change_server event: missing server address");
                }
            } else {
                LOG_W(TAG, "Unhandled event type: %s", event_type);
            }
            break;
        }
        case WStype_PING:
            LOG_I(TAG, "Received PING");
            break;

        case WStype_PONG:
            LOG_I(TAG, "Received PONG");
            break;
        default:
            LOG_W(TAG, "Unhandled WebSocket event type: %d", type);
            break;
    }
}

void NetworkManager::sendImage(const uint8_t *imageData, size_t imageLength) {
    size_t totalLength = imageLength + 6; // 6 for "IMAGE:"

    auto *combinedData = new uint8_t[totalLength];
    memcpy(combinedData, "IMAGE:", 6);
    memcpy(combinedData + 6, imageData, imageLength);
    webSocket.sendBIN(combinedData, totalLength);

    delete[] combinedData;
}

void NetworkManager::sendInitMessage() {
    webSocket.sendTXT(R"({"event_type":"init","data":{"device":"esp_cam"}})");
    ESP_LOGI(TAG, "Sent init message");
}

void NetworkManager::sendEvent(const char *eventType, const JsonObject &data) {
    JsonDocument doc;
    doc["event_type"] = eventType;
    doc["data"] = data;
    char buffer[256];
    size_t length = serializeJson(doc, buffer);
    webSocket.sendTXT(buffer, length);
    LOG_I(TAG, "Sent event: %s", eventType);
}
