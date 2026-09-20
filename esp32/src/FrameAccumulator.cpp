#include "FrameAccumulator.h"

FrameAccumulator::FrameAccumulator(int max_frames, int width, int height, int bytes_per_pixel)
    : _frames(nullptr), _output_frame(nullptr), _max_frames(0),
      _current_count(0), _width(width), _height(height) {
  _frame_size = width * height * bytes_per_pixel;
  resize(max_frames);
}

FrameAccumulator::~FrameAccumulator() {
  free_buffers();
}

void FrameAccumulator::free_buffers() {
  if (_frames) {
    for (int i = 0; i < _max_frames; i++) {
      if (_frames[i]) free(_frames[i]);
    }
    free(_frames);
    _frames = nullptr;
  }
  if (_output_frame) {
    free(_output_frame);
    _output_frame = nullptr;
  }
}

bool FrameAccumulator::resize(int new_count) {
  if (new_count == _max_frames && _output_frame != nullptr) return true;

  free_buffers();

  _max_frames = new_count;
  _current_count = 0;

  _frames = (uint8_t**)malloc(_max_frames * sizeof(uint8_t*));
  _output_frame = (uint8_t*)ps_malloc(_frame_size);

  if (!_frames || !_output_frame) return false;

  for (int i = 0; i < _max_frames; i++) {
    _frames[i] = (uint8_t*)ps_malloc(_frame_size);
    if (!_frames[i]) return false;
  }
  return true;
}

bool FrameAccumulator::push_frame(camera_fb_t* fb) {
  if (!fb || !_frames || _current_count >= _max_frames) return false;

  memcpy(_frames[_current_count], fb->buf, fb->len);
  _current_count++;
  return true;
}

void FrameAccumulator::reset() {
  _current_count = 0;
}

bool FrameAccumulator::is_full() const {
  return _current_count >= _max_frames;
}

uint8_t** FrameAccumulator::get_frames() {
  return _frames;
}

int FrameAccumulator::get_frames_count() const {
  return _current_count;
}

int FrameAccumulator::get_max_frames() const {
  return _max_frames;
}

uint8_t* FrameAccumulator::get_output_frame() {
  return _output_frame;
}

int FrameAccumulator::get_width() const {
  return _width;
}

int FrameAccumulator::get_height() const {
  return _height;
}

int FrameAccumulator::get_frame_size() const {
  return _frame_size;
}
