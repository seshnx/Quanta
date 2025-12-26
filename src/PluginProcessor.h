#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "dsp/EQProcessor.h"
#include "dsp/Compressor.h"
#include "dsp/Gate.h"
#include "dsp/Limiter.h"
#include "utils/Parameters.h"
#include "utils/SmoothValue.h"
#include "utils/FFTProcessor.h"
#include "utils/PresetManager.h"

namespace SeshEQ {

/**
 * @brief Main audio processor for SeshNx Quanta plugin
 * 
 * Signal flow:
 * Input -> Input Gain -> Multiband Dynamic EQ (8 bands with per-band dynamics) -> True Peak Limiter -> Output Gain -> Dry/Wet -> Output
 */
class PluginProcessor : public juce::AudioProcessor,
                        public juce::AudioProcessorValueTreeState::Listener {
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
    // ParameterListener for advanced features
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    //==============================================================================
    // Public accessors for editor
    
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    
    // Get DSP processors for visualization
    const EQProcessor& getEQProcessor() const { return eqProcessor; }
    
    // Get gain reduction values for metering
    float getCompressorGainReduction() const { return compressor.getGainReduction(); }
    float getGateGainReduction() const { return gate.getGainReduction(); }
    float getLimiterGainReduction() const { return limiter.getGainReduction(); }
    float getBandGainReduction(int bandIndex) const { return eqProcessor.getBandGainReduction(bandIndex); }
    float getTruePeak() const { return limiter.getTruePeak(); }
    
    // Get input/output levels for metering
    float getInputLevel() const { return inputLevelDb.load(); }
    float getOutputLevel() const { return outputLevelDb.load(); }
    
    // Get FFT processors for spectrum display
    FFTProcessor& getPreFFT() { return fftProcessor.getPreFFT(); }
    FFTProcessor& getPostFFT() { return fftProcessor.getPostFFT(); }

    // Preset manager access
    PresetManager& getPresetManager() { return presetManager; }

    // Get latency in samples
    int getLatencySamples() const;

private:
    // Parameter tree state
    juce::AudioProcessorValueTreeState apvts;

    // Preset manager
    PresetManager presetManager { apvts };

    // DSP processors
    EQProcessor eqProcessor;
    Compressor compressor;
    Gate gate;
    Limiter limiter;
    
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
    
    // Level metering
    std::atomic<float> inputLevelDb { -100.0f };
    std::atomic<float> outputLevelDb { -100.0f };
    
    // Dry buffer for wet/dry mix
    juce::AudioBuffer<float> dryBuffer;
    
    // Sample rate
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

    // Global oversampling
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;
    std::atomic<float>* oversamplingParam = nullptr;
    int currentOversamplingFactor = 1;  // 1, 2, 4, or 8

    // Helper to update oversampling factor
    void updateOversamplingFactor();
    int getOversamplingLatency() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};

} // namespace SeshEQ
