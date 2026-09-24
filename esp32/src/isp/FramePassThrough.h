#ifndef ESP32_HDR_FRAMEPASSTHROUGH_H
#define ESP32_HDR_FRAMEPASSTHROUGH_H

#include <Arduino.h>

#include "IIspStep.h"
#include "../FrameAccumulator.h"

class FramePassThrough : public IIspStep {
public:
    void process(FrameAccumulator *acc) override;
};


#endif //ESP32_HDR_FRAMEPASSTHROUGH_H
