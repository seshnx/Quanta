#pragma once

#include "LevelDetector.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace SeshEQ {

/**
 * @brief Noise gate / expander
 * 
 * Features:
 * - Threshold, ratio (1:1 to infinity for hard gate)
 * - Attack, hold, release
 * - Range (maximum attenuation)
 */
class Gate {
public:
    Gate() = default;
    
    void prepare(double sampleRate, int samplesPerBlock);
    void reset();
    
    // Parameter setters
    void setThreshold(float dB);
    void setRatio(float ratio);  // 1:1 = expander, high ratio = gate
    void setAttack(float ms);
    void setHold(float ms);
    void setRelease(float ms);
    void setRange(float dB);  // Maximum attenuation (negative)
    void setEnabled(bool enabled);
    
    // Process audio
    void process(juce::AudioBuffer<float>& buffer);
    
    // Get current gain reduction for metering
    float getGainReduction() const { return gainReductionDb; }
    
    // Connect to APVTS
    void connectToParameters(juce::AudioProcessorValueTreeState& apvts);
    void updateFromParameters();
    
    bool isEnabled() const { return enabled; }
    
private:
    // Gate states
    enum class GateState {
        Closed,
        Attack,
        Open,
        Hold,
        Release
    };
    
    LevelDetector levelDetector;
    
    // Parameters
    float thresholdDb = -40.0f;
    float ratio = 10.0f;
    float holdMs = 50.0f;
    float rangeDb = -80.0f;
    bool enabled = false;
    
    // State
    GateState state = GateState::Closed;
    float gainReductionDb = 0.0f;
    float currentGain = 0.0f;  // Linear gain (0 to 1)
    int holdCounter = 0;
    int holdSamples = 0;
    
    double sampleRate = 44100.0;
    
    // Attack/release coefficients (separate from level detector)
    float attackCoef = 0.0f;
    float releaseCoef = 0.0f;
    float attackMs = 0.5f;
    float releaseMs = 100.0f;
    
    void updateCoefficients();
    
    // APVTS parameter pointers
    std::atomic<float>* thresholdParam = nullptr;
    std::atomic<float>* ratioParam = nullptr;
    std::atomic<float>* attackParam = nullptr;
    std::atomic<float>* holdParam = nullptr;
    std::atomic<float>* releaseParam = nullptr;
    std::atomic<float>* rangeParam = nullptr;
    std::atomic<float>* enabledParam = nullptr;
};

} // namespace SeshEQ
