#include "EQProcessor.h"
#include "utils/MidSideProcessor.h"

namespace SeshEQ {

void EQProcessor::prepare(double sampleRate, int samplesPerBlock) {
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
        
        // Prepare per-band dynamics
        bandDynamics[static_cast<size_t>(i)].prepare(sampleRate, samplesPerBlock);
    }
    
    // Prepare Mid/Side buffers
    midBuffer.setSize(1, samplesPerBlock);
    sideBuffer.setSize(1, samplesPerBlock);
    
    // Prepare Linear Phase EQ
    if (linearPhaseMode) {
        if (!linearPhaseEQ) {
            linearPhaseEQ = std::make_unique<LinearPhaseEQ>();
        }
        linearPhaseEQ->prepare(sampleRate, samplesPerBlock);
    }
    
    // Prepare Dynamic EQ
    if (dynamicEQMode) {
        if (!dynamicEQ) {
            dynamicEQ = std::make_unique<DynamicEQProcessor>();
        }
        dynamicEQ->prepare(sampleRate, samplesPerBlock);
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
    
    if (numChannels < 1) return;
    
    // Use Linear Phase EQ if enabled
    if (linearPhaseMode && linearPhaseEQ) {
        linearPhaseEQ->process(buffer);
        return;
    }
    
    // Use Dynamic EQ if enabled
    if (dynamicEQMode && dynamicEQ) {
        dynamicEQ->process(buffer, buffer);  // Use same buffer as sidechain
        return;
    }
    
    // Standard processing with optional Mid/Side
    if (midSideMode && numChannels >= 2) {
        // Process in Mid/Side domain
        MidSideProcessor::process(buffer,
            [this](float* mid, int numSamples) {
                // Process mid channel
                juce::AudioBuffer<float> midBuf(1, numSamples);
                std::copy(mid, mid + numSamples, midBuf.getWritePointer(0));
                processStandard(midBuf);
                std::copy(midBuf.getReadPointer(0), midBuf.getReadPointer(0) + numSamples, mid);
            },
            [this](float* side, int numSamples) {
                // Process side channel
                juce::AudioBuffer<float> sideBuf(1, numSamples);
                std::copy(side, side + numSamples, sideBuf.getWritePointer(0));
                processStandard(sideBuf);
                std::copy(sideBuf.getReadPointer(0), sideBuf.getReadPointer(0) + numSamples, side);
            }
        );
    } else {
        // Standard stereo/mono processing
        processStandard(buffer);
    }
}

void EQProcessor::processStandard(juce::AudioBuffer<float>& buffer) {
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    
    if (numChannels < 1) return;
    
    // Create a working buffer for each band's processing
    juce::AudioBuffer<float> bandBuffer(numChannels, numSamples);
    
    // Process each enabled band
    for (int band = 0; band < numBands; ++band) {
        if (!bandEnabled[static_cast<size_t>(band)]) continue;
        
        // Copy current buffer state to band buffer
        bandBuffer.makeCopyOf(buffer, true);
        
        auto& filter = filters[static_cast<size_t>(band)];
        auto& smoother = smoothers[static_cast<size_t>(band)];
        
        // Get channel pointers for band buffer
        float* leftChannel = bandBuffer.getWritePointer(0);
        float* rightChannel = numChannels > 1 ? bandBuffer.getWritePointer(1) : nullptr;
        
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
        
        // Apply per-band dynamics processing
        auto& dynamics = bandDynamics[static_cast<size_t>(band)];
        dynamics.updateFromParameters();
        dynamics.process(bandBuffer);
        
        // Mix band output back into main buffer (additive mixing for multiband)
        for (int ch = 0; ch < numChannels; ++ch) {
            const float* bandData = bandBuffer.getReadPointer(ch);
            float* mainData = buffer.getWritePointer(ch);
            for (int i = 0; i < numSamples; ++i) {
                mainData[i] += (bandData[i] - mainData[i]) * 0.5f; // Blend
            }
        }
    }
}

void EQProcessor::setBandParameters(int bandIndex, FilterType type, float freq, float q, float gain, bool enabled) {
    if (bandIndex < 0 || bandIndex >= numBands) return;
    
    juce::ScopedLock sl(lock);
    
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
    
    juce::ScopedLock sl(lock);
    bandEnabled[static_cast<size_t>(bandIndex)] = enabled;
}

EQProcessor::BandParams EQProcessor::getBandParameters(int bandIndex) const {
    if (bandIndex < 0 || bandIndex >= numBands) {
        return { FilterType::Peak, 1000.0f, 0.707f, 0.0f, false };
    }

    // Read directly from APVTS parameters (stable values, not smoothed filter state)
    const auto& ptrs = paramPtrs[static_cast<size_t>(bandIndex)];
    if (ptrs.frequency && ptrs.q && ptrs.gain && ptrs.type && ptrs.enabled) {
        return {
            static_cast<FilterType>(static_cast<int>(ptrs.type->load())),
            ptrs.frequency->load(),
            ptrs.q->load(),
            ptrs.gain->load(),
            ptrs.enabled->load() > 0.5f
        };
    }

    // Fallback to filter state if params not connected
    juce::ScopedLock sl(lock);
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
        const auto& ptrs = paramPtrs[static_cast<size_t>(i)];
        if (ptrs.enabled && ptrs.enabled->load() > 0.5f) {
            // Use static calculation from APVTS params (stable, no smoothing)
            totalMagnitude *= BiquadFilter::calcMagnitudeFromParams(
                static_cast<FilterType>(static_cast<int>(ptrs.type->load())),
                ptrs.frequency->load(),
                ptrs.q->load(),
                ptrs.gain->load(),
                currentSampleRate,
                frequency
            );
        }
    }

