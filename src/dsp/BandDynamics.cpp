#include "BandDynamics.h"
#include "utils/Parameters.h"

namespace SeshEQ {

void BandDynamics::prepare(double newSampleRate, int samplesPerBlock) {
    sampleRate = newSampleRate;
    compressor.prepare(newSampleRate, samplesPerBlock);
    reset();
}

void BandDynamics::reset() {
    compressor.reset();
    gainReductionDb.store(0.0f);
}

void BandDynamics::setThreshold(float dB) {
    thresholdDb = dB;
    compressor.setThreshold(dB);
}

void BandDynamics::setRatio(float newRatio) {
    ratio = newRatio;
    compressor.setRatio(newRatio);
}

void BandDynamics::setAttack(float ms) {
    attackMs = ms;
    compressor.setAttack(ms);
}

void BandDynamics::setRelease(float ms) {
    releaseMs = ms;
    compressor.setRelease(ms);
}

void BandDynamics::setKnee(float dB) {
    kneeDb = dB;
    compressor.setKnee(dB);
}

void BandDynamics::setEnabled(bool newEnabled) {
    enabled = newEnabled;
    compressor.setEnabled(newEnabled);
    if (!enabled) {
        reset();
    }
}

void BandDynamics::process(juce::AudioBuffer<float>& buffer) {
    if (!enabled) {
        gainReductionDb.store(0.0f);
        return;
    }
    
    // Process through compressor
    compressor.process(buffer);
    
    // Update gain reduction for metering
    gainReductionDb.store(compressor.getGainReduction());
}

void BandDynamics::connectToParameters(juce::AudioProcessorValueTreeState& apvts, int bandIndex) {
    using namespace ParamIDs;
    
    thresholdParam = apvts.getRawParameterValue(getBandParamID(bandIndex, bandDynThreshold));
    ratioParam = apvts.getRawParameterValue(getBandParamID(bandIndex, bandDynRatio));
    attackParam = apvts.getRawParameterValue(getBandParamID(bandIndex, bandDynAttack));
    releaseParam = apvts.getRawParameterValue(getBandParamID(bandIndex, bandDynRelease));
    kneeParam = apvts.getRawParameterValue(getBandParamID(bandIndex, bandDynKnee));
    enabledParam = apvts.getRawParameterValue(getBandParamID(bandIndex, bandDynEnable));
}

void BandDynamics::updateFromParameters() {
    if (thresholdParam) setThreshold(thresholdParam->load());
    if (ratioParam) setRatio(ratioParam->load());
    if (attackParam) setAttack(attackParam->load());
    if (releaseParam) setRelease(releaseParam->load());
    if (kneeParam) setKnee(kneeParam->load());
    if (enabledParam) setEnabled(enabledParam->load() > 0.5f);
}

} // namespace SeshEQ

