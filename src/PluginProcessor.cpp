#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace SeshEQ {

PluginProcessor::PluginProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "SeshEQ", ParameterLayout::create())
{
    // Get parameter pointers
    inputGainParam = apvts.getRawParameterValue(ParamIDs::inputGain);
    outputGainParam = apvts.getRawParameterValue(ParamIDs::outputGain);
    dryWetParam = apvts.getRawParameterValue(ParamIDs::dryWet);
    bypassParam = apvts.getRawParameterValue(ParamIDs::bypass);
    
    // Advanced DSP parameters
    processingModeParam = apvts.getRawParameterValue(ParamIDs::processingMode);
    oversamplingParam = apvts.getRawParameterValue(ParamIDs::oversampling);
    scFilterModeParam = apvts.getRawParameterValue(ParamIDs::scFilterMode);
    scFilterFreqParam = apvts.getRawParameterValue(ParamIDs::scFilterFreq);
    scFilterQParam = apvts.getRawParameterValue(ParamIDs::scFilterQ);
    scFilterListenParam = apvts.getRawParameterValue(ParamIDs::scFilterListen);
    
    // Connect DSP processors to parameters
    eqProcessor.connectToParameters(apvts);
    compressor.connectToParameters(apvts);
    gate.connectToParameters(apvts);
    limiter.connectToParameters(apvts);
}

PluginProcessor::~PluginProcessor() {
}

//==============================================================================
const juce::String PluginProcessor::getName() const {
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const {
    return false;
}

bool PluginProcessor::producesMidi() const {
    return false;
}

bool PluginProcessor::isMidiEffect() const {
    return false;
}

double PluginProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int PluginProcessor::getNumPrograms() {
    return 1;
}

int PluginProcessor::getCurrentProgram() {
    return 0;
}

void PluginProcessor::setCurrentProgram(int /*index*/) {
}

const juce::String PluginProcessor::getProgramName(int /*index*/) {
    return {};
}

void PluginProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/) {
}

//==============================================================================
void PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    currentSampleRate = sampleRate;
    
    // Prepare DSP processors
    eqProcessor.prepare(sampleRate, samplesPerBlock);
    compressor.prepare(sampleRate, samplesPerBlock);
    gate.prepare(sampleRate, samplesPerBlock);
    limiter.prepare(sampleRate, samplesPerBlock);
    
    // Prepare advanced DSP
    sidechainFilter.prepare(sampleRate);
    
    // Prepare oversampler (2 channels, with current factor)
    auto osMode = oversamplingParam ? static_cast<OversamplingFactor>(static_cast<int>(oversamplingParam->load()))
                                     : OversamplingFactor::None;
    oversampler.prepare(2, samplesPerBlock, osMode);
    
    // Prepare FFT analyzer
    fftProcessor.prepare(sampleRate);
    
    // Prepare gain smoothers
    inputGainSmoother.prepare(sampleRate, 20.0);
    outputGainSmoother.prepare(sampleRate, 20.0);
    dryWetSmoother.prepare(sampleRate, 20.0);
    
    // Prepare dry buffer for wet/dry mix
    dryBuffer.setSize(2, samplesPerBlock);
    
    // Initialize gain smoothers with current values
    if (inputGainParam)
        inputGainSmoother.setTargetDb(inputGainParam->load());
    if (outputGainParam)
        outputGainSmoother.setTargetDb(outputGainParam->load());
    if (dryWetParam)
        dryWetSmoother.setTargetValue(dryWetParam->load() / 100.0f);
}

