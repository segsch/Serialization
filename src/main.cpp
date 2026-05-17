#include "calibration_pipeline.hpp"
#include "components.hpp"
#include "players.hpp"
#include "recorders.hpp"
#include <iostream>

int main() {
    using namespace calibration;

    std::cout << "=== Car Camera Calibration Serialization Demo ===\n";

    ImageCaptureService realCapture;
    CalibrationService realSolver;
    CalibrationPipeline directPipeline(realCapture, realSolver);
    auto directResult = directPipeline.processOneFrame();
    std::cout << "Direct run: frame " << directResult.frameId
              << ", focal length " << directResult.focalLength << "\n";

    RecordingSession session;
    RecordingImageProvider recordedCapture(realCapture, session);
    RecordingCalibrationSolver recordedSolver(realSolver, session);
    CalibrationPipeline recordingPipeline(recordedCapture, recordedSolver);
    auto recordedResults = recordingPipeline.processFrames(2);
    session.saveToFile("calibration_recording.json");
    std::cout << "Recorded " << recordedResults.size() << " steps to calibration_recording.json\n";

    auto playbackSession = loadPlaybackSession("calibration_recording.json");
    PlaybackImageProvider playbackCapture(playbackSession);
    PlaybackCalibrationSolver playbackSolver(playbackSession);
    CalibrationPipeline playbackPipeline(playbackCapture, playbackSolver);
    auto replayResults = playbackPipeline.processFrames(2);
    std::cout << "Replayed " << replayResults.size() << " steps from playback.\n";

    return 0;
}
