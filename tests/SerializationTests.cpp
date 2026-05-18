#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "calibration_pipeline.hpp"
#include "components.hpp"
#include "players.hpp"
#include "recorders.hpp"
#include <fstream>

using namespace calibration;
using ::testing::Return;
using ::testing::_;

// ============================================================================
// LIBRARY PURPOSE
// ============================================================================
// This library demonstrates the Decorator pattern for serialization:
// 
// Scenario 1: Direct mode - Call real components (A & B) without recording
//     CalibrationPipeline -> [ImageCaptureService, CalibrationService]
//
// Scenario 2: Recording mode - Wrap real components with recorders
//     CalibrationPipeline -> [RecordingImageProvider -> ImageCaptureService,
//                              RecordingCalibrationSolver -> CalibrationService]
//     Results are serialized to JSON.
//
// Scenario 3: Playback mode - Use players instead of real components
//     CalibrationPipeline -> [PlaybackImageProvider, PlaybackCalibrationSolver]
//     Data comes from previously recorded JSON files.
//
// Real-world context: Car camera calibration system
//   - ImageCaptureService (A): Captures frames from the vehicle's camera
//   - CalibrationService (B): Computes camera calibration parameters
//   - CalibrationPipeline (C): Orchestrates the calibration workflow
// ============================================================================

class MockImageProvider : public IImageProvider {
public:
    MOCK_METHOD(ImageFrame, captureFrame, (), (override));
};

class MockCalibrationSolver : public ICalibrationSolver {
public:
    MOCK_METHOD(CalibrationResult, calibrate, (const ImageFrame&), (override));
};

// ===========================================================================
// SCENARIO 1: DIRECT EXECUTION (No Recording)
// ===========================================================================
// Tests the baseline: CalibrationPipeline calls real services directly.
// No recording, no playback - just raw execution.
class DirectPipelineTest : public ::testing::Test {
protected:
    ImageCaptureService realCapture{1};
    CalibrationService realSolver;
    CalibrationPipeline pipeline{realCapture, realSolver};
};

// ===========================================================================
// SCENARIO 2: RECORDING WITH DECORATORS
// ===========================================================================
// Tests the decorator wrappers that intercept calls to A & B,
// delegate to the real implementations, and serialize results to JSON.
class RecordingTest : public ::testing::Test {
protected:
    ImageCaptureService realCapture{10};
    CalibrationService realSolver;
    RecordingSession session;
};

// ===========================================================================
// SCENARIO 3: PLAYBACK FROM RECORDING
// ===========================================================================
// Tests the player wrappers that replay previously recorded data.
// The real services are not called; instead, data comes from a JSON file.
class PlaybackTest : public ::testing::Test {
protected:
    ImageCaptureService realCapture{100};
    CalibrationService realSolver;
    PlaybackSession playbackSession;
    
    PlaybackTest() {
        // Step 1: Record a session with real components
        ImageCaptureService tempCapture{100};
        CalibrationService tempSolver;
        RecordingSession recordSession;
        RecordingImageProvider recordCapture(tempCapture, recordSession);
        RecordingCalibrationSolver recordSolver(tempSolver, recordSession);
        CalibrationPipeline recordPipeline(recordCapture, recordSolver);
        recordPipeline.processFrames(3);
        recordSession.saveToFile("playback_test.json");
        
        // Step 2: Load the recording for playback tests
        playbackSession = loadPlaybackSession("playback_test.json");
    }
};

// ===========================================================================
// SCENARIO 4: MOCK-BASED RECORDING TESTS
// ===========================================================================
// Tests the recorder wrappers using mock objects to verify that:
// 1. The wrapper delegates calls to the wrapped service
// 2. The wrapper correctly serializes the results
class MockBasedRecordingTest : public ::testing::Test {
protected:
    MockImageProvider mockProvider;
    MockCalibrationSolver mockSolver;
    RecordingSession session;
};

// ===========================================================================
// SCENARIO 1 TESTS: Direct Execution
// ===========================================================================

