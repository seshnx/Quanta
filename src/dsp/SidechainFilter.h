#pragma once

#include "BiquadFilter.h"
#include <juce_audio_basics/juce_audio_basics.h>

namespace SeshEQ {

/**
 * @brief Sidechain filter modes
 */
enum class SidechainFilterMode {
    Off = 0,            // No filtering
    HighPass,           // High-pass filter (common for de-essing)
    LowPass,            // Low-pass filter
    BandPass,           // Band-pass filter
    Tilt                // Tilt EQ (boost highs, cut lows or vice versa)
};

/**
 * @brief Sidechain filter for dynamics processors
 * 
 * Filters the sidechain signal before level detection.
 * Useful for:
 * - High-pass to ignore low frequencies (de-pumping)
 * - Low-pass to focus compression on bass
 * - Band-pass for de-essing
 */
class SidechainFilter {
public:
    SidechainFilter() = default;
    
    /**
     * @brief Prepare the filter
     */
    void prepare(double sampleRate) {
        currentSampleRate = sampleRate;
        filter.prepare(sampleRate);
        updateFilter();
    }
    
    /**
     * @brief Reset filter state
     */
    void reset() {
        filter.reset();
    }
    
    /**
     * @brief Set filter mode
     */
    void setMode(SidechainFilterMode newMode) {
        if (mode != newMode) {
            mode = newMode;
            updateFilter();
        }
    }
    
    /**
     * @brief Set filter frequency
     */
    void setFrequency(float freq) {
        if (std::abs(frequency - freq) > 0.001f) {
            frequency = freq;
            updateFilter();
        }
    }
    
    /**
     * @brief Set filter Q (for band-pass mode)
     */
    void setQ(float newQ) {
        if (std::abs(q - newQ) > 0.001f) {
            q = newQ;
            updateFilter();
        }
    }
    
    /**
     * @brief Set tilt amount (-1 to +1, for tilt mode)
     * Negative = more lows, Positive = more highs
     */
    void setTilt(float tiltAmount) {
        tilt = std::clamp(tiltAmount, -1.0f, 1.0f);
        updateFilter();
    }
    
    /**
     * @brief Enable/disable the filter
     */
    void setEnabled(bool shouldBeEnabled) {
        enabled = shouldBeEnabled;
    }
    
    /**
     * @brief Check if filter is active
     */
    bool isActive() const {
        return enabled && mode != SidechainFilterMode::Off;
    }
    
    /**
     * @brief Process a single sample
     */
    float processSample(float input) {
        if (!isActive()) return input;
        return filter.processSample(input);
    }
    
    /**
     * @brief Process audio buffer (in-place, mono)
     */
    void process(float* data, int numSamples) {
        if (!isActive()) return;
        filter.processBlock(data, numSamples);
    }
    
    /**
     * @brief Process stereo to mono sidechain signal
     */
    float processStereo(float left, float right) {
        // Mix to mono first
        float mono = (left + right) * 0.5f;
        return processSample(mono);
    }
    
    // Getters
    SidechainFilterMode getMode() const { return mode; }
    float getFrequency() const { return frequency; }
    float getQ() const { return q; }
    bool isEnabled() const { return enabled; }
    
private:
    void updateFilter() {
        switch (mode) {
            case SidechainFilterMode::Off:
                // No filter
                break;
                
            case SidechainFilterMode::HighPass:
                filter.setParameters(FilterType::HighPass, frequency, 0.707f, 0.0f);
                break;
                
            case SidechainFilterMode::LowPass:
                filter.setParameters(FilterType::LowPass, frequency, 0.707f, 0.0f);
                break;
                
            case SidechainFilterMode::BandPass:
                filter.setParameters(FilterType::BandPass, frequency, q, 0.0f);
                break;
                
            case SidechainFilterMode::Tilt: {
                // Use a shelf filter for tilt
                // Positive tilt = high shelf boost + implicit low cut
                // Negative tilt = low shelf boost
                const float gain = tilt * 12.0f;  // +/-12 dB max
                if (tilt >= 0.0f) {
                    filter.setParameters(FilterType::HighShelf, frequency, 0.5f, gain);
                } else {
                    filter.setParameters(FilterType::LowShelf, frequency, 0.5f, -gain);
                }
                break;
            }
        }
    }
    
    BiquadFilter filter;
    
    SidechainFilterMode mode = SidechainFilterMode::Off;
    float frequency = 100.0f;
    float q = 1.0f;
    float tilt = 0.0f;
    bool enabled = true;
    
    double currentSampleRate = 44100.0;
};

/**
 * @brief Get names for sidechain filter modes (for UI)
 */
inline juce::StringArray getSidechainFilterModeNames() {
    return { "Off", "High Pass", "Low Pass", "Band Pass", "Tilt" };
}

} // namespace SeshEQ
