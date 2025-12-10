#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Direct include without JUCE dependencies for testing
#include "dsp/BiquadFilter.h"

#include <cmath>
#include <vector>

using namespace SeshEQ;

class BiquadFilterTest : public ::testing::Test {
protected:
    void SetUp() override {
        filter.prepare(sampleRate);
    }
    
    BiquadFilter filter;
    static constexpr double sampleRate = 48000.0;
    static constexpr float tolerance = 0.01f; // 1% tolerance
};

//==============================================================================
// Basic functionality tests
//==============================================================================

TEST_F(BiquadFilterTest, ProcessesSampleWithoutCrash) {
    filter.setParameters(FilterType::Peak, 1000.0f, 0.707f, 0.0f);
    
    float output = filter.processSample(1.0f);
    EXPECT_FALSE(std::isnan(output));
    EXPECT_FALSE(std::isinf(output));
}

TEST_F(BiquadFilterTest, ResetClearsState) {
    filter.setParameters(FilterType::LowPass, 1000.0f, 0.707f, 0.0f);
    
    // Process some samples
    for (int i = 0; i < 100; ++i) {
        filter.processSample(1.0f);
    }
    
    filter.reset();
    
    // After reset, output should be same as fresh filter
    BiquadFilter freshFilter;
    freshFilter.prepare(sampleRate);
    freshFilter.setParameters(FilterType::LowPass, 1000.0f, 0.707f, 0.0f);
    
    float resetOutput = filter.processSample(0.5f);
    float freshOutput = freshFilter.processSample(0.5f);
    
    EXPECT_NEAR(resetOutput, freshOutput, tolerance);
}

//==============================================================================
// Low Pass Filter tests
//==============================================================================

TEST_F(BiquadFilterTest, LowPassAttenuatesHighFrequencies) {
    filter.setParameters(FilterType::LowPass, 1000.0f, 0.707f, 0.0f);
    
    // Magnitude at DC should be ~1 (0 dB)
    float magDC = filter.getMagnitudeAtFrequency(10.0f);
    EXPECT_NEAR(magDC, 1.0f, tolerance);
    
    // Magnitude at cutoff should be ~0.707 (-3 dB)
    float magCutoff = filter.getMagnitudeAtFrequency(1000.0f);
    EXPECT_NEAR(magCutoff, 0.707f, 0.1f);
    
    // Magnitude well above cutoff should be significantly attenuated
    float magHigh = filter.getMagnitudeAtFrequency(10000.0f);
    EXPECT_LT(magHigh, 0.2f);
}

TEST_F(BiquadFilterTest, LowPassCutoffFrequencyAffectsResponse) {
    filter.setParameters(FilterType::LowPass, 2000.0f, 0.707f, 0.0f);
    
    float mag1k = filter.getMagnitudeAtFrequency(1000.0f);
    float mag2k = filter.getMagnitudeAtFrequency(2000.0f);
    float mag4k = filter.getMagnitudeAtFrequency(4000.0f);
    
    // Below cutoff should be louder than at cutoff
    EXPECT_GT(mag1k, mag2k);
    // At cutoff should be louder than above cutoff
    EXPECT_GT(mag2k, mag4k);
}

//==============================================================================
// High Pass Filter tests
//==============================================================================

TEST_F(BiquadFilterTest, HighPassAttenuatesLowFrequencies) {
    filter.setParameters(FilterType::HighPass, 1000.0f, 0.707f, 0.0f);
    
    // Magnitude at Nyquist should be ~1 (0 dB)
    float magHigh = filter.getMagnitudeAtFrequency(20000.0f);
    EXPECT_NEAR(magHigh, 1.0f, tolerance);
    
    // Magnitude at cutoff should be ~0.707 (-3 dB)
    float magCutoff = filter.getMagnitudeAtFrequency(1000.0f);
    EXPECT_NEAR(magCutoff, 0.707f, 0.1f);
    
    // Magnitude well below cutoff should be significantly attenuated
    float magLow = filter.getMagnitudeAtFrequency(100.0f);
    EXPECT_LT(magLow, 0.2f);
}

//==============================================================================
// Peak/Bell Filter tests
//==============================================================================

