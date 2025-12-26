#pragma once

#include "Compressor.h"
#include "utils/Parameters.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>

namespace SeshEQ {

/**
 * @brief Per-band dynamics processor (Compressor/Expander)
 * 
 * Each EQ band can have its own dynamics processing with:
 * - Threshold, Ratio, Attack, Release, Knee
 * - Independent gain reduction metering
 */
class BandDynamics {
public:
    BandDynamics() = default;
    
    void prepare(double sampleRate, int samplesPerBlock);
    void reset();
    
    // Parameter setters
    void setThreshold(float dB);
    void setRatio(float ratio);
    void setAttack(float ms);
    void setRelease(float ms);
    void setKnee(float dB);
    void setEnabled(bool enabled);
    
    // Process audio for a single band
    void process(juce::AudioBuffer<float>& buffer);
    
    // Get current gain reduction for metering
    float getGainReduction() const { return gainReductionDb.load(); }
    
    // Connect to APVTS
    void connectToParameters(juce::AudioProcessorValueTreeState& apvts, int bandIndex);
    void updateFromParameters();
    
    bool isEnabled() const { return enabled; }
    
private:
    Compressor compressor;
    
    // Parameters
    float thresholdDb = -12.0f;
    float ratio = 2.0f;
    float attackMs = 10.0f;
    float releaseMs = 100.0f;
    float kneeDb = 3.0f;
    bool enabled = false;
    
    // State
    std::atomic<float> gainReductionDb { 0.0f };
    double sampleRate = 44100.0;
    
    // APVTS parameter pointers
    std::atomic<float>* thresholdParam = nullptr;
    std::atomic<float>* ratioParam = nullptr;
    std::atomic<float>* attackParam = nullptr;
    std::atomic<float>* releaseParam = nullptr;
    std::atomic<float>* kneeParam = nullptr;
    std::atomic<float>* enabledParam = nullptr;
};

} // namespace SeshEQ

