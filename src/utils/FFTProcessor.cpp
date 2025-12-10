#include "FFTProcessor.h"
#include <cmath>

namespace SeshEQ {

FFTProcessor::FFTProcessor() {
    magnitudes.fill(-100.0f);
    smoothedMagnitudes.fill(-100.0f);
}

void FFTProcessor::prepare(double newSampleRate) {
    sampleRate = newSampleRate;
    inputIndex = 0;
    inputBuffer.fill(0.0f);
    fftData.fill(0.0f);
    magnitudes.fill(-100.0f);
    smoothedMagnitudes.fill(-100.0f);
}

void FFTProcessor::pushSamples(const float* samples, int numSamples) {
    for (int i = 0; i < numSamples; ++i) {
        inputBuffer[static_cast<size_t>(inputIndex)] = samples[i];
        ++inputIndex;
        
        if (inputIndex >= fftSize) {
            inputIndex = 0;
            processFFT();
        }
    }
}

void FFTProcessor::pushBuffer(const juce::AudioBuffer<float>& buffer) {
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    
    if (numChannels == 0 || numSamples == 0) return;
    
    // Average channels for mono analysis
    if (numChannels == 1) {
        pushSamples(buffer.getReadPointer(0), numSamples);
    } else {
        // Mix to mono
        for (int i = 0; i < numSamples; ++i) {
            float sum = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch) {
                sum += buffer.getSample(ch, i);
            }
            const float mono = sum / static_cast<float>(numChannels);
            
            inputBuffer[static_cast<size_t>(inputIndex)] = mono;
            ++inputIndex;
            
            if (inputIndex >= fftSize) {
                inputIndex = 0;
                processFFT();
            }
        }
    }
}

void FFTProcessor::processFFT() {
    // Copy input to FFT buffer and apply window
    for (int i = 0; i < fftSize; ++i) {
        fftData[static_cast<size_t>(i)] = inputBuffer[static_cast<size_t>(i)];
    }
    
    // Apply window function
    window.multiplyWithWindowingTable(fftData.data(), fftSize);
    
    // Perform FFT
    fft.performFrequencyOnlyForwardTransform(fftData.data());
    
    // Calculate magnitudes in dB
    const float minDb = -100.0f;
    const float maxDb = 0.0f;
    
    juce::SpinLock::ScopedLockType scopedLock(lock);
    
    for (int i = 0; i < numBins; ++i) {
        const float magnitude = fftData[static_cast<size_t>(i)];
        
        // Convert to dB (normalized)
        float db = magnitude > 0.0f 
            ? 20.0f * std::log10(magnitude / static_cast<float>(fftSize))
            : minDb;
        
        db = std::clamp(db, minDb, maxDb);
        
        // Smooth the spectrum (exponential decay)
        if (db > smoothedMagnitudes[static_cast<size_t>(i)]) {
            // Fast attack
            smoothedMagnitudes[static_cast<size_t>(i)] = db;
        } else {
            // Slow decay
            smoothedMagnitudes[static_cast<size_t>(i)] = 
                smoothedMagnitudes[static_cast<size_t>(i)] * decayRate + 
                db * (1.0f - decayRate);
        }
        
        magnitudes[static_cast<size_t>(i)] = smoothedMagnitudes[static_cast<size_t>(i)];
    }
    
    newDataAvailable.store(true);
}

const std::array<float, FFTProcessor::numBins>& FFTProcessor::getMagnitudes() {
    newDataAvailable.store(false);
    return magnitudes;
}

float FFTProcessor::getFrequencyForBin(int binIndex) const {
    return static_cast<float>(binIndex) * static_cast<float>(sampleRate) / static_cast<float>(fftSize);
}

int FFTProcessor::getBinForFrequency(float frequency) const {
    return static_cast<int>(frequency * static_cast<float>(fftSize) / static_cast<float>(sampleRate));
}

} // namespace SeshEQ
