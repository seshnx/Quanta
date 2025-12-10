#pragma once

#include <juce_dsp/juce_dsp.h>
#include <memory>

namespace SeshEQ {

/**
 * @brief Oversampling factors
 */
enum class OversamplingFactor {
    None = 0,   // 1x (no oversampling)
    x2 = 1,     // 2x oversampling
    x4 = 2,     // 4x oversampling
    x8 = 3      // 8x oversampling
};

/**
 * @brief Oversampler wrapper for JUCE's oversampling
 * 
 * Provides easy-to-use oversampling for reducing aliasing
 * in nonlinear processing (saturation, limiting, etc.)
 */
class Oversampler {
public:
    Oversampler() = default;
    
    /**
     * @brief Prepare the oversampler
     * @param numChannels Number of audio channels
     * @param maxBlockSize Maximum samples per block
     * @param factor Oversampling factor
     */
    void prepare(int numChannels, int maxBlockSize, OversamplingFactor factor = OversamplingFactor::x2) {
        currentFactor = factor;
        
        if (factor == OversamplingFactor::None) {
            oversampler.reset();
            return;
        }
        
        // Create JUCE oversampler with specified factor
        // Using IIR filtering (lower latency than FIR)
        const int factorIndex = static_cast<int>(factor);
        
        oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
            static_cast<size_t>(numChannels),
            static_cast<size_t>(factorIndex),
            juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
            true  // Use maximum quality
        );
        
        oversampler->initProcessing(static_cast<size_t>(maxBlockSize));
    }
    
    /**
     * @brief Reset the oversampler state
     */
    void reset() {
        if (oversampler)
            oversampler->reset();
    }
    
    /**
     * @brief Set the oversampling factor (requires re-prepare)
     */
    void setFactor(OversamplingFactor factor) {
        currentFactor = factor;
    }
    
    /**
     * @brief Get current oversampling factor
     */
    OversamplingFactor getFactor() const { return currentFactor; }
    
    /**
     * @brief Get the actual multiplier (1, 2, 4, or 8)
     */
    int getFactorMultiplier() const {
        return 1 << static_cast<int>(currentFactor);
    }
    
    /**
     * @brief Get latency in samples introduced by oversampling
     */
    float getLatency() const {
        if (!oversampler || currentFactor == OversamplingFactor::None)
            return 0.0f;
        return oversampler->getLatencyInSamples();
    }
    
    /**
     * @brief Upsample and return the oversampled block
     * @param inputBlock The input audio block
     * @return Reference to oversampled block (owned by oversampler)
     */
    juce::dsp::AudioBlock<float> upsample(const juce::dsp::AudioBlock<float>& inputBlock) {
        if (!oversampler || currentFactor == OversamplingFactor::None)
            return inputBlock;
        
        return oversampler->processSamplesUp(inputBlock);
    }
    
    /**
     * @brief Downsample back to original rate
     * @param outputBlock The output block to write to
     */
    void downsample(juce::dsp::AudioBlock<float>& outputBlock) {
        if (!oversampler || currentFactor == OversamplingFactor::None)
            return;
        
        oversampler->processSamplesDown(outputBlock);
    }
    
    /**
     * @brief Check if oversampling is enabled
     */
    bool isEnabled() const {
        return oversampler != nullptr && currentFactor != OversamplingFactor::None;
    }
    
    /**
     * @brief Get the oversampled sample rate
     */
    double getOversampledSampleRate(double originalSampleRate) const {
        return originalSampleRate * getFactorMultiplier();
    }

private:
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    OversamplingFactor currentFactor = OversamplingFactor::None;
};

/**
 * @brief Get names for oversampling factors (for UI)
 */
inline juce::StringArray getOversamplingFactorNames() {
    return { "Off", "2x", "4x", "8x" };
}

} // namespace SeshEQ
