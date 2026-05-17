#pragma once

#include "types.hpp"

namespace calibration {

struct IImageProvider {
    virtual ~IImageProvider() = default;
    virtual ImageFrame captureFrame() = 0;
};

struct ICalibrationSolver {
    virtual ~ICalibrationSolver() = default;
    virtual CalibrationResult calibrate(const ImageFrame& frame) = 0;
};

} // namespace calibration
