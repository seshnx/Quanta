#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace SeshEQ {

PluginProcessor::PluginProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "SeshNxQuanta", ParameterLayout::create())
{
    // Get parameter pointers
    inputGainParam = apvts.getRawParameterValue(ParamIDs::inputGain);
    outputGainParam = apvts.getRawParameterValue(ParamIDs::outputGain);
    dryWetParam = apvts.getRawParameterValue(ParamIDs::dryWet);
    bypassParam = apvts.getRawParameterValue(ParamIDs::bypass);
    oversamplingParam = apvts.getRawParameterValue(ParamIDs::oversamplingFactor);

    // Connect DSP processors to parameters
    eqProcessor.connectToParameters(apvts);
    compressor.connectToParameters(apvts);
    gate.connectToParameters(apvts);
    limiter.connectToParameters(apvts);

    // Connect advanced feature parameters
    apvts.addParameterListener(ParamIDs::midSideMode, this);
    apvts.addParameterListener(ParamIDs::linearPhaseMode, this);
    apvts.addParameterListener(ParamIDs::dynamicEQMode, this);
    apvts.addParameterListener(ParamIDs::oversamplingFactor, this);
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
    // Return latency for linear phase mode
    const int eqLatency = eqProcessor.getLatency();
    return eqLatency > 0 ? eqLatency / currentSampleRate : 0.0;
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
    currentBlockSize = samplesPerBlock;

    // Initialize global oversampling
    // Default to 1x (no oversampling) - order 0
    // The oversampling factor is updated via parameter listener
    updateOversamplingFactor();

    // Prepare DSP processors at the oversampled rate
    const double oversampledRate = sampleRate * currentOversamplingFactor;
    const int oversampledBlockSize = samplesPerBlock * currentOversamplingFactor;

    eqProcessor.prepare(oversampledRate, oversampledBlockSize);
    compressor.prepare(oversampledRate, oversampledBlockSize);
    gate.prepare(oversampledRate, oversampledBlockSize);
    limiter.prepare(oversampledRate, oversampledBlockSize);

    // Prepare FFT analyzer (at original rate for display)
    fftProcessor.prepare(sampleRate);

    // Prepare gain smoothers (at original rate - applied before/after oversampling)
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

    // Report latency to host
    setLatencySamples(getOversamplingLatency() + eqProcessor.getLatency());
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

    // Apply input gain (before oversampling)
    for (int i = 0; i < numSamples; ++i) {
        const float gain = inputGainSmoother.getNextGain();
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            buffer.getWritePointer(ch)[i] *= gain;
        }
    }

    // Push pre-EQ samples to FFT analyzer
    fftProcessor.pushPreSamples(buffer);

    // Process with oversampling if enabled
    if (oversampling && currentOversamplingFactor > 1) {
        // Upsample
        juce::dsp::AudioBlock<float> block(buffer);
        auto oversampledBlock = oversampling->processSamplesUp(block);

        // Create temporary buffer for oversampled processing
        juce::AudioBuffer<float> oversampledBuffer(
            static_cast<int>(oversampledBlock.getNumChannels()),
            static_cast<int>(oversampledBlock.getNumSamples()));

        // Copy oversampled data to buffer
        for (size_t ch = 0; ch < oversampledBlock.getNumChannels(); ++ch) {
            std::copy(oversampledBlock.getChannelPointer(ch),
                      oversampledBlock.getChannelPointer(ch) + oversampledBlock.getNumSamples(),
                      oversampledBuffer.getWritePointer(static_cast<int>(ch)));
        }

        // Process EQ at oversampled rate
        eqProcessor.process(oversampledBuffer);

        // Process dynamics at oversampled rate
        compressor.process(oversampledBuffer);
        gate.process(oversampledBuffer);

        // True Peak Limiter at oversampled rate
        limiter.process(oversampledBuffer);

        // Copy back to oversampled block
        for (size_t ch = 0; ch < oversampledBlock.getNumChannels(); ++ch) {
            std::copy(oversampledBuffer.getReadPointer(static_cast<int>(ch)),
                      oversampledBuffer.getReadPointer(static_cast<int>(ch)) + oversampledBlock.getNumSamples(),
                      oversampledBlock.getChannelPointer(ch));
        }

        // Downsample
        oversampling->processSamplesDown(block);
    } else {
        // No oversampling - process at original rate
        eqProcessor.process(buffer);
        compressor.process(buffer);
        gate.process(buffer);
        limiter.process(buffer);
    }

    // Apply output gain (after oversampling)
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

void PluginProcessor::parameterChanged(const juce::String& parameterID, float newValue) {
    using namespace ParamIDs;

    if (parameterID == midSideMode) {
        eqProcessor.setMidSideMode(newValue > 0.5f);
    } else if (parameterID == linearPhaseMode) {
        eqProcessor.setLinearPhaseMode(newValue > 0.5f);
        // Update latency when linear phase mode changes
        setLatencySamples(getOversamplingLatency() + eqProcessor.getLatency());
    } else if (parameterID == dynamicEQMode) {
        eqProcessor.setDynamicEQMode(newValue > 0.5f);
    } else if (parameterID == oversamplingFactor) {
        // Oversampling factor changed - need to reinitialize
        updateOversamplingFactor();
        // Re-prepare DSP at new oversampled rate
        const double oversampledRate = currentSampleRate * currentOversamplingFactor;
        const int oversampledBlockSize = currentBlockSize * currentOversamplingFactor;
        eqProcessor.prepare(oversampledRate, oversampledBlockSize);
        compressor.prepare(oversampledRate, oversampledBlockSize);
        gate.prepare(oversampledRate, oversampledBlockSize);
        limiter.prepare(oversampledRate, oversampledBlockSize);
        // Update latency
        setLatencySamples(getOversamplingLatency() + eqProcessor.getLatency());
    }
}

void PluginProcessor::updateOversamplingFactor() {
    // Get the oversampling factor from parameter (0=1x, 1=2x, 2=4x, 3=8x)
    int factorIndex = 0;
    if (oversamplingParam) {
        factorIndex = static_cast<int>(oversamplingParam->load());
    }

    // Convert index to actual factor
    switch (factorIndex) {
        case 0: currentOversamplingFactor = 1; break;
        case 1: currentOversamplingFactor = 2; break;
        case 2: currentOversamplingFactor = 4; break;
        case 3: currentOversamplingFactor = 8; break;
        default: currentOversamplingFactor = 1; break;
    }

    // Create or recreate oversampling object
    if (currentOversamplingFactor > 1) {
        // Calculate order: log2(factor)
        size_t order = 0;
        if (currentOversamplingFactor == 2) order = 1;
        else if (currentOversamplingFactor == 4) order = 2;
        else if (currentOversamplingFactor == 8) order = 3;

        oversampling = std::make_unique<juce::dsp::Oversampling<float>>(
            2,  // numChannels (stereo)
            order,
            juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
            true  // isMaxQuality
        );
        oversampling->initProcessing(static_cast<size_t>(currentBlockSize));
    } else {
        oversampling.reset();
    }
}

int PluginProcessor::getOversamplingLatency() const {
    if (oversampling && currentOversamplingFactor > 1) {
        return static_cast<int>(oversampling->getLatencyInSamples());
    }
    return 0;
}

int PluginProcessor::getLatencySamples() const {
    return getOversamplingLatency() + eqProcessor.getLatency();
}

} // namespace SeshEQ

//==============================================================================
// Entry point for plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new SeshEQ::PluginProcessor();
}