TEST_F(BiquadFilterTest, PeakFilterBoostsAtCenterFrequency) {
    filter.setParameters(FilterType::Peak, 1000.0f, 1.0f, 6.0f); // +6 dB boost
    
    float magCenter = filter.getMagnitudeAtFrequency(1000.0f);
    float magAway = filter.getMagnitudeAtFrequency(100.0f);
    
    // Center should be boosted
    EXPECT_GT(magCenter, 1.5f); // ~6 dB = 2x, allowing some Q spread
    
    // Away from center should be closer to unity
    EXPECT_NEAR(magAway, 1.0f, 0.2f);
}

TEST_F(BiquadFilterTest, PeakFilterCutsAtCenterFrequency) {
    filter.setParameters(FilterType::Peak, 1000.0f, 1.0f, -6.0f); // -6 dB cut
    
    float magCenter = filter.getMagnitudeAtFrequency(1000.0f);
    float magAway = filter.getMagnitudeAtFrequency(100.0f);
    
    // Center should be cut
    EXPECT_LT(magCenter, 0.6f); // ~-6 dB = 0.5x, allowing some Q spread
    
    // Away from center should be closer to unity
    EXPECT_NEAR(magAway, 1.0f, 0.2f);
}

TEST_F(BiquadFilterTest, PeakFilterQAffectsBandwidth) {
    // Wide Q
    filter.setParameters(FilterType::Peak, 1000.0f, 0.5f, 6.0f);
    float magWide500 = filter.getMagnitudeAtFrequency(500.0f);
    
    // Narrow Q
    filter.setParameters(FilterType::Peak, 1000.0f, 4.0f, 6.0f);
    float magNarrow500 = filter.getMagnitudeAtFrequency(500.0f);
    
    // Wide Q should have more effect at 500Hz than narrow Q
    EXPECT_GT(magWide500, magNarrow500);
}

//==============================================================================
// Shelf Filter tests
//==============================================================================

TEST_F(BiquadFilterTest, LowShelfBoostsBelowFrequency) {
    filter.setParameters(FilterType::LowShelf, 1000.0f, 0.707f, 6.0f);
    
    float magLow = filter.getMagnitudeAtFrequency(100.0f);
    float magHigh = filter.getMagnitudeAtFrequency(10000.0f);
    
    // Below shelf should be boosted
    EXPECT_GT(magLow, 1.5f);
    
    // Above shelf should be near unity
    EXPECT_NEAR(magHigh, 1.0f, 0.2f);
}

TEST_F(BiquadFilterTest, HighShelfBoostsAboveFrequency) {
    filter.setParameters(FilterType::HighShelf, 1000.0f, 0.707f, 6.0f);
    
    float magLow = filter.getMagnitudeAtFrequency(100.0f);
    float magHigh = filter.getMagnitudeAtFrequency(10000.0f);
    
    // Above shelf should be boosted
    EXPECT_GT(magHigh, 1.5f);
    
    // Below shelf should be near unity
    EXPECT_NEAR(magLow, 1.0f, 0.2f);
}

//==============================================================================
// Notch Filter tests
//==============================================================================

TEST_F(BiquadFilterTest, NotchFilterAttenuatesAtCenterFrequency) {
    filter.setParameters(FilterType::Notch, 1000.0f, 10.0f, 0.0f);
    
    float magCenter = filter.getMagnitudeAtFrequency(1000.0f);
    float magAway = filter.getMagnitudeAtFrequency(100.0f);
    
    // At notch frequency should be heavily attenuated
    EXPECT_LT(magCenter, 0.1f);
    
    // Away from notch should be near unity
    EXPECT_NEAR(magAway, 1.0f, 0.1f);
}

//==============================================================================
// Band Pass Filter tests
//==============================================================================

TEST_F(BiquadFilterTest, BandPassPassesCenterFrequency) {
    filter.setParameters(FilterType::BandPass, 1000.0f, 1.0f, 0.0f);
    
    float magCenter = filter.getMagnitudeAtFrequency(1000.0f);
    float magLow = filter.getMagnitudeAtFrequency(100.0f);
    float magHigh = filter.getMagnitudeAtFrequency(10000.0f);
    
    // Center should have highest magnitude
    EXPECT_GT(magCenter, magLow);
    EXPECT_GT(magCenter, magHigh);
}

//==============================================================================
// Block processing tests
//==============================================================================

