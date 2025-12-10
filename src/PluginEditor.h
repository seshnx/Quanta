#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "ui/SpectrumAnalyzer.h"
#include "ui/EQCurveDisplay.h"
#include "ui/MeterComponent.h"
#include "ui/LookAndFeel.h"

namespace SeshEQ {

/**
 * @brief Main editor/UI for SeshEQ plugin
 * 
 * Modern UI with:
 * - Spectrum analyzer with EQ curve overlay
 * - Draggable EQ band nodes
 * - Dynamics controls
 * - Level and gain reduction meters
 * - Resizable window
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
    void setupSlider(juce::Slider& slider, juce::Slider::SliderStyle style = juce::Slider::RotaryHorizontalVerticalDrag);
    void setupLabel(juce::Label& label, const juce::String& text);
    
    // Reference to processor
    PluginProcessor& processorRef;
    
    // Custom look and feel
    SeshLookAndFeel lookAndFeel;
    
    //==============================================================================
    // Main display area - Spectrum + EQ Curve
    SpectrumAnalyzer spectrumAnalyzer;
    EQCurveDisplay eqCurveDisplay;
    
    //==============================================================================
    // EQ band controls panel
    struct BandControlPanel : public juce::Component {
        BandControlPanel(int bandIndex);
        void resized() override;
        void paint(juce::Graphics& g) override;
        
        int band;
        juce::Slider freqSlider;
        juce::Slider gainSlider;
        juce::Slider qSlider;
        juce::ComboBox typeCombo;
        juce::ToggleButton enableButton;
        juce::Label freqLabel { {}, "Freq" };
        juce::Label gainLabel { {}, "Gain" };
        juce::Label qLabel { {}, "Q" };
    };
    
    std::array<std::unique_ptr<BandControlPanel>, Constants::numEQBands> bandPanels;
    
    //==============================================================================
    // Dynamics controls
    // Compressor panel
    juce::GroupComponent compressorGroup { {}, "COMPRESSOR" };
    juce::Slider compThresholdSlider, compRatioSlider, compAttackSlider;
    juce::Slider compReleaseSlider, compKneeSlider, compMakeupSlider, compMixSlider;
    juce::ToggleButton compEnableButton { "ON" };
    juce::Label compThreshLabel { {}, "Thresh" }, compRatioLabel { {}, "Ratio" };
    juce::Label compAttackLabel { {}, "Attack" }, compReleaseLabel { {}, "Release" };
    juce::Label compKneeLabel { {}, "Knee" }, compMakeupLabel { {}, "Makeup" };
    juce::Label compMixLabel { {}, "Mix" };
    
    // Gate panel
    juce::GroupComponent gateGroup { {}, "GATE" };
    juce::Slider gateThresholdSlider, gateRatioSlider, gateAttackSlider;
    juce::Slider gateHoldSlider, gateReleaseSlider, gateRangeSlider;
    juce::ToggleButton gateEnableButton { "ON" };
    juce::Label gateThreshLabel { {}, "Thresh" }, gateRatioLabel { {}, "Ratio" };
    juce::Label gateAttackLabel { {}, "Attack" }, gateHoldLabel { {}, "Hold" };
    juce::Label gateReleaseLabel { {}, "Release" }, gateRangeLabel { {}, "Range" };
    
    // Limiter panel
    juce::GroupComponent limiterGroup { {}, "LIMITER" };
    juce::Slider limiterCeilingSlider, limiterReleaseSlider;
    juce::ToggleButton limiterEnableButton { "ON" };
    juce::Label limCeilingLabel { {}, "Ceiling" }, limReleaseLabel { {}, "Release" };
    
    //==============================================================================
    // Global controls
    juce::Slider inputGainSlider, outputGainSlider, dryWetSlider;
    juce::ToggleButton bypassButton { "BYPASS" };
    juce::Label inputLabel { {}, "IN" }, outputLabel { {}, "OUT" }, mixLabel { {}, "MIX" };
    
    //==============================================================================
    // Advanced DSP controls
    juce::ComboBox processingModeCombo;
    juce::ComboBox oversamplingCombo;
    juce::ComboBox scFilterModeCombo;
    juce::Slider scFilterFreqSlider;
    juce::Slider scFilterQSlider;
    juce::ToggleButton scListenButton { "SC Listen" };
    
    juce::Label processingModeLabel { {}, "Mode" };
    juce::Label oversamplingLabel { {}, "OS" };
    juce::Label scFilterLabel { {}, "SC Filter" };
    
    //==============================================================================
    // Metering
    DynamicsMeterPanel meterPanel;
    
    //==============================================================================
    // Parameter attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    
    // Global
    std::unique_ptr<SliderAttachment> inputGainAttach, outputGainAttach, dryWetAttach;
    std::unique_ptr<ButtonAttachment> bypassAttach;
    
    // EQ bands
    struct BandAttachments {
        std::unique_ptr<SliderAttachment> freq, gain, q;
        std::unique_ptr<ComboAttachment> type;
        std::unique_ptr<ButtonAttachment> enable;
    };
    std::array<BandAttachments, Constants::numEQBands> bandAttachments;
    
    // Compressor
    std::unique_ptr<SliderAttachment> compThresholdAttach, compRatioAttach;
    std::unique_ptr<SliderAttachment> compAttackAttach, compReleaseAttach;
    std::unique_ptr<SliderAttachment> compKneeAttach, compMakeupAttach, compMixAttach;
    std::unique_ptr<ButtonAttachment> compEnableAttach;
    
    // Gate
    std::unique_ptr<SliderAttachment> gateThresholdAttach, gateRatioAttach;
    std::unique_ptr<SliderAttachment> gateAttackAttach, gateHoldAttach;
    std::unique_ptr<SliderAttachment> gateReleaseAttach, gateRangeAttach;
    std::unique_ptr<ButtonAttachment> gateEnableAttach;
    
    // Limiter
    std::unique_ptr<SliderAttachment> limiterCeilingAttach, limiterReleaseAttach;
    std::unique_ptr<ButtonAttachment> limiterEnableAttach;
    
    // Advanced DSP
    std::unique_ptr<ComboAttachment> processingModeAttach, oversamplingAttach;
    std::unique_ptr<ComboAttachment> scFilterModeAttach;
    std::unique_ptr<SliderAttachment> scFilterFreqAttach, scFilterQAttach;
    std::unique_ptr<ButtonAttachment> scListenAttach;
    
    //==============================================================================
    // Sizing
    static constexpr int defaultWidth = 1100;
    static constexpr int defaultHeight = 700;
    static constexpr int minWidth = 800;
    static constexpr int minHeight = 500;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};

} // namespace SeshEQ
