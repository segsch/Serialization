#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace calibration {

using json = nlohmann::json;

struct ImageFrame {
    int frameId = 0;
    std::string scene = "unknown";
    double exposure = 0.0;
};

struct CalibrationResult {
    int frameId = 0;
    double focalLength = 0.0;
    double principalX = 0.0;
    double principalY = 0.0;
    double reprojectionError = 0.0;
};

inline void to_json(json& j, const ImageFrame& frame) {
    j = json{
        {"frameId", frame.frameId},
        {"scene", frame.scene},
        {"exposure", frame.exposure}
    };
}

inline void from_json(const json& j, ImageFrame& frame) {
    j.at("frameId").get_to(frame.frameId);
    j.at("scene").get_to(frame.scene);
    j.at("exposure").get_to(frame.exposure);
}

inline void to_json(json& j, const CalibrationResult& result) {
    j = json{
        {"frameId", result.frameId},
        {"focalLength", result.focalLength},
        {"principalX", result.principalX},
        {"principalY", result.principalY},
        {"reprojectionError", result.reprojectionError}
    };
}

inline void from_json(const json& j, CalibrationResult& result) {
    j.at("frameId").get_to(result.frameId);
    j.at("focalLength").get_to(result.focalLength);
    j.at("principalX").get_to(result.principalX);
    j.at("principalY").get_to(result.principalY);
    j.at("reprojectionError").get_to(result.reprojectionError);
}

} // namespace calibration