TEST_F(DirectPipelineTest, ProcessOneFrameWithoutRecording) {
    // Verify that direct execution produces valid calibration results
    auto result = pipeline.processOneFrame();
    EXPECT_EQ(result.frameId, 1);
    EXPECT_GT(result.focalLength, 0.0);
    EXPECT_GE(result.reprojectionError, 0.0);
    EXPECT_LT(result.reprojectionError, 1.0);
}

TEST_F(DirectPipelineTest, ProcessMultipleFramesWithoutRecording) {
    // Verify that direct execution can process multiple frames in sequence
    auto results = pipeline.processFrames(5);
    EXPECT_EQ(results.size(), 5);
    
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(results[i].frameId, i + 1);
        EXPECT_GT(results[i].focalLength, 0.0);
    }
}

// ===========================================================================
// SCENARIO 2 TESTS: Recording with Decorators
// ===========================================================================

TEST_F(RecordingTest, RecordingWrapperCapturesFrames) {
    // Test: Verify that recording wrappers capture frames and calibrations
    // to an in-memory JSON structure.
    RecordingImageProvider recordCapture(realCapture, session);
    RecordingCalibrationSolver recordSolver(realSolver, session);
    CalibrationPipeline pipeline(recordCapture, recordSolver);
    
    pipeline.processFrames(3);
    
    // Verify that 3 captures and 3 calibrations were recorded
    EXPECT_EQ(session.journal["captures"].size(), 3);
    EXPECT_EQ(session.journal["calibrations"].size(), 3);
}

TEST_F(RecordingTest, RecordingToJsonFile) {
    // Test: Verify that recording can be serialized to a JSON file
    // and the file contains the expected data.
    RecordingImageProvider recordCapture(realCapture, session);
    RecordingCalibrationSolver recordSolver(realSolver, session);
    CalibrationPipeline pipeline(recordCapture, recordSolver);
    
    pipeline.processFrames(2);
    session.saveToFile("test_recording.json");
    
    std::ifstream input("test_recording.json");
    ASSERT_TRUE(input.good());
    
    json journal;
    input >> journal;
    EXPECT_EQ(journal["captures"].size(), 2);
    EXPECT_EQ(journal["calibrations"].size(), 2);
}

// ===========================================================================
// SCENARIO 3 TESTS: Playback from Recording
// ===========================================================================

TEST_F(PlaybackTest, PlaybackImageProvider) {
    // Test: Verify that PlaybackImageProvider replays captured frames
    // from the previously recorded session.
    PlaybackImageProvider playbackCapture(playbackSession);
    
    for (int i = 0; i < 3; ++i) {
        auto frame = playbackCapture.captureFrame();
        EXPECT_EQ(frame.frameId, 100 + i);
    }
}

TEST_F(PlaybackTest, PlaybackCalibrationSolver) {
    // Test: Verify that PlaybackCalibrationSolver replays calibration
    // results from the previously recorded session.
    PlaybackCalibrationSolver playbackSolver(playbackSession);
    
    for (int i = 0; i < 3; ++i) {
        ImageFrame frame;
        frame.frameId = 100 + i;
        auto result = playbackSolver.calibrate(frame);
        EXPECT_EQ(result.frameId, 100 + i);
    }
}

TEST_F(PlaybackTest, ReplayMatchesOriginal) {
    // Test: Verify that replaying a recording produces identical results
    // to the original execution. This is a critical validation:
    // the playback must be bit-identical to ensure reproducibility.
    RecordingSession originalSession;
    RecordingImageProvider recordCapture(realCapture, originalSession);
    RecordingCalibrationSolver recordSolver(realSolver, originalSession);
    CalibrationPipeline recordPipeline(recordCapture, recordSolver);
    auto originalResults = recordPipeline.processFrames(2);
    
    PlaybackImageProvider playbackCapture(playbackSession);
    PlaybackCalibrationSolver playbackSolver(playbackSession);
    CalibrationPipeline playbackPipeline(playbackCapture, playbackSolver);
    auto replayResults = playbackPipeline.processFrames(2);
    
    EXPECT_EQ(replayResults.size(), originalResults.size());
    for (size_t i = 0; i < replayResults.size(); ++i) {
        EXPECT_EQ(replayResults[i].frameId, originalResults[i].frameId);
        EXPECT_DOUBLE_EQ(replayResults[i].focalLength, originalResults[i].focalLength);
        EXPECT_DOUBLE_EQ(replayResults[i].principalX, originalResults[i].principalX);
        EXPECT_DOUBLE_EQ(replayResults[i].principalY, originalResults[i].principalY);
    }
}

