#pragma once

#include <cmath>
#include <array>

namespace SeshEQ {

//==============================================================================
// Filter Types (standalone, no JUCE dependency)
//==============================================================================
enum class FilterType {
    LowPass = 0,
    HighPass,
    BandPass,
    Notch,
    Peak,       // Bell/Parametric
    LowShelf,
    HighShelf,
    AllPass
};

/**
 * @brief Biquad filter implementation using Direct Form II Transposed
 * 
 * Based on Robert Bristow-Johnson's Audio EQ Cookbook.
 * Uses double precision for coefficient calculation, float for processing.
 * 
 * Transfer function: H(z) = (b0 + b1*z^-1 + b2*z^-2) / (1 + a1*z^-1 + a2*z^-2)
 */
class BiquadFilter {
public:
    BiquadFilter() = default;
    
    /**
     * @brief Prepare the filter for processing
     * @param sampleRate The audio sample rate
     */
    void prepare(double sampleRate);
    
    /**
     * @brief Reset filter state (clear delay lines)
     */
    void reset();
    
    /**
     * @brief Set filter parameters and recalculate coefficients
     * @param type Filter type
     * @param frequency Center/cutoff frequency in Hz
     * @param q Q factor (bandwidth)
     * @param gainDb Gain in dB (for shelf and peak filters)
     */
    void setParameters(FilterType type, float frequency, float q, float gainDb = 0.0f);
    
    /**
     * @brief Update only the frequency
     */
    void setFrequency(float frequency);
    
    /**
     * @brief Update only the Q
     */
    void setQ(float q);
    
    /**
     * @brief Update only the gain (for shelf/peak)
     */
    void setGain(float gainDb);
    
    /**
     * @brief Update the filter type
     */
    void setType(FilterType type);
    
    /**
     * @brief Process a single sample
     */
    float processSample(float input);
    
    /**
     * @brief Process a block of samples in place
     */
    void processBlock(float* data, int numSamples);
    
    /**
     * @brief Get the magnitude response at a given frequency
     * @param frequency Frequency in Hz
     * @return Magnitude (linear scale)
     */
    float getMagnitudeAtFrequency(float frequency) const;

    /**
     * @brief Get the phase response at a given frequency
     * @param frequency Frequency in Hz
     * @return Phase in radians
     */
    float getPhaseAtFrequency(float frequency) const;

    /**
     * @brief Calculate magnitude from parameters (static, no filter state needed)
     * @param type Filter type
     * @param frequency Center frequency in Hz
     * @param q Q factor
     * @param gainDb Gain in dB
     * @param sampleRate Sample rate
     * @param evalFrequency Frequency to evaluate magnitude at
     * @return Magnitude (linear scale)
     */
    static float calcMagnitudeFromParams(FilterType type, float frequency, float q,
                                          float gainDb, double sampleRate, float evalFrequency);
    
    // Getters for current parameters
    FilterType getType() const { return currentType; }
    float getFrequency() const { return currentFreq; }
    float getQ() const { return currentQ; }
    float getGain() const { return currentGain; }
    
    // Get coefficients (for debugging/visualization)
    struct Coefficients {
        double b0, b1, b2;  // Numerator (feedforward)
        double a1, a2;       // Denominator (feedback), a0 normalized to 1
    };
    
    Coefficients getCoefficients() const { return { b0, b1, b2, a1, a2 }; }
    
private:
    /**
     * @brief Recalculate filter coefficients based on current parameters
     */
    void updateCoefficients();
    
    // Sample rate
    double sampleRate = 44100.0;
    
    // Current parameters
    FilterType currentType = FilterType::Peak;
    float currentFreq = 1000.0f;
    float currentQ = 0.707f;
    float currentGain = 0.0f;
    
    // Filter coefficients (normalized, a0 = 1)
    double b0 = 1.0, b1 = 0.0, b2 = 0.0;
    double a1 = 0.0, a2 = 0.0;
    
    // State variables (Direct Form II Transposed)
    double z1 = 0.0;  // z^-1 state
    double z2 = 0.0;  // z^-2 state
    
    // Pi constant
    static constexpr double pi = 3.14159265358979323846;
};

/**
 * @brief Stereo biquad filter (processes two channels)
 */
class StereoBiquadFilter {
public:
    void prepare(double sampleRate) {
        leftFilter.prepare(sampleRate);
        rightFilter.prepare(sampleRate);
    }
    
    void reset() {
        leftFilter.reset();
        rightFilter.reset();
    }
    
    void setParameters(FilterType type, float frequency, float q, float gainDb = 0.0f) {
        leftFilter.setParameters(type, frequency, q, gainDb);
        rightFilter.setParameters(type, frequency, q, gainDb);
    }
    
    void setFrequency(float frequency) {
        leftFilter.setFrequency(frequency);
        rightFilter.setFrequency(frequency);
    }
    
    void setQ(float q) {
        leftFilter.setQ(q);
        rightFilter.setQ(q);
    }
    
    void setGain(float gainDb) {
        leftFilter.setGain(gainDb);
        rightFilter.setGain(gainDb);
    }
    
    void setType(FilterType type) {
        leftFilter.setType(type);
        rightFilter.setType(type);
    }
    
    void processStereo(float& left, float& right) {
        left = leftFilter.processSample(left);
        right = rightFilter.processSample(right);
    }
    
    void processBlock(float* leftData, float* rightData, int numSamples) {
        leftFilter.processBlock(leftData, numSamples);
        rightFilter.processBlock(rightData, numSamples);
    }
    
    float getMagnitudeAtFrequency(float frequency) const {
        return leftFilter.getMagnitudeAtFrequency(frequency);
    }
    
    BiquadFilter::Coefficients getCoefficients() const {
        return leftFilter.getCoefficients();
    }
    
    FilterType getType() const { return leftFilter.getType(); }
    float getFrequency() const { return leftFilter.getFrequency(); }
    float getQ() const { return leftFilter.getQ(); }
    float getGain() const { return leftFilter.getGain(); }
    
private:
    BiquadFilter leftFilter;
    BiquadFilter rightFilter;
};

} // namespace SeshEQ
