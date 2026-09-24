#include "IspPipeline.h"

IspPipeline::~IspPipeline() {
  clear();
}

void IspPipeline::add_step(IIspStep *step) {
  _steps.push_back(step);
}

void IspPipeline::execute(FrameAccumulator* acc) {
    for (IIspStep* step : _steps) {
        step->process(acc);
    }
}

void IspPipeline::clear() {
    for (IIspStep* step : _steps) {
        delete step;
    }
    _steps.clear();
}
