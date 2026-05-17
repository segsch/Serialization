#pragma once

#include "interfaces.hpp"
#include <vector>

namespace calibration {

class CalibrationPipeline {
public:
    CalibrationPipeline(IImageProvider& provider, ICalibrationSolver& solver)
        : provider_(provider), solver_(solver) {}

    CalibrationResult processOneFrame() {
        auto frame = provider_.captureFrame();
        return solver_.calibrate(frame);
    }

    std::vector<CalibrationResult> processFrames(int count) {
        std::vector<CalibrationResult> results;
        results.reserve(count);
        for (int i = 0; i < count; ++i) {
            results.push_back(processOneFrame());
        }
        return results;
    }

private:
    IImageProvider& provider_;
    ICalibrationSolver& solver_;
};

} // namespace calibration
