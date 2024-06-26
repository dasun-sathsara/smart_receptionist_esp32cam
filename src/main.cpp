#include <Arduino.h>
#include "events.h"
#include "camera.h"
#include "network_manager.h"
#include "esp_now_manager.h"
#include <esp_heap_caps.h>
#include "logger.h"

#define LOG_BUFFER_SIZE 128

// Define currentLogLevel
LogLevel currentLogLevel = LOG_DEBUG;

// Define the logger function
void logger(LogLevel level, const char *tag, const char *format, ...) {
    if (level > currentLogLevel) return; // Skip if below current log level

    char message[LOG_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(message, LOG_BUFFER_SIZE, format, args);
    va_end(args);

    // Formatted output
    Serial.printf("[%s][%s]: %s\n",
                  level == LOG_ERROR ? "ERROR" : (
                          level == LOG_WARN ? "WARN" : (
                                  level == LOG_INFO ? "INFO" : (
                                          level == LOG_DEBUG ? "DEBUG" : "???"))),
                  tag, message);
}

static const char *TAG = "MAIN";

EventDispatcher eventDispatcher;

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

    Camera::begin(eventDispatcher);
    NetworkManager::begin(eventDispatcher);
    ESPNow::begin(eventDispatcher);

}

void loop() {
    NetworkManager::loop();
}