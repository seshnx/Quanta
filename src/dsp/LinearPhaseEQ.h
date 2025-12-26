#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <vector>
#include <complex>
#include <cmath>

namespace SeshEQ {

/**
 * @brief Linear Phase EQ using FIR filters
 * 
 * Provides zero phase distortion at the cost of higher latency.
 * Uses FFT-based convolution for efficient processing.
 */
class LinearPhaseEQ {
public:
    LinearPhaseEQ();
    ~LinearPhaseEQ() = default;
    
    void prepare(double sampleRate, int maximumBlockSize);
    void reset();
    
    /**
     * @brief Set EQ band parameters
     * @param bandIndex Band index (0-7)
     * @param frequency Center frequency in Hz
     * @param q Q factor
     * @param gainDb Gain in dB
     * @param enabled Whether band is enabled
     */
    void setBandParameters(int bandIndex, float frequency, float q, float gainDb, bool enabled);
    
    /**
     * @brief Process audio buffer
     */
    void process(juce::AudioBuffer<float>& buffer);
    
    /**
     * @brief Get magnitude response at frequency
     */
    float getMagnitudeAtFrequency(float frequency) const;
    
    /**
     * @brief Get latency in samples
     */
    int getLatency() const { return fftSize / 2; }
    
private:
    static constexpr int numBands = 8;
    static constexpr int fftSize = 4096;  // Must be power of 2
    static constexpr int impulseLength = fftSize / 2;
    
    void updateImpulseResponse();
    float calculateBandResponse(float frequency, float q, float gainDb, float freq) const;
    
    struct BandParams {
        float frequency = 1000.0f;
        float q = 0.707f;
        float gainDb = 0.0f;
        bool enabled = false;
    };
    
    std::array<BandParams, numBands> bandParams;
    bool paramsChanged = true;
    
    // FFT for convolution
    juce::dsp::FFT fft;
    std::vector<std::complex<float>> frequencyResponse;
    std::vector<float> impulseResponse;
    
    // Overlap-save buffers
    juce::AudioBuffer<float> inputBuffer;
    juce::AudioBuffer<float> outputBuffer;
    int inputBufferPos = 0;
    
    double currentSampleRate = 44100.0;
    bool prepared = false;
};

} // namespace SeshEQ

