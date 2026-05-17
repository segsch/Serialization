#pragma once

#include "interfaces.hpp"
#include <fstream>
#include <unordered_map>
#include <stdexcept>

namespace calibration {

struct PlaybackSession {
    std::vector<ImageFrame> captures;
    std::unordered_map<int, CalibrationResult> calibrations;
};

inline PlaybackSession loadPlaybackSession(const std::string& filePath) {
    std::ifstream input(filePath);
    if (!input) {
        throw std::runtime_error("Cannot open playback file: " + filePath);
    }

    json journal;
    input >> journal;

    PlaybackSession session;
    for (auto const& entry : journal.at("captures")) {
        session.captures.push_back(entry.get<ImageFrame>());
    }

    for (auto const& entry : journal.at("calibrations")) {
        auto result = entry.get<CalibrationResult>();
        session.calibrations[result.frameId] = result;
    }

    return session;
}

class PlaybackImageProvider : public IImageProvider {
public:
    explicit PlaybackImageProvider(const PlaybackSession& session)
        : session_(session), nextIndex_(0) {}

    ImageFrame captureFrame() override {
        if (nextIndex_ >= session_.captures.size()) {
            throw std::out_of_range("PlaybackImageProvider has no more recorded frames.");
        }
        return session_.captures[nextIndex_++];
    }

private:
    const PlaybackSession& session_;
    size_t nextIndex_;
};

class PlaybackCalibrationSolver : public ICalibrationSolver {
public:
    explicit PlaybackCalibrationSolver(const PlaybackSession& session)
        : session_(session) {}

    CalibrationResult calibrate(const ImageFrame& frame) override {
        auto it = session_.calibrations.find(frame.frameId);
        if (it == session_.calibrations.end()) {
            throw std::runtime_error("No playback entry for frameId " + std::to_string(frame.frameId));
        }
        return it->second;
    }

private:
    const PlaybackSession& session_;
};

} // namespace calibration
