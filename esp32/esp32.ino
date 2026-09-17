#include <Preferences.h>

#include "esp_camera.h"

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

Preferences preferences;
camera_config_t config;

#define TFT_CS 10
#define TFT_DC 13
#define TFT_RST 14
#define TFT_MOSI 11
#define TFT_SCLK 12

// screen dimensions
#define SW 240
#define SH 320

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 canvas(SW, SH);

void setup() {
  Serial.begin(115200);
  preferences.begin("config", false);

  // TFT init
  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.init(240, 320);
  tft.setSPISpeed(80'000'000);
  tft.invertDisplay(false);
  tft.setRotation(0);
  // TFT init end

  if (init_camera() != ESP_OK) {
    Serial.println("camera init failed");
  }

  canvas.fillScreen(ST77XX_BLACK);
  canvas.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  canvas.setTextSize(2);
  canvas.println("test");
  
  tft.drawRGBBitmap(0, 0, canvas.getBuffer(), SW, SH);
}

void loop() {
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
