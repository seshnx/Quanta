#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <atomic>
#include <vector>

namespace SeshEQ {

/**
 * @brief FFT processor for spectrum analysis
 * 
 * Provides real-time FFT analysis with windowing and averaging.
 * Thread-safe for use between audio and GUI threads.
 */
class FFTProcessor {
public:
    // FFT size options
    static constexpr int fftOrder = 11;  // 2^11 = 2048 points
    static constexpr int fftSize = 1 << fftOrder;
    static constexpr int numBins = fftSize / 2;
    
    FFTProcessor();
    
    /**
     * @brief Prepare the FFT processor
     */
    void prepare(double sampleRate);
    
    /**
     * @brief Push samples into the FFT buffer (call from audio thread)
     */
    void pushSamples(const float* samples, int numSamples);
    
    /**
     * @brief Push a stereo buffer (averages L+R)
     */
    void pushBuffer(const juce::AudioBuffer<float>& buffer);
    
    /**
     * @brief Check if new FFT data is available
     */
    bool isNewDataAvailable() const { return newDataAvailable.load(); }
    
    /**
     * @brief Get the current magnitude spectrum (call from GUI thread)
     * @return Array of magnitudes in dB, from 0 to Nyquist
     */
    const std::array<float, numBins>& getMagnitudes();
    
    /**
     * @brief Get frequency for a given bin index
     */
    float getFrequencyForBin(int binIndex) const;
    
    /**
     * @brief Get bin index for a given frequency
     */
    int getBinForFrequency(float frequency) const;
    
    /**
     * @brief Get the sample rate
     */
    double getSampleRate() const { return sampleRate; }
    
    /**
     * @brief Set decay rate for spectrum smoothing (0-1, higher = faster decay)
     */
    void setDecayRate(float rate) { decayRate = rate; }

private:
    void processFFT();
    void applyWindow();
    
    // FFT engine
    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window { fftSize, juce::dsp::WindowingFunction<float>::hann };
    
    // Buffers
    std::array<float, fftSize> inputBuffer {};
    std::array<float, fftSize * 2> fftData {};
    std::array<float, numBins> magnitudes {};
    std::array<float, numBins> smoothedMagnitudes {};
    
    // State
    int inputIndex = 0;
    double sampleRate = 44100.0;
    float decayRate = 0.7f;
    
    // Thread synchronization
    std::atomic<bool> newDataAvailable { false };
    juce::SpinLock lock;
};

/**
 * @brief Dual FFT processor for pre/post analysis
 */
class DualFFTProcessor {
public:
    void prepare(double sampleRate) {
        preFFT.prepare(sampleRate);
        postFFT.prepare(sampleRate);
    }
    
    void pushPreSamples(const juce::AudioBuffer<float>& buffer) {
        preFFT.pushBuffer(buffer);
    }
    
    void pushPostSamples(const juce::AudioBuffer<float>& buffer) {
        postFFT.pushBuffer(buffer);
    }
    
    FFTProcessor& getPreFFT() { return preFFT; }
    FFTProcessor& getPostFFT() { return postFFT; }
    
private:
    FFTProcessor preFFT;
    FFTProcessor postFFT;
};

} // namespace SeshEQ
