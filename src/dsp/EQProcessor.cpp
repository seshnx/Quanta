#include "EQProcessor.h"

namespace SeshEQ {

void EQProcessor::prepare(double sampleRate, int /*samplesPerBlock*/) {
    currentSampleRate = sampleRate;
    
    for (int i = 0; i < numBands; ++i) {
        filters[static_cast<size_t>(i)].prepare(sampleRate);
        
        // Initialize smoothers
        smoothers[static_cast<size_t>(i)].frequency.prepare(sampleRate, 20.0);
        smoothers[static_cast<size_t>(i)].q.prepare(sampleRate, 20.0);
        smoothers[static_cast<size_t>(i)].gain.prepare(sampleRate, 20.0);
        
        // Set default parameters
        filters[static_cast<size_t>(i)].setParameters(
            Constants::defaultBandTypes[static_cast<size_t>(i)],
            Constants::defaultBandFrequencies[static_cast<size_t>(i)],
            Constants::defaultQ,
            0.0f
        );
        
        bandEnabled[static_cast<size_t>(i)] = true;
    }
    
    prepared = true;
}

void EQProcessor::reset() {
    for (auto& filter : filters) {
        filter.reset();
    }
}

void EQProcessor::process(juce::AudioBuffer<float>& buffer) {
    if (!prepared) return;
    
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    
    if (numChannels < 1) return;
    
    // Get channel pointers
    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;
    
    // Process each enabled band
    for (int band = 0; band < numBands; ++band) {
        if (!bandEnabled[static_cast<size_t>(band)]) continue;
        
        auto& filter = filters[static_cast<size_t>(band)];
        auto& smoother = smoothers[static_cast<size_t>(band)];
        
        // Check if we need per-sample parameter updates
        const bool needsSmoothing = smoother.frequency.isSmoothing() ||
                                     smoother.q.isSmoothing() ||
                                     smoother.gain.isSmoothing();
        
        if (needsSmoothing) {
            // Per-sample processing with smoothing
            for (int i = 0; i < numSamples; ++i) {
                // Update filter parameters with smoothed values
                filter.setFrequency(smoother.frequency.getNextValue());
                filter.setQ(smoother.q.getNextValue());
                filter.setGain(smoother.gain.getNextValue());
                
                // Process sample
                if (rightChannel) {
                    filter.processStereo(leftChannel[i], rightChannel[i]);
                } else {
                    leftChannel[i] = filter.getMagnitudeAtFrequency(leftChannel[i]);
                }
            }
        } else {
            // Block processing (faster)
            if (rightChannel) {
                filter.processBlock(leftChannel, rightChannel, numSamples);
            } else {
                // Mono - process left channel only
                for (int i = 0; i < numSamples; ++i) {
                    float sample = leftChannel[i];
                    filter.processStereo(sample, sample);
                    leftChannel[i] = sample;
                }
            }
        }
    }
}

void EQProcessor::setBandParameters(int bandIndex, FilterType type, float freq, float q, float gain, bool enabled) {
    if (bandIndex < 0 || bandIndex >= numBands) return;
    
    auto& filter = filters[static_cast<size_t>(bandIndex)];
    auto& smoother = smoothers[static_cast<size_t>(bandIndex)];
    
    // Set filter type immediately (no smoothing)
    filter.setType(type);
    
    // Set target values for smoothing
    smoother.frequency.setTargetValue(freq);
    smoother.q.setTargetValue(q);
    smoother.gain.setTargetValue(gain);
    
    bandEnabled[static_cast<size_t>(bandIndex)] = enabled;
}

void EQProcessor::setBandEnabled(int bandIndex, bool enabled) {
    if (bandIndex < 0 || bandIndex >= numBands) return;
    bandEnabled[static_cast<size_t>(bandIndex)] = enabled;
}

EQProcessor::BandParams EQProcessor::getBandParameters(int bandIndex) const {
    if (bandIndex < 0 || bandIndex >= numBands) {
        return { FilterType::Peak, 1000.0f, 0.707f, 0.0f, false };
    }
    
    const auto& filter = filters[static_cast<size_t>(bandIndex)];
    return {
        filter.getType(),
        filter.getFrequency(),
        filter.getQ(),
        filter.getGain(),
        bandEnabled[static_cast<size_t>(bandIndex)]
    };
}

float EQProcessor::getMagnitudeAtFrequency(float frequency) const {
    float totalMagnitude = 1.0f;
    
    for (int i = 0; i < numBands; ++i) {
        if (bandEnabled[static_cast<size_t>(i)]) {
            totalMagnitude *= filters[static_cast<size_t>(i)].getMagnitudeAtFrequency(frequency);
        }
    }
    
    return totalMagnitude;
}

float EQProcessor::getBandMagnitudeAtFrequency(int bandIndex, float frequency) const {
    if (bandIndex < 0 || bandIndex >= numBands) return 1.0f;
    
    if (!bandEnabled[static_cast<size_t>(bandIndex)]) return 1.0f;
    
    return filters[static_cast<size_t>(bandIndex)].getMagnitudeAtFrequency(frequency);
}

void EQProcessor::connectToParameters(juce::AudioProcessorValueTreeState& apvts) {
    using namespace ParamIDs;
    
    for (int i = 0; i < numBands; ++i) {
        auto& ptrs = paramPtrs[static_cast<size_t>(i)];
        
        ptrs.frequency = apvts.getRawParameterValue(getBandParamID(i, bandFreq));
        ptrs.q = apvts.getRawParameterValue(getBandParamID(i, bandQ));
        ptrs.gain = apvts.getRawParameterValue(getBandParamID(i, bandGain));
        ptrs.type = apvts.getRawParameterValue(getBandParamID(i, bandType));
        ptrs.enabled = apvts.getRawParameterValue(getBandParamID(i, bandEnable));
    }
}

void EQProcessor::updateFromParameters() {
    for (int i = 0; i < numBands; ++i) {
        const auto& ptrs = paramPtrs[static_cast<size_t>(i)];
        
        if (ptrs.frequency && ptrs.q && ptrs.gain && ptrs.type && ptrs.enabled) {
            setBandParameters(
                i,
                static_cast<FilterType>(static_cast<int>(ptrs.type->load())),
                ptrs.frequency->load(),
                ptrs.q->load(),
                ptrs.gain->load(),
                ptrs.enabled->load() > 0.5f
            );
        }
    }
}

} // namespace SeshEQ