    return totalMagnitude;
}

float EQProcessor::getBandMagnitudeAtFrequency(int bandIndex, float frequency) const {
    if (bandIndex < 0 || bandIndex >= numBands) return 1.0f;

    const auto& ptrs = paramPtrs[static_cast<size_t>(bandIndex)];
    if (!ptrs.enabled || ptrs.enabled->load() <= 0.5f) return 1.0f;

    // Use static calculation from APVTS params (stable, no smoothing)
    return BiquadFilter::calcMagnitudeFromParams(
        static_cast<FilterType>(static_cast<int>(ptrs.type->load())),
        ptrs.frequency->load(),
        ptrs.q->load(),
        ptrs.gain->load(),
        currentSampleRate,
        frequency
    );
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
        
        // Connect per-band dynamics
        bandDynamics[static_cast<size_t>(i)].connectToParameters(apvts, i);
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

void EQProcessor::setMidSideMode(bool enabled) {
    midSideMode = enabled;
}

void EQProcessor::setLinearPhaseMode(bool enabled) {
    linearPhaseMode = enabled;
    if (enabled && prepared) {
        if (!linearPhaseEQ) {
            linearPhaseEQ = std::make_unique<LinearPhaseEQ>();
        }
        linearPhaseEQ->prepare(currentSampleRate, 512);  // Default block size
    }
}

void EQProcessor::setDynamicEQMode(bool enabled) {
    dynamicEQMode = enabled;
    if (enabled && prepared) {
        if (!dynamicEQ) {
            dynamicEQ = std::make_unique<DynamicEQProcessor>();
        }
        dynamicEQ->prepare(currentSampleRate, 512);  // Default block size
    }
}

float EQProcessor::getBandGainReduction(int bandIndex) const {
    if (bandIndex < 0 || bandIndex >= numBands) return 0.0f;
    return bandDynamics[static_cast<size_t>(bandIndex)].getGainReduction();
}

int EQProcessor::getLatency() const {
    if (linearPhaseMode && linearPhaseEQ) {
        return linearPhaseEQ->getLatency();
    }
    return 0;
}

} // namespace SeshEQ
