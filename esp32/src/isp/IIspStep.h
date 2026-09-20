#ifndef ESP32_HDR_IISPSTEP_H
#define ESP32_HDR_IISPSTEP_H

#include "../FrameAccumulator.h"
#include <Arduino.h>

class IIspStep {
public:
    virtual ~IIspStep() = default;
    virtual void process(FrameAccumulator* acc) = 0;
};


#endif //ESP32_HDR_IISPSTEP_H
