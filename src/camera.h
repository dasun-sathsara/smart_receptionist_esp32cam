// camera.h
#ifndef CAMERA_H
#define CAMERA_H

#include <esp_camera.h>
#include "events.h"

class Camera {
public:
    static void begin(EventDispatcher &dispatcher);

    static void captureImage();


private:
    static EventDispatcher *eventDispatcher;

    static esp_err_t initCamera();

};

#endif // CAMERA_H