void PluginProcessor::releaseResources() {
    // Release any resources
}

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    // Support mono and stereo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // Input must match output
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                    juce::MidiBuffer& /*midiMessages*/) {
    juce::ScopedNoDenormals noDenormals;
    
    const int totalNumInputChannels = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    // Clear unused output channels
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, numSamples);

    // Check bypass
    const bool bypassed = bypassParam && bypassParam->load() > 0.5f;
    if (bypassed) {
        return;
    }
    
    // Update parameters from APVTS
    eqProcessor.updateFromParameters();
    compressor.updateFromParameters();
    gate.updateFromParameters();
    limiter.updateFromParameters();
    
    // Update advanced DSP parameters
    if (processingModeParam)
        midSideProcessor.setMode(static_cast<ProcessingMode>(static_cast<int>(processingModeParam->load())));
    
    if (scFilterModeParam)
        sidechainFilter.setMode(static_cast<SidechainFilterMode>(static_cast<int>(scFilterModeParam->load())));
    if (scFilterFreqParam)
        sidechainFilter.setFrequency(scFilterFreqParam->load());
    if (scFilterQParam)
        sidechainFilter.setQ(scFilterQParam->load());
    
    // Update gain targets
    if (inputGainParam)
        inputGainSmoother.setTargetDb(inputGainParam->load());
    if (outputGainParam)
        outputGainSmoother.setTargetDb(outputGainParam->load());
    if (dryWetParam)
        dryWetSmoother.setTargetValue(dryWetParam->load() / 100.0f);
    
    // Calculate input level for metering
    float maxInputLevel = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        maxInputLevel = std::max(maxInputLevel, buffer.getMagnitude(ch, 0, numSamples));
    }
    inputLevelDb.store(dBUtils::linearToDb(maxInputLevel));
    
    // Store dry signal for wet/dry mix
    const float currentWet = dryWetSmoother.getCurrentValue();
    const bool needsMix = currentWet < 0.99f || dryWetSmoother.isSmoothing();
    
    if (needsMix) {
        dryBuffer.makeCopyOf(buffer, true);
    }
    
    // Apply input gain
    for (int i = 0; i < numSamples; ++i) {
        const float gain = inputGainSmoother.getNextGain();
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            buffer.getWritePointer(ch)[i] *= gain;
        }
    }
    
    // Push pre-EQ samples to FFT analyzer
    fftProcessor.pushPreSamples(buffer);
    
    // Apply Mid/Side encoding if enabled
    midSideProcessor.prepareBuffer(buffer);
    
    // Process EQ
    eqProcessor.process(buffer);
    
    // Process dynamics (in order: compressor -> gate -> limiter)
    compressor.process(buffer);
    gate.process(buffer);
    limiter.process(buffer);
    
    // Apply Mid/Side decoding if enabled
    midSideProcessor.finalizeBuffer(buffer);
    
    // Apply output gain
    for (int i = 0; i < numSamples; ++i) {
        const float gain = outputGainSmoother.getNextGain();
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            buffer.getWritePointer(ch)[i] *= gain;
        }
    }
    
    // Apply wet/dry mix
    if (needsMix) {
        for (int i = 0; i < numSamples; ++i) {
            const float wet = dryWetSmoother.getNextValue();
            const float dry = 1.0f - wet;
            
            for (int ch = 0; ch < std::min(buffer.getNumChannels(), dryBuffer.getNumChannels()); ++ch) {
                float* wetData = buffer.getWritePointer(ch);
                const float* dryData = dryBuffer.getReadPointer(ch);
                wetData[i] = wetData[i] * wet + dryData[i] * dry;
            }
        }
    }
    
    // Push post-processing samples to FFT analyzer
    fftProcessor.pushPostSamples(buffer);
    
    // Calculate output level for metering
    float maxOutputLevel = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        maxOutputLevel = std::max(maxOutputLevel, buffer.getMagnitude(ch, 0, numSamples));
    }
    outputLevelDb.store(dBUtils::linearToDb(maxOutputLevel));
}

//==============================================================================
bool PluginProcessor::hasEditor() const {
    return true;
}

juce::AudioProcessorEditor* PluginProcessor::createEditor() {
    return new PluginEditor(*this);
}

//==============================================================================
void PluginProcessor::getStateInformation(juce::MemoryBlock& destData) {
    // Save plugin state
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void PluginProcessor::setStateInformation(const void* data, int sizeInBytes) {
    // Restore plugin state
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState != nullptr) {
        if (xmlState->hasTagName(apvts.state.getType())) {
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

} // namespace SeshEQ

//==============================================================================
// Entry point for plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new SeshEQ::PluginProcessor();
}
