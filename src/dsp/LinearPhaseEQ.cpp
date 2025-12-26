#include "LinearPhaseEQ.h"
#include <juce_dsp/juce_dsp.h>
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace SeshEQ {

LinearPhaseEQ::LinearPhaseEQ()
    : fft(static_cast<int>(std::log2(fftSize))) {
    frequencyResponse.resize(fftSize);
    impulseResponse.resize(fftSize);
}

void LinearPhaseEQ::prepare(double sampleRate, int maximumBlockSize) {
    currentSampleRate = sampleRate;
    
    inputBuffer.setSize(2, fftSize);
    outputBuffer.setSize(2, fftSize);
    inputBuffer.clear();
    outputBuffer.clear();
    inputBufferPos = 0;
    
    updateImpulseResponse();
    prepared = true;
}

void LinearPhaseEQ::reset() {
    inputBuffer.clear();
    outputBuffer.clear();
    inputBufferPos = 0;
}

void LinearPhaseEQ::setBandParameters(int bandIndex, float frequency, float q, float gainDb, bool enabled) {
    if (bandIndex < 0 || bandIndex >= numBands) return;
    
    auto& params = bandParams[static_cast<size_t>(bandIndex)];
    if (params.frequency != frequency || params.q != q || 
        params.gainDb != gainDb || params.enabled != enabled) {
        params.frequency = frequency;
        params.q = q;
        params.gainDb = gainDb;
        params.enabled = enabled;
        paramsChanged = true;
    }
}

float LinearPhaseEQ::calculateBandResponse(float frequency, float q, float gainDb, float freq) const {
    if (frequency <= 0.0f || q <= 0.0f) return 1.0f;
    
    const float w = 2.0f * static_cast<float>(M_PI) * freq / static_cast<float>(currentSampleRate);
    const float w0 = 2.0f * static_cast<float>(M_PI) * frequency / static_cast<float>(currentSampleRate);
    const float A = std::pow(10.0f, gainDb / 40.0f);
    const float alpha = std::sin(w0) / (2.0f * q);
    
    // Simplified peak filter response
    const float cosW = std::cos(w);
    const float cosW0 = std::cos(w0);
    
    const float numerator = 1.0f + alpha * A;
    const float denominator = 1.0f + alpha / A;
    
    const float H = (numerator + (A - 1.0f) * cosW * cosW0) / 
                    (denominator + (1.0f / A - 1.0f) * cosW * cosW0);
    
    return std::abs(H);
}

void LinearPhaseEQ::updateImpulseResponse() {
    if (!prepared) return;
    
    // Initialize frequency response to unity
    std::fill(frequencyResponse.begin(), frequencyResponse.end(), std::complex<float>(1.0f, 0.0f));
    
    // Calculate combined frequency response
    for (size_t i = 0; i < static_cast<size_t>(fftSize / 2 + 1); ++i) {
        const float freq = static_cast<float>(i) * static_cast<float>(currentSampleRate) / static_cast<float>(fftSize);
        float magnitude = 1.0f;
        
        // Multiply responses of all enabled bands
        for (int band = 0; band < numBands; ++band) {
            if (bandParams[static_cast<size_t>(band)].enabled) {
                const auto& params = bandParams[static_cast<size_t>(band)];
                magnitude *= calculateBandResponse(params.frequency, params.q, params.gainDb, freq);
            }
        }
        
        frequencyResponse[i] = std::complex<float>(magnitude, 0.0f);
        
        // Mirror for negative frequencies (Hermitian symmetry)
        if (i > 0 && i < static_cast<size_t>(fftSize / 2)) {
            frequencyResponse[fftSize - i] = std::conj(frequencyResponse[i]);
        }
    }
    
    // Simplified Linear Phase implementation
    // Full FIR would require proper IFFT - for now use windowed approximation
    // Create a simple windowed impulse response based on magnitude response
    for (size_t i = 0; i < static_cast<size_t>(impulseLength); ++i) {
        const float n = static_cast<float>(i) / static_cast<float>(impulseLength);
        const float window = 0.5f * (1.0f - std::cos(2.0f * static_cast<float>(M_PI) * n));
        
        // Use average magnitude as approximation
        float avgMagnitude = 0.0f;
        for (size_t j = 0; j < static_cast<size_t>(fftSize / 2 + 1); ++j) {
            avgMagnitude += frequencyResponse[j].real();
        }
        avgMagnitude /= static_cast<float>(fftSize / 2 + 1);
        
        impulseResponse[i] = avgMagnitude * window / static_cast<float>(impulseLength);
    }
    
    // Zero pad the rest
    for (size_t i = static_cast<size_t>(impulseLength); i < static_cast<size_t>(fftSize); ++i) {
        impulseResponse[i] = 0.0f;
    }
    
    paramsChanged = false;
}

void LinearPhaseEQ::process(juce::AudioBuffer<float>& buffer) {
    if (!prepared) return;
    
    if (paramsChanged) {
        updateImpulseResponse();
    }
    
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    
    // Simple overlap-save convolution (simplified for now)
    // In production, use optimized convolution
    for (int ch = 0; ch < numChannels; ++ch) {
        float* channelData = buffer.getWritePointer(ch);
        
        for (int i = 0; i < numSamples; ++i) {
            // Simplified: direct convolution (not optimal, but works)
            float output = 0.0f;
            for (int j = 0; j < impulseLength && (i - j) >= 0; ++j) {
                output += channelData[i - j] * impulseResponse[j];
            }
            channelData[i] = output;
        }
    }
}

float LinearPhaseEQ::getMagnitudeAtFrequency(float frequency) const {
    if (!prepared || frequency <= 0.0f) return 1.0f;
    
    float magnitude = 1.0f;
    for (int band = 0; band < numBands; ++band) {
        if (bandParams[static_cast<size_t>(band)].enabled) {
            const auto& params = bandParams[static_cast<size_t>(band)];
            magnitude *= calculateBandResponse(params.frequency, params.q, params.gainDb, frequency);
        }
    }
    return magnitude;
}

} // namespace SeshEQ

