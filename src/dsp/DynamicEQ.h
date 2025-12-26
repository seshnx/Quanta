#pragma once

#include "BiquadFilter.h"
#include "LevelDetector.h"
#include "utils/SmoothValue.h"
#include <array>
#include <juce_audio_processors/juce_audio_processors.h>

namespace SeshEQ {

/**
 * @brief Dynamic EQ band - EQ that responds to audio level
 * 
 * Each band can have its gain modulated by a sidechain signal.
 * Useful for de-essing, dynamic frequency shaping, etc.
 */
class DynamicEQBand {
public:
    DynamicEQBand();
    
    void prepare(double sampleRate, int samplesPerBlock);
    void reset();
    
    /**
     * @brief Set static EQ parameters
     */
    void setEQParameters(FilterType type, float frequency, float q, float gainDb);
    
    /**
     * @brief Set dynamic parameters
     * @param threshold Detection threshold in dB
     * @param ratio Compression ratio (1.0 = no compression, higher = more)
     * @param attack Attack time in ms
     * @param release Release time in ms
     * @param enabled Whether dynamic mode is enabled
     */
    void setDynamicParameters(float threshold, float ratio, float attack, float release, bool enabled);
    
    /**
     * @brief Process audio with sidechain detection
     * @param buffer Audio buffer to process
     * @param sidechainBuffer Sidechain signal for detection (can be same as buffer)
     */
    void process(juce::AudioBuffer<float>& buffer, const juce::AudioBuffer<float>& sidechainBuffer);
    
    /**
     * @brief Get current gain reduction from dynamic processing
     */
    float getGainReduction() const { return gainReductionDb.load(); }
    
private:
    StereoBiquadFilter filter;
    LevelDetector detector;
    
    // Static EQ parameters
    FilterType filterType = FilterType::Peak;
    float frequency = 1000.0f;
    float q = 0.707f;
    float staticGainDb = 0.0f;
    
    // Dynamic parameters
    float thresholdDb = -12.0f;
    float ratio = 2.0f;
    bool dynamicEnabled = false;
    
    // State
    std::atomic<float> gainReductionDb { 0.0f };
    double currentSampleRate = 44100.0;
    bool prepared = false;
};

/**
 * @brief Multi-band Dynamic EQ processor
 */
class DynamicEQProcessor {
public:
    static constexpr int numBands = 8;
    
    DynamicEQProcessor();
    
    void prepare(double sampleRate, int samplesPerBlock);
    void reset();
    
    /**
     * @brief Set band parameters
     */
    void setBandParameters(int bandIndex, FilterType type, float freq, float q, float gain, bool enabled);
    
    /**
     * @brief Set dynamic parameters for a band
     */
    void setBandDynamicParameters(int bandIndex, float threshold, float ratio, float attack, float release, bool enabled);
    
    /**
     * @brief Process audio with sidechain
     */
    void process(juce::AudioBuffer<float>& buffer, const juce::AudioBuffer<float>& sidechainBuffer);
    
    /**
     * @brief Get gain reduction for a band
     */
    float getBandGainReduction(int bandIndex) const;
    
private:
    std::array<DynamicEQBand, numBands> bands;
    bool prepared = false;
};

} // namespace SeshEQ

