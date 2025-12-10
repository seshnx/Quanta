#include "Gate.h"
#include "utils/Parameters.h"

namespace SeshEQ {

void Gate::prepare(double newSampleRate, int /*samplesPerBlock*/) {
    sampleRate = newSampleRate;
    levelDetector.prepare(sampleRate);
    levelDetector.setAttackTime(0.1f);  // Fast detection
    levelDetector.setReleaseTime(50.0f);
    updateCoefficients();
    reset();
}

void Gate::reset() {
    levelDetector.reset();
    state = GateState::Closed;
    currentGain = 0.0f;
    holdCounter = 0;
    gainReductionDb = rangeDb;
}

void Gate::setThreshold(float dB) {
    thresholdDb = dB;
}

void Gate::setRatio(float newRatio) {
    ratio = std::max(1.0f, newRatio);
}

void Gate::setAttack(float ms) {
    attackMs = ms;
    updateCoefficients();
}

void Gate::setHold(float ms) {
    holdMs = ms;
    holdSamples = static_cast<int>((holdMs / 1000.0) * sampleRate);
}

void Gate::setRelease(float ms) {
    releaseMs = ms;
    updateCoefficients();
}

void Gate::setRange(float dB) {
    rangeDb = std::min(0.0f, dB);  // Must be negative or zero
}

void Gate::setEnabled(bool newEnabled) {
    enabled = newEnabled;
    if (!enabled) {
        reset();
    }
}

void Gate::updateCoefficients() {
    if (attackMs > 0.0f) {
        const double attackSeconds = attackMs / 1000.0;
        attackCoef = static_cast<float>(std::exp(-1.0 / (attackSeconds * sampleRate)));
    } else {
        attackCoef = 0.0f;
    }
    
    if (releaseMs > 0.0f) {
        const double releaseSeconds = releaseMs / 1000.0;
        releaseCoef = static_cast<float>(std::exp(-1.0 / (releaseSeconds * sampleRate)));
    } else {
        releaseCoef = 0.0f;
    }
    
    holdSamples = static_cast<int>((holdMs / 1000.0) * sampleRate);
}

void Gate::process(juce::AudioBuffer<float>& buffer) {
    if (!enabled) {
        gainReductionDb = 0.0f;
        return;
    }
    
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    
    if (numChannels < 1 || numSamples < 1) return;
    
    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;
    
    // Calculate target gain based on range
    const float closedGain = dBUtils::dbToLinear(rangeDb);
    float maxGainReduction = 0.0f;
    
    for (int i = 0; i < numSamples; ++i) {
        // Get input level
        float inputLevel;
        if (rightChannel) {
            inputLevel = levelDetector.processStereo(leftChannel[i], rightChannel[i]);
        } else {
            inputLevel = levelDetector.processSample(leftChannel[i]);
        }
        
        const float thresholdLinear = dBUtils::dbToLinear(thresholdDb);
        
        // Determine if we're above or below threshold
        const bool aboveThreshold = inputLevel > thresholdLinear;
        
        // State machine for gate
        float targetGain = closedGain;
        
        switch (state) {
            case GateState::Closed:
                if (aboveThreshold) {
                    state = GateState::Attack;
                }
                targetGain = closedGain;
                break;
                
            case GateState::Attack:
                targetGain = 1.0f;
                if (currentGain >= 0.99f) {
                    state = GateState::Open;
                    currentGain = 1.0f;
                }
                if (!aboveThreshold) {
                    state = GateState::Hold;
                    holdCounter = holdSamples;
                }
                break;
                
            case GateState::Open:
                targetGain = 1.0f;
                if (!aboveThreshold) {
                    state = GateState::Hold;
                    holdCounter = holdSamples;
                }
                break;
                
            case GateState::Hold:
                targetGain = 1.0f;
                if (aboveThreshold) {
                    state = GateState::Open;
                } else if (--holdCounter <= 0) {
                    state = GateState::Release;
                }
                break;
                
            case GateState::Release:
                targetGain = closedGain;
                if (aboveThreshold) {
                    state = GateState::Attack;
                } else if (currentGain <= closedGain + 0.001f) {
                    state = GateState::Closed;
                    currentGain = closedGain;
                }
                break;
        }
        
        // Smooth gain changes
        if (targetGain > currentGain) {
            // Attack (opening)
            currentGain = attackCoef * currentGain + (1.0f - attackCoef) * targetGain;
        } else {
            // Release (closing)
            currentGain = releaseCoef * currentGain + (1.0f - releaseCoef) * targetGain;
        }
        
        // Apply expansion ratio for soft gate behavior
        float finalGain = currentGain;
        if (ratio < 100.0f && currentGain < 1.0f) {
            // Apply expansion ratio
            const float expansionDb = dBUtils::linearToDb(currentGain);
            const float expandedDb = expansionDb / ratio;
            finalGain = std::max(closedGain, dBUtils::dbToLinear(expandedDb));
        }
        
        // Track gain reduction
        const float gr = dBUtils::linearToDb(finalGain);
        maxGainReduction = std::min(maxGainReduction, gr);
        
        // Apply gain
        leftChannel[i] *= finalGain;
        if (rightChannel) {
            rightChannel[i] *= finalGain;
        }
    }
    
    gainReductionDb = maxGainReduction;
}

void Gate::connectToParameters(juce::AudioProcessorValueTreeState& apvts) {
    using namespace ParamIDs;
    
    thresholdParam = apvts.getRawParameterValue(gateThreshold);
    ratioParam = apvts.getRawParameterValue(gateRatio);
    attackParam = apvts.getRawParameterValue(gateAttack);
    holdParam = apvts.getRawParameterValue(gateHold);
    releaseParam = apvts.getRawParameterValue(gateRelease);
    rangeParam = apvts.getRawParameterValue(gateRange);
    enabledParam = apvts.getRawParameterValue(gateEnable);
}

void Gate::updateFromParameters() {
    if (thresholdParam) setThreshold(thresholdParam->load());
    if (ratioParam) setRatio(ratioParam->load());
    if (attackParam) setAttack(attackParam->load());
    if (holdParam) setHold(holdParam->load());
    if (releaseParam) setRelease(releaseParam->load());
    if (rangeParam) setRange(rangeParam->load());
    if (enabledParam) setEnabled(enabledParam->load() > 0.5f);
}

} // namespace SeshEQ