// ===========================================================================
// SCENARIO 4 TESTS: Mock-Based Recording Validation
// ===========================================================================

TEST_F(MockBasedRecordingTest, RecordingWrapperDelegatesAndRecords) {
    // Test: Verify that RecordingImageProvider delegates to the wrapped
    // service (mock) and correctly serializes the returned frame to JSON.
    ImageFrame expectedFrame;
    expectedFrame.frameId = 42;
    expectedFrame.scene = "test scene";
    expectedFrame.exposure = 1.5;
    
    EXPECT_CALL(mockProvider, captureFrame())
        .WillOnce(Return(expectedFrame));
    
    RecordingImageProvider recordCapture(mockProvider, session);
    auto capturedFrame = recordCapture.captureFrame();
    
    EXPECT_EQ(capturedFrame.frameId, 42);
    EXPECT_EQ(session.journal["captures"].size(), 1);
    EXPECT_EQ(session.journal["captures"][0]["frameId"], 42);
}

TEST_F(MockBasedRecordingTest, CalibrationRecordingWrapperDelegatesAndRecords) {
    // Test: Verify that RecordingCalibrationSolver delegates to the wrapped
    // service (mock) and correctly serializes the calibration result to JSON.
    ImageFrame inputFrame;
    inputFrame.frameId = 7;
    
    CalibrationResult expectedResult;
    expectedResult.frameId = 7;
    expectedResult.focalLength = 800.0;
    expectedResult.principalX = 640.5;
    expectedResult.principalY = 360.2;
    expectedResult.reprojectionError = 0.1;
    
    EXPECT_CALL(mockSolver, calibrate(_))
        .WillOnce(Return(expectedResult));
    
    RecordingCalibrationSolver recordSolver(mockSolver, session);
    auto result = recordSolver.calibrate(inputFrame);
    
    EXPECT_EQ(result.focalLength, 800.0);
    EXPECT_EQ(session.journal["calibrations"].size(), 1);
    EXPECT_EQ(session.journal["calibrations"][0]["focalLength"], 800.0);
}

// ===========================================================================
// SCENARIO 5: MOCK-BASED HAPPY PATH
// ===========================================================================
// Verifies pipeline correctness end-to-end with controlled inputs/outputs.
// Demonstrates: Field() argument matcher, Return() with full structs,
// and asserting every field of the returned CalibrationResult.
class MockPipelineTest : public ::testing::Test {
protected:
    MockImageProvider mockProvider;
    MockCalibrationSolver mockSolver;
};

TEST_F(MockPipelineTest, PipelinePassesCapturedFrameToSolverAndReturnsResult) {
    ImageFrame expectedFrame;
    expectedFrame.frameId = 5;
    expectedFrame.scene = "test road";
    expectedFrame.exposure = 1.75;

    CalibrationResult expectedResult;
    expectedResult.frameId = 5;
    expectedResult.focalLength = 820.0;
    expectedResult.principalX = 641.0;
    expectedResult.principalY = 361.0;
    expectedResult.reprojectionError = 0.05;

    EXPECT_CALL(mockProvider, captureFrame())
        .WillOnce(Return(expectedFrame));
    // Field() verifies calibrate receives the frame that captureFrame returned.
    EXPECT_CALL(mockSolver, calibrate(::testing::Field(&ImageFrame::frameId, 5)))
        .WillOnce(Return(expectedResult));

    CalibrationPipeline pipeline(mockProvider, mockSolver);
    auto result = pipeline.processOneFrame();

    EXPECT_EQ(result.frameId, expectedResult.frameId);
    EXPECT_DOUBLE_EQ(result.focalLength, expectedResult.focalLength);
    EXPECT_DOUBLE_EQ(result.principalX, expectedResult.principalX);
    EXPECT_DOUBLE_EQ(result.principalY, expectedResult.principalY);
    EXPECT_DOUBLE_EQ(result.reprojectionError, expectedResult.reprojectionError);
}

