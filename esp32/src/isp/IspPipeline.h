#ifndef ESP32_HDR_ISPPIPELINE_H
#define ESP32_HDR_ISPPIPELINE_H

#include <vector>
#include "IIspStep.h"
#include "../FrameAccumulator.h"

class IspPipeline {
private:
    std::vector<IIspStep*> _steps;

public:
    IspPipeline() = default;
    ~IspPipeline();

    void add_step(IIspStep* step);
    void execute(FrameAccumulator* acc);
    void clear();
};


#endif //ESP32_HDR_ISPPIPELINE_H
