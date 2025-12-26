#include "Limiter.h"
#include "utils/Parameters.h"
#include <juce_dsp/juce_dsp.h>

namespace SeshEQ {

Limiter::~Limiter() = default;

void Limiter::prepare(double newSampleRate, int samplesPerBlock) {
    sampleRate = newSampleRate;
    maxBlockSize = samplesPerBlock;
    
    updateOversampling();
    updateCoefficients();
    reset();
}

void Limiter::reset() {
    currentGain = 1.0f;
    gainReductionDb.store(0.0f);
    truePeakDb.store(-100.0f);
    if (oversampler) {
        oversampler->reset();
    }
}

void Limiter::setThreshold(float dB) {
    thresholdDb = dB;
}

void Limiter::setCeiling(float dB) {
    ceilingDb = dB;
}

void Limiter::setRelease(float ms) {
    releaseMs = ms;
    updateCoefficients();
}

void Limiter::setEnabled(bool newEnabled) {
    enabled = newEnabled;
    if (!enabled) {
        reset();
    }
}

void Limiter::setOversamplingFactor(int factor) {
    // Clamp to valid values: 1, 2, 4, or 8
    if (factor != 1 && factor != 2 && factor != 4 && factor != 8) {
        factor = 4; // Default to 4x
    }
    oversamplingFactor = factor;
    updateOversampling();
}

void Limiter::updateCoefficients() {
    if (releaseMs > 0.0f) {
        const double releaseSeconds = releaseMs / 1000.0;
        releaseCoef = static_cast<float>(std::exp(-1.0 / (releaseSeconds * sampleRate * oversamplingFactor)));
    } else {
        releaseCoef = 0.0f;
    }
}

void Limiter::updateOversampling() {
    if (oversamplingFactor > 1) {
        oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
            2, // 2 channels (stereo)
            oversamplingFactor == 8 ? 3 : (oversamplingFactor == 4 ? 2 : 1),
            juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
            false, // don't use steep filter
            false  // don't use zero phase
        );
        oversampler->initProcessing(static_cast<size_t>(maxBlockSize));
    } else {
        oversampler.reset();
    }
    updateCoefficients();
}

int Limiter::getLatency() const {
    if (oversampler) {
        return static_cast<int>(oversampler->getLatencyInSamples());
    }
    return 0;
}

float Limiter::detectTruePeak(const float* samples, int numSamples, int numChannels) {
    float maxPeak = 0.0f;
    
    // Simple inter-sample peak detection using linear interpolation
    // For more accurate detection, we'd use oversampling (which we do in process)
    for (int i = 0; i < numSamples - 1; ++i) {
        for (int ch = 0; ch < numChannels; ++ch) {
            const int idx = ch * numSamples + i;
            const float s1 = std::abs(samples[idx]);
            const float s2 = std::abs(samples[idx + 1]);
            
            // Check for inter-sample peaks (simple linear interpolation)
            const float peak = std::max(s1, s2);
            // More sophisticated: check if there's a peak between samples
            // For now, we rely on oversampling for accuracy
            maxPeak = std::max(maxPeak, peak);
        }
    }
    
    return maxPeak;
}

void Limiter::process(juce::AudioBuffer<float>& buffer) {
    if (!enabled) {
        gainReductionDb.store(0.0f);
        truePeakDb.store(-100.0f);
        return;
    }
    
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    
    if (numChannels < 1 || numSamples < 1) return;
    
    const float thresholdLinear = dBUtils::dbToLinear(thresholdDb);
    const float ceilingLinear = dBUtils::dbToLinear(ceilingDb);
    float maxGainReduction = 0.0f;
    float maxTruePeak = 0.0f;
    
    // Apply oversampling if enabled
    juce::dsp::AudioBlock<float> inputBlock(buffer);
    juce::dsp::AudioBlock<float> processBlock = inputBlock;
    juce::HeapBlock<char> oversampledData;
    juce::dsp::AudioBlock<float> oversampledBlock;
    
    if (oversampler && oversamplingFactor > 1) {
        processBlock = oversampler->processSamplesUp(inputBlock);
    }
    
    const int processNumSamples = static_cast<int>(processBlock.getNumSamples());
    
    for (int i = 0; i < processNumSamples; ++i) {
        // Find True Peak across all channels
        float peak = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch) {
            const float sample = std::abs(processBlock.getSample(ch, i));
            peak = std::max(peak, sample);
        }
        
        maxTruePeak = std::max(maxTruePeak, peak);
        
        // Calculate required gain reduction based on threshold
        float targetGain = 1.0f;
        if (peak > thresholdLinear) {
            // Apply limiting above threshold
            if (peak > ceilingLinear) {
                targetGain = ceilingLinear / peak;
            } else {
                // Soft knee region between threshold and ceiling
                const float excess = peak - thresholdLinear;
                const float kneeRange = ceilingLinear - thresholdLinear;
                const float kneeFactor = std::min(excess / kneeRange, 1.0f);
                targetGain = 1.0f - (kneeFactor * (1.0f - ceilingLinear / peak));
            }
        }
        
        // Apply envelope (instant attack, smooth release)
        if (targetGain < currentGain) {
            // Instant attack - immediately reduce gain
            currentGain = targetGain;
        } else {
            // Smooth release
            currentGain = releaseCoef * currentGain + (1.0f - releaseCoef) * targetGain;
        }
        
        // Track maximum gain reduction
        const float gr = dBUtils::linearToDb(currentGain);
        maxGainReduction = std::min(maxGainReduction, gr);
        
        // Apply gain to all channels
        for (int ch = 0; ch < numChannels; ++ch) {
            processBlock.getChannelPointer(ch)[i] *= currentGain;
            
            // Hard clip as safety (should rarely be needed)
            processBlock.getChannelPointer(ch)[i] = std::clamp(
                processBlock.getChannelPointer(ch)[i], -ceilingLinear, ceilingLinear);
        }
    }
    
    // Downsample if oversampled
    if (oversampler && oversamplingFactor > 1) {
        oversampler->processSamplesDown(inputBlock);
    }
    
    gainReductionDb.store(maxGainReduction);
    truePeakDb.store(dBUtils::linearToDb(maxTruePeak));
}

void Limiter::connectToParameters(juce::AudioProcessorValueTreeState& apvts) {
    using namespace ParamIDs;
    
    thresholdParam = apvts.getRawParameterValue(limiterThreshold);
    ceilingParam = apvts.getRawParameterValue(limiterCeiling);
    releaseParam = apvts.getRawParameterValue(limiterRelease);
    enabledParam = apvts.getRawParameterValue(limiterEnable);
}

void Limiter::updateFromParameters() {
    if (thresholdParam) setThreshold(thresholdParam->load());
    if (ceilingParam) setCeiling(ceilingParam->load());
    if (releaseParam) setRelease(releaseParam->load());
    if (enabledParam) setEnabled(enabledParam->load() > 0.5f);
}

} // namespace SeshEQ
