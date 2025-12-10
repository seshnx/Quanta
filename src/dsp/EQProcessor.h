#pragma once

#include "BiquadFilter.h"
#include "utils/Parameters.h"
#include "utils/SmoothValue.h"
#include <array>
#include <juce_audio_processors/juce_audio_processors.h>

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
};

} // namespace SeshEQ
