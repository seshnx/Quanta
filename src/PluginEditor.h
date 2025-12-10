#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

namespace SeshEQ {

/**
 * @brief Main editor/UI for SeshEQ plugin
 * 
 * Phase 1: Basic UI with parameter controls
 * Future phases will add spectrum analyzer, EQ curve display, etc.
 */
class PluginEditor : public juce::AudioProcessorEditor,
                      private juce::Timer {
public:
    explicit PluginEditor(PluginProcessor&);
    ~PluginEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    
    // Reference to our processor
    PluginProcessor& processorRef;
    
    //==============================================================================
    // Global controls
    juce::Slider inputGainSlider;
    juce::Slider outputGainSlider;
    juce::Slider dryWetSlider;
    juce::ToggleButton bypassButton { "Bypass" };
    
    juce::Label inputGainLabel { {}, "Input" };
    juce::Label outputGainLabel { {}, "Output" };
    juce::Label dryWetLabel { {}, "Mix" };
    
    //==============================================================================
    // EQ band controls (simplified for Phase 1)
    struct BandControls {
        juce::Slider freqSlider;
        juce::Slider gainSlider;
        juce::Slider qSlider;
        juce::ComboBox typeCombo;
        juce::ToggleButton enableButton;
        juce::Label label;
    };
    std::array<BandControls, Constants::numEQBands> bandControls;
    
    //==============================================================================
    // Dynamics controls
    // Compressor
    juce::Slider compThresholdSlider;
    juce::Slider compRatioSlider;
    juce::Slider compAttackSlider;
    juce::Slider compReleaseSlider;
    juce::Slider compKneeSlider;
    juce::Slider compMakeupSlider;
    juce::Slider compMixSlider;
    juce::ToggleButton compEnableButton { "Comp" };
    
    // Gate
    juce::Slider gateThresholdSlider;
    juce::Slider gateRatioSlider;
    juce::Slider gateAttackSlider;
    juce::Slider gateHoldSlider;
    juce::Slider gateReleaseSlider;
    juce::Slider gateRangeSlider;
    juce::ToggleButton gateEnableButton { "Gate" };
    
    // Limiter
    juce::Slider limiterCeilingSlider;
    juce::Slider limiterReleaseSlider;
    juce::ToggleButton limiterEnableButton { "Limiter" };
    
    //==============================================================================
    // Metering labels (simple text display for Phase 1)
    juce::Label inputLevelLabel;
    juce::Label outputLevelLabel;
    juce::Label gainReductionLabel;
    
    //==============================================================================
    // Parameter attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    
    // Global
    std::unique_ptr<SliderAttachment> inputGainAttach;
    std::unique_ptr<SliderAttachment> outputGainAttach;
    std::unique_ptr<SliderAttachment> dryWetAttach;
    std::unique_ptr<ButtonAttachment> bypassAttach;
    
    // EQ bands
    struct BandAttachments {
        std::unique_ptr<SliderAttachment> freq;
        std::unique_ptr<SliderAttachment> gain;
        std::unique_ptr<SliderAttachment> q;
        std::unique_ptr<ComboAttachment> type;
        std::unique_ptr<ButtonAttachment> enable;
    };
    std::array<BandAttachments, Constants::numEQBands> bandAttachments;
    
    // Compressor
    std::unique_ptr<SliderAttachment> compThresholdAttach;
    std::unique_ptr<SliderAttachment> compRatioAttach;
    std::unique_ptr<SliderAttachment> compAttackAttach;
    std::unique_ptr<SliderAttachment> compReleaseAttach;
    std::unique_ptr<SliderAttachment> compKneeAttach;
    std::unique_ptr<SliderAttachment> compMakeupAttach;
    std::unique_ptr<SliderAttachment> compMixAttach;
    std::unique_ptr<ButtonAttachment> compEnableAttach;
    
    // Gate
    std::unique_ptr<SliderAttachment> gateThresholdAttach;
    std::unique_ptr<SliderAttachment> gateRatioAttach;
    std::unique_ptr<SliderAttachment> gateAttackAttach;
    std::unique_ptr<SliderAttachment> gateHoldAttach;
    std::unique_ptr<SliderAttachment> gateReleaseAttach;
    std::unique_ptr<SliderAttachment> gateRangeAttach;
    std::unique_ptr<ButtonAttachment> gateEnableAttach;
    
    // Limiter
    std::unique_ptr<SliderAttachment> limiterCeilingAttach;
    std::unique_ptr<SliderAttachment> limiterReleaseAttach;
    std::unique_ptr<ButtonAttachment> limiterEnableAttach;
    
    //==============================================================================
    // Helper methods
    void setupSlider(juce::Slider& slider, juce::Slider::SliderStyle style = juce::Slider::RotaryHorizontalVerticalDrag);
    void setupLabel(juce::Label& label, const juce::String& text);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};

} // namespace SeshEQ
