#include "DynamicEQ.h"
#include "LevelDetector.h"
#include <algorithm>
#include <atomic>

namespace SeshEQ {

//==============================================================================
// DynamicEQBand
//==============================================================================

DynamicEQBand::DynamicEQBand() = default;

void DynamicEQBand::prepare(double sampleRate, int /*samplesPerBlock*/) {
    currentSampleRate = sampleRate;
    filter.prepare(sampleRate);
    detector.prepare(sampleRate);
    prepared = true;
}

void DynamicEQBand::reset() {
    filter.reset();
    detector.reset();
    gainReductionDb.store(0.0f);
}

void DynamicEQBand::setEQParameters(FilterType type, float freq, float qValue, float gainDb) {
    filterType = type;
    frequency = freq;
    q = qValue;
    staticGainDb = gainDb;
    
    if (prepared) {
        filter.setParameters(type, freq, qValue, gainDb);
    }
}

void DynamicEQBand::setDynamicParameters(float threshold, float ratioValue, float attack, float release, bool enabled) {
    thresholdDb = threshold;
    ratio = ratioValue;
    dynamicEnabled = enabled;
    
    if (prepared) {
        detector.setAttackTime(attack);
        detector.setReleaseTime(release);
    }
}

void DynamicEQBand::process(juce::AudioBuffer<float>& buffer, const juce::AudioBuffer<float>& sidechainBuffer) {
    if (!prepared || !dynamicEnabled) {
        // Process with static gain only
        filter.setParameters(filterType, frequency, q, staticGainDb);
        const int numSamples = buffer.getNumSamples();
        if (buffer.getNumChannels() >= 2) {
            filter.processBlock(buffer.getWritePointer(0), buffer.getWritePointer(1), numSamples);
        } else if (buffer.getNumChannels() >= 1) {
            float* left = buffer.getWritePointer(0);
            for (int i = 0; i < numSamples; ++i) {
                float sample = left[i];
                filter.processStereo(sample, sample);
                left[i] = sample;
            }
        }
        gainReductionDb.store(0.0f);
        return;
    }
    
    // Detect level from sidechain
    const int numSamples = buffer.getNumSamples();
    const int numChannels = sidechainBuffer.getNumChannels();
    
    float maxLevel = 0.0f;
    for (int ch = 0; ch < numChannels; ++ch) {
        const float* channelData = sidechainBuffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; ++i) {
            maxLevel = std::max(maxLevel, std::abs(channelData[i]));
        }
    }
    
    const float levelDb = dBUtils::linearToDb(maxLevel);
    
    // Calculate gain reduction based on threshold and ratio
    float dynamicGainDb = staticGainDb;
    float gr = 0.0f;
    
    if (levelDb > thresholdDb) {
        const float excess = levelDb - thresholdDb;
        const float compressedExcess = excess / ratio;
        gr = excess - compressedExcess;
        dynamicGainDb = staticGainDb - gr;  // Reduce gain when level exceeds threshold
    }
    
    gainReductionDb.store(gr);
    
    // Apply dynamic gain to filter
    filter.setParameters(filterType, frequency, q, dynamicGainDb);
    
    // Process audio
    const int bufferChannels = buffer.getNumChannels();
    if (bufferChannels >= 2) {
        filter.processBlock(buffer.getWritePointer(0), buffer.getWritePointer(1), numSamples);
    } else if (bufferChannels >= 1) {
        float* left = buffer.getWritePointer(0);
        for (int i = 0; i < numSamples; ++i) {
            float sample = left[i];
            filter.processStereo(sample, sample);
            left[i] = sample;
        }
    }
}

//==============================================================================
// DynamicEQProcessor
//==============================================================================

DynamicEQProcessor::DynamicEQProcessor() = default;

void DynamicEQProcessor::prepare(double sampleRate, int samplesPerBlock) {
    for (auto& band : bands) {
        band.prepare(sampleRate, samplesPerBlock);
    }
    prepared = true;
}

void DynamicEQProcessor::reset() {
    for (auto& band : bands) {
        band.reset();
    }
}

void DynamicEQProcessor::setBandParameters(int bandIndex, FilterType type, float freq, float q, float gain, bool enabled) {
    if (bandIndex >= 0 && bandIndex < numBands) {
        bands[static_cast<size_t>(bandIndex)].setEQParameters(type, freq, q, gain);
    }
}

void DynamicEQProcessor::setBandDynamicParameters(int bandIndex, float threshold, float ratio, float attack, float release, bool enabled) {
    if (bandIndex >= 0 && bandIndex < numBands) {
        bands[static_cast<size_t>(bandIndex)].setDynamicParameters(threshold, ratio, attack, release, enabled);
    }
}

void DynamicEQProcessor::process(juce::AudioBuffer<float>& buffer, const juce::AudioBuffer<float>& sidechainBuffer) {
    if (!prepared) return;
    
    // Process each band in series
    for (auto& band : bands) {
        band.process(buffer, sidechainBuffer);
    }
}

float DynamicEQProcessor::getBandGainReduction(int bandIndex) const {
    if (bandIndex >= 0 && bandIndex < numBands) {
        return bands[static_cast<size_t>(bandIndex)].getGainReduction();
    }
    return 0.0f;
}

} // namespace SeshEQ

