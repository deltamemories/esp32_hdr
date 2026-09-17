#include <Preferences.h>
#include "esp_camera.h"


Preferences preferences;
camera_config_t config;

void setup() {
  Serial.begin(115200);
  preferences.begin("config", false);

  if (init_camera() != ESP_OK) {
    Serial.println("camera init failed");
  }
}

void loop(){
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    return;
  }

  uint8_t *raw_pixels = fb->buf;
  size_t len = fb->len;

  Serial.println(len);

  esp_camera_fb_return(fb);

  delay(100);
}

esp_err_t init_camera() {
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 1;
  config.pin_d1 = 2;
  config.pin_d2 = 4;
  config.pin_d3 = 5;
  config.pin_d4 = 6;
  config.pin_d5 = 7;
  config.pin_d6 = 15;
  config.pin_d7 = 16;
  config.pin_xclk = 17;
  config.pin_pclk = 18;
  config.pin_vsync = 21;
  config.pin_href = 38;
  config.pin_sscb_sda = 41;
  config.pin_sscb_scl = 42;
  config.pin_pwdn = 40;
  config.pin_reset = 39;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_YUV422;
  config.frame_size = FRAMESIZE_VGA;
  config.fb_count = 2;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.grab_mode = CAMERA_GRAB_LATEST;

  return esp_camera_init(&config);
}
