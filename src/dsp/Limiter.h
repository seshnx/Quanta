#pragma once

#include "LevelDetector.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace SeshEQ {

/**
 * @brief Simple look-ahead limiter
 * 
 * Brickwall limiter with:
 * - Adjustable ceiling
 * - Auto release
 * - Gain reduction metering
 */
class Limiter {
public:
    Limiter() = default;
    
    void prepare(double sampleRate, int samplesPerBlock);
    void reset();
    
    // Parameter setters
    void setCeiling(float dB);
    void setRelease(float ms);
    void setEnabled(bool enabled);
    
    // Process audio
    void process(juce::AudioBuffer<float>& buffer);
    
    // Get current gain reduction for metering
    float getGainReduction() const { return gainReductionDb; }
    
    // Connect to APVTS
    void connectToParameters(juce::AudioProcessorValueTreeState& apvts);
    void updateFromParameters();
    
    bool isEnabled() const { return enabled; }
    
    // Get latency in samples (for look-ahead)
    int getLatency() const { return 0; } // No look-ahead in simple version
    
private:
    // Parameters
    float ceilingDb = -0.3f;
    float releaseMs = 100.0f;
    bool enabled = false;
    
    // State
    float gainReductionDb = 0.0f;
    float currentGain = 1.0f;
    double sampleRate = 44100.0;
    
    // Release coefficient
    float releaseCoef = 0.0f;
    
    void updateCoefficients();
    
    // APVTS parameter pointers
    std::atomic<float>* ceilingParam = nullptr;
    std::atomic<float>* releaseParam = nullptr;
    std::atomic<float>* enabledParam = nullptr;
};

} // namespace SeshEQ
