#include "logger.h"

LogLevel Logger::currentLogLevel = LOG_INFO;

Logger::Logger() : mqttLogger(nullptr), mqttInitialized(false) {}

void Logger::begin(PubSubClient &client) {
    if (mqttLogger != nullptr) {
        delete mqttLogger;
    }
    mqttLogger = new MqttLogger(client, "smartreceptionist/log", MqttLoggerMode::MqttAndSerial);
    mqttInitialized = true;
}

void Logger::log(LogLevel level, const char *tag, const char *format, ...) {
    return;
    if (level > currentLogLevel) return;

    va_list args;
    va_start(args, format);
    vsnprintf(logBuffer, LOG_BUFFER_SIZE, format, args);
    va_end(args);

    const char *levelStr;
    switch (level) {
        case LOG_ERROR:
            levelStr = "ERROR";
            break;
        case LOG_WARN:
            levelStr = "WARN";
            break;
        case LOG_INFO:
            levelStr = "INFO";
            break;
        case LOG_DEBUG:
            levelStr = "DEBUG";
            break;
        default:
            levelStr = "???";
            break;
    }

    char fullMessage[LOG_BUFFER_SIZE];
    snprintf(fullMessage, LOG_BUFFER_SIZE, "[%s][%s]: %s", levelStr, tag, logBuffer);

    // Add '[CAM]' to the beginning of the message
    char fullMessageWithCam[LOG_BUFFER_SIZE];
    snprintf(fullMessageWithCam, LOG_BUFFER_SIZE, "[CAM] %s", fullMessage);


    if (mqttInitialized && mqttLogger) {
        mqttLogger->println(fullMessageWithCam);
    } else {
        Serial.println(fullMessageWithCam);
    }
}

Logger logger;