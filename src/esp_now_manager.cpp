#include "esp_now_manager.h"
#include "logger.h"

static const char *TAG = "ESPNow";

EventDispatcher *ESPNow::eventDispatcher = nullptr;

void ESPNow::begin(EventDispatcher &dispatcher) {
    eventDispatcher = &dispatcher;

    if (esp_now_init() != ESP_OK) {
        LOG_E(TAG, "Error initializing ESP-NOW");
        return;
    }

    esp_now_register_recv_cb(onDataReceived);
    esp_now_register_send_cb(onDataSent);

    LOG_I(TAG, "ESP-NOW initialized successfully");
}

void ESPNow::onDataReceived(const uint8_t *mac, const uint8_t *data, int len) {
    char *receivedData = (char *) malloc(len + 1);
    memcpy(receivedData, data, len);
    receivedData[len] = '\0';

    LOG_I(TAG, "Received command: %s", receivedData);

    if (strcmp(receivedData, "capture_image") == 0) {
        eventDispatcher->dispatchEvent({CMD_CAPTURE_IMAGE, ""});
    }

    free(receivedData);
}

void ESPNow::onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    LOG_I(TAG, "Last Packet Send Status: %s", status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}