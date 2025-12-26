#pragma once

#include "BiquadFilter.h"
#include "LinearPhaseEQ.h"
#include "DynamicEQ.h"
#include "BandDynamics.h"
#include "utils/MidSideProcessor.h"
#include "utils/Parameters.h"
#include "utils/SmoothValue.h"
#include <array>
#include <memory>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>

namespace SeshEQ {

/**
 * @brief Multi-band parametric EQ processor
 * 
 * Provides 8 bands of parametric EQ with various filter types.
 * Each band can be individually enabled/disabled.
 */
class EQProcessor {
public:
    EQProcessor() = default;
    
    /**
     * @brief Prepare the EQ for processing
     */
    void prepare(double sampleRate, int samplesPerBlock);
    
    /**
     * @brief Reset all filter states
     */
    void reset();
    
    /**
     * @brief Process a stereo audio buffer
     */
    void process(juce::AudioBuffer<float>& buffer);
    
    /**
     * @brief Set Mid/Side processing mode
     * @param enabled If true, process Mid and Side channels separately
     */
    void setMidSideMode(bool enabled);
    
    /**
     * @brief Set Linear Phase mode
     * @param enabled If true, use linear phase EQ (higher latency, zero phase distortion)
     */
    void setLinearPhaseMode(bool enabled);
    
    /**
     * @brief Get latency in samples (for linear phase mode)
     */
    int getLatency() const;
    
    /**
     * @brief Update parameters for a specific band
     */
    void setBandParameters(int bandIndex, FilterType type, float freq, float q, float gain, bool enabled);
    
    /**
     * @brief Enable or disable a band
     */
    void setBandEnabled(int bandIndex, bool enabled);
    
    /**
     * @brief Get band parameters
     */
    struct BandParams {
        FilterType type;
        float frequency;
        float q;
        float gain;
        bool enabled;
    };
    
    BandParams getBandParameters(int bandIndex) const;
    
    /**
     * @brief Get the combined magnitude response at a frequency
     * @param frequency Frequency in Hz
     * @return Combined magnitude in linear scale
     */
    float getMagnitudeAtFrequency(float frequency) const;
    
    /**
     * @brief Get magnitude response for a single band
     */
    float getBandMagnitudeAtFrequency(int bandIndex, float frequency) const;
    
    /**
     * @brief Connect to APVTS for parameter automation
     */
    void connectToParameters(juce::AudioProcessorValueTreeState& apvts);
    
    /**
     * @brief Update from APVTS (call before processing)
     */
    void updateFromParameters();
    
    /**
     * @brief Set Dynamic EQ mode
     * @param enabled If true, enable dynamic EQ processing
     */
    void setDynamicEQMode(bool enabled);
    
    /**
     * @brief Get gain reduction for a specific band (for metering)
     */
    float getBandGainReduction(int bandIndex) const;
    
private:
    /**
     * @brief Standard EQ processing (used by both normal and M/S modes)
     */
    void processStandard(juce::AudioBuffer<float>& buffer);
    
private:
    static constexpr int numBands = Constants::numEQBands;
    
    // Stereo filters for each band
    std::array<StereoBiquadFilter, numBands> filters;
    
    // Band enabled states
    std::array<bool, numBands> bandEnabled;
    
    // Parameter smoothers for each band
    struct BandSmoothers {
        SmoothValue<float> frequency { 1000.0f };
        SmoothValue<float> q { 0.707f };
        SmoothValue<float> gain { 0.0f };
    };
    std::array<BandSmoothers, numBands> smoothers;
    
    // APVTS parameter pointers
    struct BandParamPtrs {
        std::atomic<float>* frequency = nullptr;
        std::atomic<float>* q = nullptr;
        std::atomic<float>* gain = nullptr;
        std::atomic<float>* type = nullptr;
        std::atomic<float>* enabled = nullptr;
    };
    std::array<BandParamPtrs, numBands> paramPtrs;
    
    double currentSampleRate = 44100.0;
    bool prepared = false;
    
    // Thread safety for UI access
    mutable juce::CriticalSection lock;
    
    // Advanced features
    bool midSideMode = false;
    bool linearPhaseMode = false;
    bool dynamicEQMode = false;
    
    // Mid/Side buffers
    juce::AudioBuffer<float> midBuffer;
    juce::AudioBuffer<float> sideBuffer;
    
    // Linear Phase EQ (for zero phase distortion)
    std::unique_ptr<LinearPhaseEQ> linearPhaseEQ;
    
    // Dynamic EQ (for sidechain-responsive bands)
    std::unique_ptr<DynamicEQProcessor> dynamicEQ;
    
    // Per-band dynamics processors
    std::array<BandDynamics, numBands> bandDynamics;
};

} // namespace SeshEQ
