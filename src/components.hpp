#pragma once

#include "interfaces.hpp"
#include <cmath>

namespace calibration {

class ImageCaptureService : public IImageProvider {
public:
    explicit ImageCaptureService(int startFrame = 1)
        : nextFrameId_(startFrame) {}

    ImageFrame captureFrame() override {
        ImageFrame frame;
        frame.frameId = nextFrameId_++;
        frame.scene = "calibration pattern on roadway";
        frame.exposure = 1.0 + 0.15 * frame.frameId;
        return frame;
    }

private:
    FrameId nextFrameId_;
};

class CalibrationService : public ICalibrationSolver {
public:
    CalibrationResult calibrate(const ImageFrame& frame) override {
        CalibrationResult result;
        result.frameId = frame.frameId;
        result.focalLength = 750.0 + 8.0 * frame.frameId;
        result.principalX = 640.0 + 0.5 * frame.exposure;
        result.principalY = 360.0 + 0.35 * frame.exposure;
        result.reprojectionError = std::fmod(frame.frameId * 0.22 + frame.exposure * 0.08, 1.0);
        return result;
    }
};

} // namespace calibration
