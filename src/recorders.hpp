#pragma once

#include "interfaces.hpp"
#include <fstream>

namespace calibration {

struct RecordingSession {
    json journal = json{{"captures", json::array()}, {"calibrations", json::array()}};

    void saveToFile(const std::string& path) const {
        std::ofstream output(path);
        output << journal.dump(2);
    }
};

class RecordingImageProvider : public IImageProvider {
public:
    RecordingImageProvider(IImageProvider& wrapped, RecordingSession& session)
        : wrapped_(wrapped), session_(session) {}

    ImageFrame captureFrame() override {
        auto frame = wrapped_.captureFrame();
        session_.journal["captures"].push_back(frame);
        return frame;
    }

private:
    IImageProvider& wrapped_;
    RecordingSession& session_;
};

class RecordingCalibrationSolver : public ICalibrationSolver {
public:
    RecordingCalibrationSolver(ICalibrationSolver& wrapped, RecordingSession& session)
        : wrapped_(wrapped), session_(session) {}

    CalibrationResult calibrate(const ImageFrame& frame) override {
        auto result = wrapped_.calibrate(frame);
        session_.journal["calibrations"].push_back(result);
        return result;
    }

private:
    ICalibrationSolver& wrapped_;
    RecordingSession& session_;
};

} // namespace calibration
