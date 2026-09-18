#ifndef ISP_H
#define ISP_H

#include <Arduino.h>
#include "esp_camera.h"

class FrameAccumulator {
private:
  uint8_t** _frames;
  uint8_t* _output_frame;
  int _max_frames;
  int _current_count;
  int _width;
  int _height;
  int _frame_size;

  void free_buffers();

public:
  FrameAccumulator(int max_frames, int width, int height, int bytes_per_pixel);
  ~FrameAccumulator();

  bool resize(int new_count);
  bool push_frame(camera_fb_t* fb);
  void reset();
  bool is_full() const;

  uint8_t** get_frames();
  int get_frames_count() const;
  int get_max_frames() const;
  uint8_t* get_output_frame();
  int get_width() const;
  int get_height() const;
};

#endif
