#include "Compressor.h"
#include "utils/Parameters.h"

namespace SeshEQ {

void Compressor::prepare(double newSampleRate, int /*samplesPerBlock*/) {
    sampleRate = newSampleRate;
    levelDetector.prepare(sampleRate);
    makeupGain.prepare(sampleRate, 20.0);
    reset();
}

void Compressor::reset() {
    levelDetector.reset();
    gainReductionDb.store(0.0f);
}

void Compressor::setThreshold(float dB) {
    thresholdDb = dB;
}

void Compressor::setRatio(float newRatio) {
    // Allow ratios from 0.1 (expansion) to 20.0 (limiting)
    // Ratio < 1.0 = Expander mode (boost below threshold)
    // Ratio > 1.0 = Compressor mode (reduce above threshold)
    ratio = std::clamp(newRatio, 0.1f, 20.0f);
}

void Compressor::setAttack(float ms) {
    levelDetector.setAttackTime(ms);
}

void Compressor::setRelease(float ms) {
    levelDetector.setReleaseTime(ms);
}

void Compressor::setKnee(float dB) {
    kneeDb = std::max(0.0f, dB);
}

void Compressor::setMakeupGain(float dB) {
    makeupGainDb = dB;
    makeupGain.setTargetDb(dB);
}

void Compressor::setMix(float percent) {
    mix = std::clamp(percent, 0.0f, 100.0f) / 100.0f;
}

void Compressor::setDetectionMode(DetectionMode mode) {
    levelDetector.setMode(mode);
}

void Compressor::setEnabled(bool newEnabled) {
    enabled = newEnabled;
}

float Compressor::computeGain(float inputDb) const {
    // Soft knee dynamics curve
    // Supports both compression (ratio > 1) and expansion (ratio < 1)
    // Based on: https://www.musicdsp.org/en/latest/Effects/169-compressor.html

    const float halfKnee = kneeDb / 2.0f;
    const float kneeStart = thresholdDb - halfKnee;
    const float kneeEnd = thresholdDb + halfKnee;

    float outputDb;

    if (ratio >= 1.0f) {
        // COMPRESSION MODE (ratio >= 1.0)
        // Reduce gain for signals above threshold
        if (inputDb < kneeStart) {
            // Below knee - no compression
            outputDb = inputDb;
        } else if (inputDb > kneeEnd) {
            // Above knee - full compression
            outputDb = thresholdDb + (inputDb - thresholdDb) / ratio;
        } else {
            // In knee region - smooth transition
            const float kneeInput = inputDb - kneeStart;
            const float kneeRange = kneeDb;
            const float compressionRatio = 1.0f + (ratio - 1.0f) * (kneeInput / kneeRange);
            outputDb = kneeStart + kneeInput / compressionRatio;
        }
    } else {
        // EXPANSION MODE (ratio < 1.0)
        // Reduce gain for signals BELOW threshold (downward expansion)
        // This creates a gate-like effect with smooth transition
        if (inputDb > kneeEnd) {
            // Above knee - no expansion (pass through)
            outputDb = inputDb;
        } else if (inputDb < kneeStart) {
            // Below knee - full expansion
            // For expansion, we attenuate signals below threshold
            // The expansion ratio inverts: ratio 0.5 means 2:1 expansion
            const float expansionRatio = 1.0f / ratio;
            outputDb = thresholdDb - (thresholdDb - inputDb) * expansionRatio;
        } else {
            // In knee region - smooth transition
            const float kneeInput = kneeEnd - inputDb;
            const float kneeRange = kneeDb;
            const float t = kneeInput / kneeRange; // 0 at kneeEnd, 1 at kneeStart
            const float expansionRatio = 1.0f + (1.0f / ratio - 1.0f) * t;
            outputDb = inputDb - (thresholdDb - inputDb) * (expansionRatio - 1.0f) * t;
        }
    }

    // Return gain change (negative for reduction, positive for boost)
    return outputDb - inputDb;
}

void Compressor::process(juce::AudioBuffer<float>& buffer) {
    if (!enabled) {
        gainReductionDb.store(0.0f);
        return;
    }
    
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    
    if (numChannels < 1 || numSamples < 1) return;
    
    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;
    
    float maxGainReduction = 0.0f;
    
    for (int i = 0; i < numSamples; ++i) {
        // Get input level
        float inputLevel;
        if (rightChannel) {
            inputLevel = levelDetector.processStereo(leftChannel[i], rightChannel[i]);
        } else {
            inputLevel = levelDetector.processSample(leftChannel[i]);
        }
        
        // Convert to dB
        const float inputDb = dBUtils::linearToDb(inputLevel);
        
        // Compute gain reduction
        const float gr = computeGain(inputDb);
        maxGainReduction = std::min(maxGainReduction, gr);
        
        // Convert gain reduction to linear and apply makeup
        const float gainLinear = dBUtils::dbToLinear(gr) * makeupGain.getNextGain();
        
        // Apply gain with mix (parallel compression)
        const float dryGain = 1.0f - mix;
        const float wetGain = mix * gainLinear;
        
        leftChannel[i] = leftChannel[i] * dryGain + leftChannel[i] * wetGain;
        
        if (rightChannel) {
            rightChannel[i] = rightChannel[i] * dryGain + rightChannel[i] * wetGain;
        }
    }
    
    gainReductionDb.store(maxGainReduction);
}

void Compressor::connectToParameters(juce::AudioProcessorValueTreeState& apvts) {
    using namespace ParamIDs;
    
    thresholdParam = apvts.getRawParameterValue(compThreshold);
    ratioParam = apvts.getRawParameterValue(compRatio);
    attackParam = apvts.getRawParameterValue(compAttack);
    releaseParam = apvts.getRawParameterValue(compRelease);
    kneeParam = apvts.getRawParameterValue(compKnee);
    makeupParam = apvts.getRawParameterValue(compMakeup);
    mixParam = apvts.getRawParameterValue(compMix);
    enabledParam = apvts.getRawParameterValue(compEnable);
}

void Compressor::updateFromParameters() {
    if (thresholdParam) setThreshold(thresholdParam->load());
    if (ratioParam) setRatio(ratioParam->load());
    if (attackParam) setAttack(attackParam->load());
    if (releaseParam) setRelease(releaseParam->load());
    if (kneeParam) setKnee(kneeParam->load());
    if (makeupParam) setMakeupGain(makeupParam->load());
    if (mixParam) setMix(mixParam->load());
    if (enabledParam) setEnabled(enabledParam->load() > 0.5f);
}

} // namespace SeshEQ
