#ifndef EVENTS_H
#define EVENTS_H

#include <functional>
#include <unordered_map>
#include <queue>

enum EventType {
    WIFI_CONNECTED,
    WIFI_DISCONNECTED,
    WS_CONNECTED,
    CMD_CAPTURE_IMAGE,
    IMAGE_CAPTURED,
};

struct Event {
    EventType type;
    std::string data;
    size_t dataLength;
};

using EventCallback = std::function<void(const Event &)>;

class EventDispatcher {
public:
    void registerCallback(EventType type, const EventCallback &callback);

    void dispatchEvent(const Event &event);

private:
    std::unordered_map<EventType, std::vector<EventCallback>> callbacks;
};

#endif // EVENTS_H
