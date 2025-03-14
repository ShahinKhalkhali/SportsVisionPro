#pragma once
#include "esp_camera.h"
#define sensor_t esp_sensor_t

// cam header (camera.h)
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM -1
#define SIOD_GPIO_NUM 2
#define SIOC_GPIO_NUM 1
#define Y9_GPIO_NUM 16
#define Y8_GPIO_NUM 39
#define Y7_GPIO_NUM 15
#define Y6_GPIO_NUM 40
#define Y5_GPIO_NUM 7
#define Y4_GPIO_NUM 41
#define Y3_GPIO_NUM 6
#define Y2_GPIO_NUM 42
#define VSYNC_GPIO_NUM 4
#define HREF_GPIO_NUM 5
#define PCLK_GPIO_NUM 38

void initCamera() {
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
    config.xclk_freq_hz = 12000000;
    config.fb_location = CAMERA_FB_IN_DRAM;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.pixel_format = PIXFORMAT_JPEG;

    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 6;
    config.fb_count = 2;

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }
}