TEST_F(BiquadFilterTest, BlockProcessingMatchesSampleBySample) {
    filter.setParameters(FilterType::LowPass, 1000.0f, 0.707f, 0.0f);
    
    std::vector<float> blockData = { 0.5f, -0.3f, 0.8f, -0.1f, 0.2f };
    std::vector<float> sampleData = blockData;
    
    // Process block
    filter.processBlock(blockData.data(), static_cast<int>(blockData.size()));
    
    // Process sample by sample with fresh filter
    filter.reset();
    for (size_t i = 0; i < sampleData.size(); ++i) {
        sampleData[i] = filter.processSample(sampleData[i]);
    }
    
    // Results should match
    for (size_t i = 0; i < blockData.size(); ++i) {
        EXPECT_NEAR(blockData[i], sampleData[i], 1e-6f);
    }
}

//==============================================================================
// Stereo filter tests
//==============================================================================

TEST_F(BiquadFilterTest, StereoFilterProcessesBothChannels) {
    StereoBiquadFilter stereoFilter;
    stereoFilter.prepare(sampleRate);
    stereoFilter.setParameters(FilterType::LowPass, 1000.0f, 0.707f, 0.0f);
    
    float left = 0.5f;
    float right = -0.3f;
    
    stereoFilter.processStereo(left, right);
    
    EXPECT_FALSE(std::isnan(left));
    EXPECT_FALSE(std::isnan(right));
    EXPECT_FALSE(std::isinf(left));
    EXPECT_FALSE(std::isinf(right));
}

//==============================================================================
// Edge case tests
//==============================================================================

TEST_F(BiquadFilterTest, HandlesExtremeFrequencies) {
    // Very low frequency
    filter.setParameters(FilterType::Peak, 20.0f, 0.707f, 6.0f);
    float output1 = filter.processSample(0.5f);
    EXPECT_FALSE(std::isnan(output1));
    
    // Very high frequency (near Nyquist)
    filter.setParameters(FilterType::Peak, 20000.0f, 0.707f, 6.0f);
    float output2 = filter.processSample(0.5f);
    EXPECT_FALSE(std::isnan(output2));
}

TEST_F(BiquadFilterTest, HandlesExtremeQ) {
    // Very low Q
    filter.setParameters(FilterType::Peak, 1000.0f, 0.1f, 6.0f);
    float output1 = filter.processSample(0.5f);
    EXPECT_FALSE(std::isnan(output1));
    
    // Very high Q
    filter.setParameters(FilterType::Peak, 1000.0f, 18.0f, 6.0f);
    float output2 = filter.processSample(0.5f);
    EXPECT_FALSE(std::isnan(output2));
}

TEST_F(BiquadFilterTest, HandlesExtremeGain) {
    // Maximum boost
    filter.setParameters(FilterType::Peak, 1000.0f, 0.707f, 24.0f);
    float output1 = filter.processSample(0.5f);
    EXPECT_FALSE(std::isnan(output1));
    
    // Maximum cut
    filter.setParameters(FilterType::Peak, 1000.0f, 0.707f, -24.0f);
    float output2 = filter.processSample(0.5f);
    EXPECT_FALSE(std::isnan(output2));
}

TEST_F(BiquadFilterTest, HandlesZeroGainPeak) {
    filter.setParameters(FilterType::Peak, 1000.0f, 0.707f, 0.0f);
    
    // With zero gain, peak filter should be unity
    float mag = filter.getMagnitudeAtFrequency(1000.0f);
    EXPECT_NEAR(mag, 1.0f, tolerance);
}

//==============================================================================
// Coefficient access tests
//==============================================================================

TEST_F(BiquadFilterTest, CoefficientsAreValid) {
    filter.setParameters(FilterType::LowPass, 1000.0f, 0.707f, 0.0f);
    
    auto coefs = filter.getCoefficients();
    
    // Coefficients should not be NaN or infinity
    EXPECT_FALSE(std::isnan(coefs.b0));
    EXPECT_FALSE(std::isnan(coefs.b1));
    EXPECT_FALSE(std::isnan(coefs.b2));
    EXPECT_FALSE(std::isnan(coefs.a1));
    EXPECT_FALSE(std::isnan(coefs.a2));
    
    EXPECT_FALSE(std::isinf(coefs.b0));
    EXPECT_FALSE(std::isinf(coefs.b1));
    EXPECT_FALSE(std::isinf(coefs.b2));
    EXPECT_FALSE(std::isinf(coefs.a1));
    EXPECT_FALSE(std::isinf(coefs.a2));
}
