#include <Arduino.h>
#include "events.h"
#include "camera.h"
#include "network_manager.h"
#include "esp_now_manager.h"
#include <esp_heap_caps.h>
#include "logger.h"

static const char *TAG = "MAIN";

EventDispatcher eventDispatcher;
NetworkManager network;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setup() {
    Serial.begin(115200);

    eventDispatcher.registerCallback(WS_CONNECTED, [](const Event &) {
        NetworkManager::sendInitMessage();
    });

    eventDispatcher.registerCallback(CMD_CAPTURE_IMAGE, [&](const Event &event) {
        Camera::captureImage();
    });

    eventDispatcher.registerCallback(IMAGE_CAPTURED, [&](const Event &event) {
        NetworkManager::sendImage(reinterpret_cast<const uint8_t *>(event.data.c_str()), event.data.length());
        LOG_I(TAG, "Image sent");
    });

    network.begin(eventDispatcher);
    vTaskDelay(2000);

    // Initialize MQTT client
    mqttClient.setServer("192.168.17.218", 1883);
    logger.begin(mqttClient);
    mqttClient.connect("SmartReceptionist");

    Camera::begin(eventDispatcher);
    ESPNow::begin(eventDispatcher);

}

void loop() {
}