// ===========================================================================
// INTEGRATION TESTS: End-to-End Workflows
// ===========================================================================

TEST(CombinedScenarioTest, RecordThenPlayback) {
    // Integration test: Complete workflow
    //   1. Execute CalibrationPipeline with recording decorators
    //   2. Save recording to JSON file
    //   3. Load the JSON file
    //   4. Replay the recording using playback components
    //   5. Verify results match original execution
    //
    // This demonstrates the full cycle: real -> record -> serialize -> load -> playback
    
    ImageCaptureService capture;
    CalibrationService solver;
    
    // Phase 1: Record
    RecordingSession session;
    RecordingImageProvider recordCapture(capture, session);
    RecordingCalibrationSolver recordSolver(solver, session);
    CalibrationPipeline recordPipeline(recordCapture, recordSolver);
    auto originalResults = recordPipeline.processFrames(4);
    session.saveToFile("combined_scenario.json");
    
    // Phase 2: Playback
    auto playbackSession = loadPlaybackSession("combined_scenario.json");
    PlaybackImageProvider playbackCapture(playbackSession);
    PlaybackCalibrationSolver playbackSolver(playbackSession);
    CalibrationPipeline playbackPipeline(playbackCapture, playbackSolver);
    auto replayResults = playbackPipeline.processFrames(4);
    
    // Phase 3: Verify
    ASSERT_EQ(replayResults.size(), 4);
    for (size_t i = 0; i < replayResults.size(); ++i) {
        EXPECT_EQ(replayResults[i].frameId, originalResults[i].frameId);
        EXPECT_DOUBLE_EQ(replayResults[i].focalLength, originalResults[i].focalLength);
    }
}

// ===========================================================================
// ERROR HANDLING TESTS: Playback Edge Cases
// ===========================================================================

// GMock fixture for pipeline error tests.
// Demonstrates: Throw() action, Times(0) "never called" assertion,
// and exact call-count verification across two collaborators.
class MockPipelineErrorTest : public ::testing::Test {
protected:
    MockImageProvider mockProvider;
    MockCalibrationSolver mockSolver;
};

TEST_F(MockPipelineErrorTest, CalibrateNeverCalledWhenCaptureThrows) {
    // If captureFrame() throws, calibrate() must not be called at all.
    EXPECT_CALL(mockProvider, captureFrame())
        .WillOnce(::testing::Throw(std::runtime_error("camera failure")));
    EXPECT_CALL(mockSolver, calibrate(_))
        .Times(0);

    CalibrationPipeline pipeline(mockProvider, mockSolver);
    EXPECT_THROW(pipeline.processOneFrame(), std::runtime_error);
}

TEST_F(MockPipelineErrorTest, CaptureCalledOnceWhenCalibrateThrows) {
    // If calibrate() throws, captureFrame() was already called exactly once —
    // the pipeline does not retry or skip the capture step.
    ImageFrame frame;
    frame.frameId = 5;

    EXPECT_CALL(mockProvider, captureFrame())
        .Times(1)
        .WillOnce(Return(frame));
    EXPECT_CALL(mockSolver, calibrate(_))
        .WillOnce(::testing::Throw(std::runtime_error("solver failure")));

    CalibrationPipeline pipeline(mockProvider, mockSolver);
    EXPECT_THROW(pipeline.processOneFrame(), std::runtime_error);
}

TEST(PlaybackErrorTest, PlaybackWithoutRecordingThrowsOnCapture) {
    // Test: Verify that PlaybackImageProvider throws when there is no
    // recorded data to replay (e.g., empty session).
    PlaybackSession emptySession;
    PlaybackImageProvider playback(emptySession);
    
    EXPECT_THROW(playback.captureFrame(), std::out_of_range);
}

TEST(PlaybackErrorTest, PlaybackWithoutRecordingThrowsOnCalibrate) {
    // Test: Verify that PlaybackCalibrationSolver throws when there is no
    // recorded calibration result for a requested frame ID.
    PlaybackSession emptySession;
    PlaybackCalibrationSolver playback(emptySession);
    
    ImageFrame frame;
    frame.frameId = 999;
    EXPECT_THROW(playback.calibrate(frame), std::runtime_error);
}
