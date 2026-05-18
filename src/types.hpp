#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace calibration {

using json = nlohmann::json;
using FrameId = int;

struct ImageFrame {
    FrameId frameId = 0;
    std::string scene = "unknown";
    double exposure = 0.0;
};

struct CalibrationResult {
    FrameId frameId = 0;
    double focalLength = 0.0;
    double principalX = 0.0;
    double principalY = 0.0;
    // Normalized reprojection error for the calibration result.
    // In this demo, the value is produced by a synthetic formula and is
    // expected to remain in the [0.0, 1.0) range.
    double reprojectionError = 0.0;
};

// Generates to_json / from_json for JSON serialization and deserialization.
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ImageFrame, frameId, scene, exposure)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CalibrationResult, frameId, focalLength, principalX, principalY, reprojectionError)

} // namespace calibration
