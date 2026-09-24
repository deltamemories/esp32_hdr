#include <Preferences.h>

#include "esp_camera.h"

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

#include "src/FrameAccumulator.h"
#include "src/isp/IspPipeline.h"
#include "src/isp/FramePassThrough.h"

extern "C" {
  uint8_t SCCB_Read(uint8_t slv_addr, uint8_t reg);
  int SCCB_Write(uint8_t slv_addr, uint8_t reg, uint8_t data);
}

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

// camera dimensions
#define CAM_W 640
#define CAM_H 480

// camera regs
#define REG_GAIN 0x00
#define REG_COM1 0x04
#define REG_AECHH 0x07
#define REG_AECH 0x10
#define REG_COM8 0x13
#define REG_COM13 0x3D
#define REG_COM16 0x41

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 canvas(SW, SH);

FrameAccumulator frameAcc(1, CAM_W, CAM_H, 2);
IspPipeline pipeline;


void setup() {
  Serial.begin(115200);
  preferences.begin("config", false);

  // TFT init
  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.init(240, 320);
  tft.setSPISpeed(80'000'000);
  tft.invertDisplay(false);
  tft.setRotation(0);

  canvas.fillScreen(ST77XX_BLACK);
  canvas.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  canvas.setTextSize(2);

  canvas.println("setting up sensor...");
  tft.drawRGBBitmap(0, 0, canvas.getBuffer(), SW, SH);

  if (init_camera() != ESP_OK) {
    Serial.println("camera init failed");
    return;
  }

  sensor_t* s = esp_camera_sensor_get();
  if (enable_camera_manual_mode(s)) {
    set_manual_exposure(s, 100);
    set_manual_gain(s, 0x00);
  } else {
    Serial.println("manual control setup failed");
  }

  pipeline.add_step(new FramePassThrough());

  tft.setRotation(2);
}

void loop() {
  frameAcc.reset();

  while (!frameAcc.is_full()) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      continue;
    }

    frameAcc.push_frame(fb);
    esp_camera_fb_return(fb);
  }

  pipeline.execute(&frameAcc);

  uint8_t* result_frame = frameAcc.get_output_frame();
  Serial.println("render...");
  render_frame(result_frame, CAM_W, CAM_H);
}

void set_manual_exposure(sensor_t* s, uint16_t exposure_lines) {
  if (!s) return;

  uint8_t com1 = SCCB_Read(s->slv_addr, REG_COM1);
  com1 = (com1 & ~0x03) | (exposure_lines & 0x03);
  SCCB_Write(s->slv_addr, REG_COM1, com1);

  SCCB_Write(s->slv_addr, REG_AECH, (exposure_lines >> 2) & 0xFF);

  uint8_t aechh = SCCB_Read(s->slv_addr, REG_AECHH);
  aechh = (aechh & ~0x3F) | ((exposure_lines >> 10) & 0x3F);
  SCCB_Write(s->slv_addr, REG_AECHH, aechh);
}

void set_manual_gain(sensor_t* s, uint8_t gain_val) {
  if (!s) return;
  SCCB_Write(s->slv_addr, REG_GAIN, gain_val);
}

bool enable_camera_manual_mode(sensor_t* s) {
  if (!s) return false;

  if (s->set_exposure_ctrl) s->set_exposure_ctrl(s, 0);
  if (s->set_gain_ctrl) s->set_gain_ctrl(s, 0);
  if (s->set_whitebal) s->set_whitebal(s, 0);

  if (s->set_bpc) s->set_bpc(s, 0);
  if (s->set_wpc) s->set_wpc(s, 0);
  if (s->set_lenc) s->set_lenc(s, 0);

  uint8_t com8 = SCCB_Read(s->slv_addr, REG_COM8);
  com8 &= ~0x47;
  SCCB_Write(s->slv_addr, REG_COM8, com8);

  uint8_t com16 = SCCB_Read(s->slv_addr, REG_COM16);
  com16 &= ~0x40;
  SCCB_Write(s->slv_addr, REG_COM16, com16);

  uint8_t com13 = SCCB_Read(s->slv_addr, REG_COM13);
  com13 &= ~0x80;
  SCCB_Write(s->slv_addr, REG_COM13, com13);

  Serial.println("SUCCESS: Absolute static manual mode engaged");
  return true;
}

void render_frame(uint8_t* yuv_buf, int src_w, int src_h) {
  if (!yuv_buf) return;

  uint16_t* buf = canvas.getBuffer();

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