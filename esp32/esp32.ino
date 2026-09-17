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


#define FRAMES_CNT 3
#define FRAME_BUF_SIZE (640 * 480 * 2)

// TODO: refactor into class
uint8_t *frames_pool[FRAMES_CNT];
size_t frames_lengths[FRAMES_CNT];
int curr_write_id = 0;


void init_frames_buffers() {
  for (int i = 0; i < FRAMES_CNT; i++) {
    frames_pool[i] = (uint8_t *)ps_malloc(FRAME_BUF_SIZE);
    if (!frames_pool[i]) {
      Serial.println("failed to allocate PSRAM for frame buffer");
    }
  }
}

void capture_frame() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    return;
  }

  uint8_t *target_buf = frames_pool[curr_write_id];
  memcpy(target_buf, fb->buf, fb->len);
  frames_lengths[curr_write_id] = fb->len;

  esp_camera_fb_return(fb);

  curr_write_id = (curr_write_id + 1) % FRAMES_CNT;
}

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

  init_frames_buffers();

  canvas.fillScreen(ST77XX_BLACK);
  canvas.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  canvas.setTextSize(2);
  canvas.println("test");

  tft.drawRGBBitmap(0, 0, canvas.getBuffer(), SW, SH);
}

void loop() {
  capture_frame();

  render_first_frame(640, 480);

  delay(1000);
}

void render_first_frame(int src_w, int src_h) {
  uint8_t *yuv_buf = frames_pool[0];
  if (!yuv_buf) {
    return;
  }

  uint16_t *buf = canvas.getBuffer();

  for (int y = 0; y < SH; y++) {
    int src_y = (y * src_h) / SH;
    for (int x = 0; x < SW; x++) {
      int src_x = (x * src_w) / SW;

      int yuv_idx = (src_y * src_w + src_x) * 2;
      int pair_base = yuv_idx & ~3;

      uint8_t y_val = yuv_buf[pair_base + ((src_x & 1) ? 2 : 0)];
      uint8_t u_val = yuv_buf[pair_base + 1];
      uint8_t v_val = yuv_buf[pair_base + 3];

      buf[y * SW + x] = yuv_to_rgb565(y_val, u_val, v_val);
    }
  }

  tft.drawRGBBitmap(0, 0, buf, SW, SH);
}

uint16_t yuv_to_rgb565(uint8_t y, uint8_t u, uint8_t v) {
  int c = (int)y - 16;
  int d = (int)u - 128;
  int e = (int)v - 128;

  int r = (298 * c + 409 * e + 128) >> 8;
  int g = (298 * c - 100 * d - 208 * e + 128) >> 8;
  int b = (298 * c + 516 * d + 128) >> 8;

  r = r < 0 ? 0 : (r > 255 ? 255 : r);
  g = g < 0 ? 0 : (g > 255 ? 255 : g);
  b = b < 0 ? 0 : (b > 255 ? 255 : b);

  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
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
