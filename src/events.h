#ifndef EVENTS_H
#define EVENTS_H

#include <functional>
#include <unordered_map>
#include <queue>

enum EventType {
    EVENT_WIFI_CONNECTED,
    EVENT_WIFI_DISCONNECTED,
    EVENT_WEBSOCKET_CONNECTED,
    EVENT_WEBSOCKET_DISCONNECTED,
    EVENT_CAPTURE_IMAGE,
    EVENT_IMAGE_CAPTURED,
};

struct Event {
    EventType type;
    std::string data;
    size_t dataLength;
};

using EventCallback = std::function<void(const Event &)>;

class EventDispatcher {
public:
    void registerCallback(EventType type, const EventCallback& callback);

    void dispatchEvent(const Event &event);

private:
    std::unordered_map<EventType, std::vector<EventCallback>> callbacks;
};

#endif // EVENTS_H
