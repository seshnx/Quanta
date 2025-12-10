#pragma once

#include <cmath>
#include <algorithm>

namespace SeshEQ {

/**
 * @brief Level detection mode
 */
enum class DetectionMode {
    Peak,       // Instantaneous peak level
    RMS,        // Root Mean Square level
    TruePeak    // Oversampled peak (simplified)
};

/**
 * @brief Envelope follower with attack/release ballistics
 * 
 * Used for compressor/gate level detection. Operates in linear domain.
 */
class LevelDetector {
public:
    LevelDetector() = default;
    
    /**
     * @brief Prepare detector with sample rate
     */
    void prepare(double sampleRate);
    
    /**
     * @brief Reset envelope state
     */
    void reset();
    
    /**
     * @brief Set attack time in milliseconds
     */
    void setAttackTime(float attackMs);
    
    /**
     * @brief Set release time in milliseconds
     */
    void setReleaseTime(float releaseMs);
    
    /**
     * @brief Set detection mode
     */
    void setMode(DetectionMode mode);
    
    /**
     * @brief Process a single sample and return envelope level
     * @param input Input sample (linear)
     * @return Envelope level (linear)
     */
    float processSample(float input);
    
    /**
     * @brief Process stereo input (returns max of both channels)
     */
    float processStereo(float left, float right);
    
    /**
     * @brief Get current envelope level without processing
     */
    float getCurrentLevel() const { return envelope; }
    
    /**
     * @brief Get current level in dB
     */
    float getCurrentLevelDb() const;
    
private:
    void updateCoefficients();
    
    double sampleRate = 44100.0;
    float attackMs = 10.0f;
    float releaseMs = 100.0f;
    
    float attackCoef = 0.0f;
    float releaseCoef = 0.0f;
    
    float envelope = 0.0f;
    
    DetectionMode mode = DetectionMode::Peak;
    
    // RMS calculation
    float rmsSum = 0.0f;
    int rmsWindowSamples = 0;
    int rmsSampleCount = 0;
    static constexpr float rmsWindowMs = 50.0f;
};

/**
 * @brief Decibel conversion utilities
 */
namespace dBUtils {
    inline float linearToDb(float linear) {
        return 20.0f * std::log10(std::max(linear, 1e-10f));
    }
    
    inline float dbToLinear(float dB) {
        return std::pow(10.0f, dB / 20.0f);
    }
    
    inline double linearToDb(double linear) {
        return 20.0 * std::log10(std::max(linear, 1e-10));
    }
    
    inline double dbToLinear(double dB) {
        return std::pow(10.0, dB / 20.0);
    }
}

} // namespace SeshEQ
