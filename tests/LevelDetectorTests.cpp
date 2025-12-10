#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Direct include without JUCE dependencies for testing
#include "dsp/LevelDetector.h"

#include <cmath>
#include <vector>

using namespace SeshEQ;

class LevelDetectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        detector.prepare(sampleRate);
    }
    
    LevelDetector detector;
    static constexpr double sampleRate = 48000.0;
    static constexpr float tolerance = 0.01f;
};

//==============================================================================
// Basic functionality tests
//==============================================================================

TEST_F(LevelDetectorTest, ProcessesSampleWithoutCrash) {
    float output = detector.processSample(0.5f);
    EXPECT_FALSE(std::isnan(output));
    EXPECT_FALSE(std::isinf(output));
}

TEST_F(LevelDetectorTest, ResetClearsEnvelope) {
    // Process some samples to build up envelope
    for (int i = 0; i < 1000; ++i) {
        detector.processSample(1.0f);
    }
    
    EXPECT_GT(detector.getCurrentLevel(), 0.0f);
    
    detector.reset();
    
    EXPECT_EQ(detector.getCurrentLevel(), 0.0f);
}

//==============================================================================
// Peak detection tests
//==============================================================================

TEST_F(LevelDetectorTest, PeakDetectionTracksPositiveInput) {
    detector.setMode(DetectionMode::Peak);
    detector.setAttackTime(0.0f);  // Instant attack
    detector.setReleaseTime(1000.0f);  // Slow release
    
    // Feed constant signal
    for (int i = 0; i < 100; ++i) {
        detector.processSample(0.8f);
    }
    
    float level = detector.getCurrentLevel();
    EXPECT_NEAR(level, 0.8f, tolerance);
}

TEST_F(LevelDetectorTest, PeakDetectionTracksNegativeInput) {
    detector.setMode(DetectionMode::Peak);
    detector.setAttackTime(0.0f);
    detector.setReleaseTime(1000.0f);
    
    // Feed negative signal (should track absolute value)
    for (int i = 0; i < 100; ++i) {
        detector.processSample(-0.6f);
    }
    
    float level = detector.getCurrentLevel();
    EXPECT_NEAR(level, 0.6f, tolerance);
}

//==============================================================================
// Attack/Release tests
//==============================================================================

TEST_F(LevelDetectorTest, AttackTimeAffectsRiseTime) {
    detector.setMode(DetectionMode::Peak);
    detector.setReleaseTime(1000.0f);
    
    // Fast attack
    detector.setAttackTime(1.0f);
    detector.reset();
    
    for (int i = 0; i < 50; ++i) {
        detector.processSample(1.0f);
    }
    float fastAttackLevel = detector.getCurrentLevel();
    
    // Slow attack
    detector.setAttackTime(100.0f);
    detector.reset();
    
    for (int i = 0; i < 50; ++i) {
        detector.processSample(1.0f);
    }
    float slowAttackLevel = detector.getCurrentLevel();
    
    // Fast attack should reach higher level in same time
    EXPECT_GT(fastAttackLevel, slowAttackLevel);
}

TEST_F(LevelDetectorTest, ReleaseTimeAffectsFallTime) {
    detector.setMode(DetectionMode::Peak);
    detector.setAttackTime(0.0f);
    
    // Build up to 1.0
    for (int i = 0; i < 100; ++i) {
        detector.processSample(1.0f);
    }
    
    // Fast release
    detector.setReleaseTime(10.0f);
    for (int i = 0; i < 500; ++i) {
        detector.processSample(0.0f);
    }
    float fastReleaseLevel = detector.getCurrentLevel();
    
    // Reset and build up again
    detector.reset();
    for (int i = 0; i < 100; ++i) {
        detector.processSample(1.0f);
    }
    
    // Slow release
    detector.setReleaseTime(1000.0f);
    for (int i = 0; i < 500; ++i) {
        detector.processSample(0.0f);
    }
    float slowReleaseLevel = detector.getCurrentLevel();
    
    // Slow release should maintain higher level
    EXPECT_GT(slowReleaseLevel, fastReleaseLevel);
}

//==============================================================================
// RMS detection tests
//==============================================================================

TEST_F(LevelDetectorTest, RMSDetectionGivesLowerValueForSameSignal) {
    detector.setAttackTime(1.0f);
    detector.setReleaseTime(100.0f);
    
    // Generate a test signal
    std::vector<float> signal(1000);
    for (size_t i = 0; i < signal.size(); ++i) {
        signal[i] = std::sin(2.0f * 3.14159f * 1000.0f * static_cast<float>(i) / static_cast<float>(sampleRate));
    }
    
    // Peak detection
    detector.setMode(DetectionMode::Peak);
    detector.reset();
    for (float sample : signal) {
        detector.processSample(sample);
    }
    float peakLevel = detector.getCurrentLevel();
    
    // RMS detection
    detector.setMode(DetectionMode::RMS);
    detector.reset();
    for (float sample : signal) {
        detector.processSample(sample);
    }
    float rmsLevel = detector.getCurrentLevel();
    
    // For sine wave, RMS should be about 0.707 of peak
    // But due to attack/release, we just check RMS < Peak
    EXPECT_LT(rmsLevel, peakLevel);
}

