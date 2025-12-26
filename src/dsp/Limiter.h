#pragma once

#include "LevelDetector.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <memory>

namespace SeshEQ {

/**
 * @brief True Peak Limiter with oversampling
 * 
 * Features:
 * - True Peak detection (inter-sample peak detection)
 * - Up to 8x oversampling for accurate peak detection
 * - Adjustable threshold and ceiling
 * - Auto release
 * - Gain reduction and True Peak metering
 */
class Limiter {
public:
    Limiter() = default;
    ~Limiter();
    
    void prepare(double sampleRate, int samplesPerBlock);
    void reset();
    
    // Parameter setters
    void setThreshold(float dB);
    void setCeiling(float dB);
    void setRelease(float ms);
    void setEnabled(bool enabled);
    void setOversamplingFactor(int factor); // 1, 2, 4, or 8
    
    // Process audio
    void process(juce::AudioBuffer<float>& buffer);
    
    // Get current gain reduction for metering
    float getGainReduction() const { return gainReductionDb.load(); }
    
    // Get True Peak level for metering
    float getTruePeak() const { return truePeakDb.load(); }
    
    // Connect to APVTS
    void connectToParameters(juce::AudioProcessorValueTreeState& apvts);
    void updateFromParameters();
    
    bool isEnabled() const { return enabled; }
    
    // Get latency in samples (for oversampling)
    int getLatency() const;
    
private:
    // True Peak detection using oversampling
    float detectTruePeak(const float* samples, int numSamples, int numChannels);
    
    // Parameters
    float thresholdDb = -3.0f;
    float ceilingDb = -0.3f;
    float releaseMs = 100.0f;
    bool enabled = false;
    int oversamplingFactor = 4; // Default 4x oversampling
    
    // State
    std::atomic<float> gainReductionDb { 0.0f };
    std::atomic<float> truePeakDb { -100.0f };
    float currentGain = 1.0f;
    double sampleRate = 44100.0;
    int maxBlockSize = 512;
    
    // Release coefficient
    float releaseCoef = 0.0f;
    
    // Oversampling
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    
    void updateCoefficients();
    void updateOversampling();
    
    // APVTS parameter pointers
    std::atomic<float>* thresholdParam = nullptr;
    std::atomic<float>* ceilingParam = nullptr;
    std::atomic<float>* releaseParam = nullptr;
    std::atomic<float>* enabledParam = nullptr;
};

} // namespace SeshEQ
