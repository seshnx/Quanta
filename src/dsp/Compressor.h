#pragma once

#include "LevelDetector.h"
#include "utils/SmoothValue.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace SeshEQ {

/**
 * @brief Feed-forward compressor with soft knee
 * 
 * Features:
 * - Threshold, ratio, attack, release, knee
 * - Makeup gain
 * - Parallel mix (dry/wet)
 * - Peak/RMS detection
 * - Sidechain support (future)
 */
class Compressor {
public:
    Compressor() = default;
    
    void prepare(double sampleRate, int samplesPerBlock);
    void reset();
    
    // Parameter setters
    void setThreshold(float dB);
    void setRatio(float ratio);
    void setAttack(float ms);
    void setRelease(float ms);
    void setKnee(float dB);
    void setMakeupGain(float dB);
    void setMix(float percent);  // 0-100
    void setDetectionMode(DetectionMode mode);
    void setEnabled(bool enabled);
    
    // Process audio
    void process(juce::AudioBuffer<float>& buffer);
    
    // Get current gain reduction in dB (for metering)
    float getGainReduction() const { return gainReductionDb; }
    
    // Connect to APVTS
    void connectToParameters(juce::AudioProcessorValueTreeState& apvts);
    void updateFromParameters();
    
    bool isEnabled() const { return enabled; }
    
private:
    // Calculate gain reduction for a given input level (dB)
    float computeGain(float inputDb) const;
    
    LevelDetector levelDetector;
    
    // Parameters
    float thresholdDb = -18.0f;
    float ratio = 4.0f;
    float kneeDb = 6.0f;
    float makeupGainDb = 0.0f;
    float mix = 1.0f;  // 0-1
    bool enabled = false;
    
    // Smoothed parameters
    SmoothGain<float> makeupGain;
    
    // State
    float gainReductionDb = 0.0f;
    double sampleRate = 44100.0;
    
    // APVTS parameter pointers
    std::atomic<float>* thresholdParam = nullptr;
    std::atomic<float>* ratioParam = nullptr;
    std::atomic<float>* attackParam = nullptr;
    std::atomic<float>* releaseParam = nullptr;
    std::atomic<float>* kneeParam = nullptr;
    std::atomic<float>* makeupParam = nullptr;
    std::atomic<float>* mixParam = nullptr;
    std::atomic<float>* enabledParam = nullptr;
};

} // namespace SeshEQ