//==============================================================================
// Stereo processing tests
//==============================================================================

TEST_F(LevelDetectorTest, StereoProcessingTakesMaximum) {
    detector.setMode(DetectionMode::Peak);
    detector.setAttackTime(0.0f);
    detector.setReleaseTime(1000.0f);
    
    // Left channel higher
    for (int i = 0; i < 100; ++i) {
        detector.processStereo(0.8f, 0.2f);
    }
    float level1 = detector.getCurrentLevel();
    EXPECT_NEAR(level1, 0.8f, tolerance);
    
    // Right channel higher
    detector.reset();
    for (int i = 0; i < 100; ++i) {
        detector.processStereo(0.3f, 0.9f);
    }
    float level2 = detector.getCurrentLevel();
    EXPECT_NEAR(level2, 0.9f, tolerance);
}

//==============================================================================
// dB conversion tests
//==============================================================================

TEST_F(LevelDetectorTest, DbConversionIsCorrect) {
    detector.setMode(DetectionMode::Peak);
    detector.setAttackTime(0.0f);
    detector.setReleaseTime(10000.0f);
    
    // Feed 0.5 signal (should be -6 dB)
    for (int i = 0; i < 100; ++i) {
        detector.processSample(0.5f);
    }
    
    float levelDb = detector.getCurrentLevelDb();
    EXPECT_NEAR(levelDb, -6.0f, 0.5f);
}

TEST_F(LevelDetectorTest, DbUtilsLinearToDb) {
    EXPECT_NEAR(dBUtils::linearToDb(1.0f), 0.0f, 0.01f);
    EXPECT_NEAR(dBUtils::linearToDb(0.5f), -6.02f, 0.1f);
    EXPECT_NEAR(dBUtils::linearToDb(0.1f), -20.0f, 0.1f);
    EXPECT_NEAR(dBUtils::linearToDb(2.0f), 6.02f, 0.1f);
}

TEST_F(LevelDetectorTest, DbUtilsDbToLinear) {
    EXPECT_NEAR(dBUtils::dbToLinear(0.0f), 1.0f, 0.01f);
    EXPECT_NEAR(dBUtils::dbToLinear(-6.0f), 0.5f, 0.02f);
    EXPECT_NEAR(dBUtils::dbToLinear(-20.0f), 0.1f, 0.01f);
    EXPECT_NEAR(dBUtils::dbToLinear(6.0f), 2.0f, 0.1f);
}

TEST_F(LevelDetectorTest, DbConversionRoundTrip) {
    float original = 0.75f;
    float db = dBUtils::linearToDb(original);
    float back = dBUtils::dbToLinear(db);
    EXPECT_NEAR(back, original, 0.001f);
}

//==============================================================================
// Edge case tests
//==============================================================================

TEST_F(LevelDetectorTest, HandlesZeroInput) {
    detector.setMode(DetectionMode::Peak);
    detector.setAttackTime(0.0f);
    
    float output = detector.processSample(0.0f);
    EXPECT_GE(output, 0.0f);
    EXPECT_FALSE(std::isnan(output));
}

TEST_F(LevelDetectorTest, HandlesSilence) {
    detector.setMode(DetectionMode::Peak);
    detector.setAttackTime(0.0f);
    detector.setReleaseTime(10.0f);
    
    // Process silence
    for (int i = 0; i < 10000; ++i) {
        detector.processSample(0.0f);
    }
    
    float level = detector.getCurrentLevel();
    EXPECT_LT(level, 0.001f);
    EXPECT_GE(level, 0.0f);
}

TEST_F(LevelDetectorTest, HandlesVerySmallInput) {
    detector.setMode(DetectionMode::Peak);
    detector.setAttackTime(0.0f);
    
    float output = detector.processSample(1e-10f);
    EXPECT_FALSE(std::isnan(output));
    EXPECT_GE(output, 0.0f);
}

TEST_F(LevelDetectorTest, DbConversionHandlesVerySmallValues) {
    float db = dBUtils::linearToDb(1e-20f);
    EXPECT_FALSE(std::isnan(db));
    EXPECT_FALSE(std::isinf(db));
    EXPECT_LT(db, -100.0f);
}

//==============================================================================
// Prepare tests
//==============================================================================

TEST_F(LevelDetectorTest, PrepareWithDifferentSampleRates) {
    detector.prepare(44100.0);
    float output1 = detector.processSample(0.5f);
    EXPECT_FALSE(std::isnan(output1));
    
    detector.prepare(96000.0);
    float output2 = detector.processSample(0.5f);
    EXPECT_FALSE(std::isnan(output2));
    
    detector.prepare(192000.0);
    float output3 = detector.processSample(0.5f);
    EXPECT_FALSE(std::isnan(output3));
}
