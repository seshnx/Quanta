#include "Limiter.h"
#include "utils/Parameters.h"

namespace SeshEQ {

void Limiter::prepare(double newSampleRate, int /*samplesPerBlock*/) {
    sampleRate = newSampleRate;
    updateCoefficients();
    reset();
}

void Limiter::reset() {
    currentGain = 1.0f;
    gainReductionDb = 0.0f;
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

void Limiter::updateCoefficients() {
    if (releaseMs > 0.0f) {
        const double releaseSeconds = releaseMs / 1000.0;
        releaseCoef = static_cast<float>(std::exp(-1.0 / (releaseSeconds * sampleRate)));
    } else {
        releaseCoef = 0.0f;
    }
}

void Limiter::process(juce::AudioBuffer<float>& buffer) {
    if (!enabled) {
        gainReductionDb = 0.0f;
        return;
    }
    
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    
    if (numChannels < 1 || numSamples < 1) return;
    
    const float ceilingLinear = dBUtils::dbToLinear(ceilingDb);
    float maxGainReduction = 0.0f;
    
    for (int i = 0; i < numSamples; ++i) {
        // Find peak across all channels
        float peak = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch) {
            const float sample = std::abs(buffer.getSample(ch, i));
            peak = std::max(peak, sample);
        }
        
        // Calculate required gain reduction
        float targetGain = 1.0f;
        if (peak > ceilingLinear) {
            targetGain = ceilingLinear / peak;
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
            float* channelData = buffer.getWritePointer(ch);
            channelData[i] *= currentGain;
            
            // Hard clip as safety (should rarely be needed)
            channelData[i] = std::clamp(channelData[i], -ceilingLinear, ceilingLinear);
        }
    }
    
    gainReductionDb = maxGainReduction;
}

void Limiter::connectToParameters(juce::AudioProcessorValueTreeState& apvts) {
    using namespace ParamIDs;
    
    ceilingParam = apvts.getRawParameterValue(limiterCeiling);
    releaseParam = apvts.getRawParameterValue(limiterRelease);
    enabledParam = apvts.getRawParameterValue(limiterEnable);
}

void Limiter::updateFromParameters() {
    if (ceilingParam) setCeiling(ceilingParam->load());
    if (releaseParam) setRelease(releaseParam->load());
    if (enabledParam) setEnabled(enabledParam->load() > 0.5f);
}

} // namespace SeshEQ
