#include "camera.h"
#include "config.h"
#include "logger.h"
#include <Arduino.h>

static const char *TAG = "Camera";

EventDispatcher *Camera::eventDispatcher = nullptr;

void Camera::begin(EventDispatcher &dispatcher) {
    eventDispatcher = &dispatcher;
    if (initCamera() == ESP_OK) {
        LOG_I(TAG, "Camera initialized successfully");
    } else {
        LOG_E(TAG, "Failed to initialize camera");
    }

}

void Camera::captureImage() {
    camera_fb_t *fb;

    fb = esp_camera_fb_get();
    esp_camera_fb_return(fb);

    fb = esp_camera_fb_get();
    LOG_I(TAG, "Image captured: %dx%d", fb->width, fb->height);

    if (!fb) {
        LOG_E(TAG, "Image capture failed");
        esp_camera_fb_return(fb);
        ESP.restart();
        return;
    }

    eventDispatcher->dispatchEvent(
            {IMAGE_CAPTURED, std::string(reinterpret_cast<const char *>(fb->buf), fb->len)}
    );
    esp_camera_fb_return(fb);
}

esp_err_t Camera::initCamera() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_UXGA;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 8;
    config.fb_count = 1;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        LOG_E(TAG, "Failed to initialize camera: 0x%x", err);
        return err;
    }

    sensor_t *s = esp_camera_sensor_get();
    s->set_framesize(s, FRAMESIZE_UXGA);

    return ESP_OK;
}
