#include "FramePassThrough.h"
#include <Arduino.h>


void FramePassThrough::process(FrameAccumulator *acc) {
    if (acc->get_frames_count() == 0) return;

    uint8_t** frames = acc->get_frames();
    uint8_t* output = acc->get_output_frame();
    int size = acc->get_frame_size();

    for (int i = 0; i < size; i++) {
        output[i] = frames[0][i];
    }
}
