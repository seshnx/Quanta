#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "dsp/EQProcessor.h"
#include "dsp/Compressor.h"
#include "dsp/Gate.h"
#include "dsp/Limiter.h"
#include "dsp/MidSideProcessor.h"
#include "dsp/Oversampler.h"
#include "dsp/SidechainFilter.h"
#include "utils/Parameters.h"
#include "utils/SmoothValue.h"
#include "utils/FFTProcessor.h"
#include "presets/PresetManager.h"

namespace SeshEQ {

/**
 * @brief Main audio processor for SeshEQ plugin
 * 
 * Signal flow:
 * Input -> Input Gain -> EQ -> Compressor -> Gate -> Limiter -> Output Gain -> Dry/Wet -> Output
 */
class PluginProcessor : public juce::AudioProcessor {
public:
    PluginProcessor();
    ~PluginProcessor() override;

    //==============================================================================
    // AudioProcessor overrides
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    // Public accessors for editor
    
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    
    // Get DSP processors for visualization
    const EQProcessor& getEQProcessor() const { return eqProcessor; }
    
    // Get gain reduction values for metering
    float getCompressorGainReduction() const { return compressor.getGainReduction(); }
    float getGateGainReduction() const { return gate.getGainReduction(); }
    float getLimiterGainReduction() const { return limiter.getGainReduction(); }
    
    // Get input/output levels for metering
    float getInputLevel() const { return inputLevelDb.load(); }
    float getOutputLevel() const { return outputLevelDb.load(); }
    
    // Get FFT processors for spectrum display
    FFTProcessor& getPreFFT() { return fftProcessor.getPreFFT(); }
    FFTProcessor& getPostFFT() { return fftProcessor.getPostFFT(); }
    
    // Get preset manager
    PresetManager& getPresetManager() { return presetManager; }

private:
    // Parameter tree state
    juce::AudioProcessorValueTreeState apvts;
    
    // Preset manager (must be after apvts initialization)
    PresetManager presetManager { apvts };
    
    // DSP processors
    EQProcessor eqProcessor;
    Compressor compressor;
    Gate gate;
    Limiter limiter;
    
    // Advanced DSP
    MidSideProcessor midSideProcessor;
    Oversampler oversampler;
    SidechainFilter sidechainFilter;
    
    // FFT for spectrum analysis
    DualFFTProcessor fftProcessor;
    
    // Global parameters
    SmoothGain<float> inputGainSmoother;
    SmoothGain<float> outputGainSmoother;
    SmoothValue<float> dryWetSmoother { 1.0f };
    
    // Parameter pointers
    std::atomic<float>* inputGainParam = nullptr;
    std::atomic<float>* outputGainParam = nullptr;
    std::atomic<float>* dryWetParam = nullptr;
    std::atomic<float>* bypassParam = nullptr;
    
    // Advanced DSP parameters
    std::atomic<float>* processingModeParam = nullptr;
    std::atomic<float>* oversamplingParam = nullptr;
    std::atomic<float>* scFilterModeParam = nullptr;
    std::atomic<float>* scFilterFreqParam = nullptr;
    std::atomic<float>* scFilterQParam = nullptr;
    std::atomic<float>* scFilterListenParam = nullptr;
    
    // Level metering
    std::atomic<float> inputLevelDb { -100.0f };
    std::atomic<float> outputLevelDb { -100.0f };
    
    // Dry buffer for wet/dry mix
    juce::AudioBuffer<float> dryBuffer;
    
    // Sample rate
    double currentSampleRate = 44100.0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};

} // namespace SeshEQ
