#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "ui/SpectrumAnalyzer.h"
#include "ui/EQCurveDisplay.h"
#include "ui/MeterComponent.h"
#include "ui/LookAndFeel.h"

namespace SeshEQ {

/**
 * @brief Main editor/UI for SeshNx Quanta plugin
 * 
 * Sci-Fi themed UI with:
 * - Real-Time Spectrum Analyzer (central focus)
 * - Interactive EQ Curve overlay (central focus)
 * - 8-band Multiband Dynamic EQ with per-band controls
 * - Per-band Gain Reduction meters
 * - True Peak Limiter with dedicated meter
 * - Fully resizable window
 */
class PluginEditor : public juce::AudioProcessorEditor,
                      private juce::Timer {
public:
    explicit PluginEditor(PluginProcessor&);
    ~PluginEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void paintOverChildren(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void setupSlider(juce::Slider& slider, juce::Slider::SliderStyle style = juce::Slider::RotaryHorizontalVerticalDrag);
    void setupLabel(juce::Label& label, const juce::String& text);
    void loadLogo();
    void refreshPresetList();
    void saveCurrentPreset();
    void updateLatencyDisplay();
    
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
        void setGainReduction(float dB) { gainReductionDb = dB; repaint(); }

        int band;
        float gainReductionDb = 0.0f;

        // EQ controls
        juce::Slider freqSlider;
        juce::Slider gainSlider;
        juce::Slider qSlider;
        juce::ComboBox typeCombo;
        juce::ToggleButton enableButton;
        juce::Label freqLabel { {}, "Freq" };
        juce::Label gainLabel { {}, "Gain" };
        juce::Label qLabel { {}, "Q" };

        // Per-band dynamics controls
        juce::Slider dynThreshSlider;
        juce::Slider dynRatioSlider;
        juce::ToggleButton dynEnableButton { "DYN" };
        juce::Label dynThreshLabel { {}, "Thr" };
        juce::Label dynRatioLabel { {}, "Rat" };
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
    juce::GroupComponent limiterGroup { {}, "TRUE PEAK LIMITER" };
    juce::Slider limiterThresholdSlider, limiterCeilingSlider, limiterReleaseSlider;
    juce::ToggleButton limiterEnableButton { "ON" };
    juce::Label limThreshLabel { {}, "Thresh" }, limCeilingLabel { {}, "Ceiling" }, limReleaseLabel { {}, "Release" };
    
    //==============================================================================
    // Global controls
    juce::Slider inputGainSlider, outputGainSlider, dryWetSlider;
    juce::ToggleButton bypassButton { "BYPASS" };
    juce::Label inputLabel { {}, "IN" }, outputLabel { {}, "OUT" }, mixLabel { {}, "MIX" };

    // Advanced mode toggles
    juce::ToggleButton linearPhaseButton { "LINEAR PHASE" };
    juce::ToggleButton midSideButton { "MID/SIDE" };
    juce::ToggleButton dynamicEQButton { "DYNAMIC EQ" };

    // Oversampling control
    juce::ComboBox oversamplingCombo;
    juce::Label oversamplingLabel { {}, "OVERSAMPLE" };

    // Preset controls
    juce::ComboBox presetCombo;
    juce::TextButton savePresetButton { "Save" };
    juce::Label presetLabel { {}, "PRESET" };
    juce::Label latencyLabel { {}, "0 samples" };
    
    //==============================================================================
    // Metering
    DynamicsMeterPanel meterPanel;
    
    //==============================================================================
    // Company branding
    juce::ImageComponent logoComponent;
    juce::TextButton websiteLink;
    juce::Image logoImage;
    
    //==============================================================================
    // Parameter attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    
    // Global
    std::unique_ptr<SliderAttachment> inputGainAttach, outputGainAttach, dryWetAttach;
    std::unique_ptr<ButtonAttachment> bypassAttach;
    std::unique_ptr<ButtonAttachment> linearPhaseAttach, midSideAttach, dynamicEQAttach;
    std::unique_ptr<ComboAttachment> oversamplingAttach;
    
    // EQ bands
    struct BandAttachments {
        std::unique_ptr<SliderAttachment> freq, gain, q;
        std::unique_ptr<ComboAttachment> type;
        std::unique_ptr<ButtonAttachment> enable;
        // Per-band dynamics
        std::unique_ptr<SliderAttachment> dynThresh, dynRatio;
        std::unique_ptr<ButtonAttachment> dynEnable;
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
    std::unique_ptr<SliderAttachment> limiterThresholdAttach, limiterCeilingAttach, limiterReleaseAttach;
    std::unique_ptr<ButtonAttachment> limiterEnableAttach;
    
    //==============================================================================
    // Sizing
    static constexpr int defaultWidth = 1400;
    static constexpr int defaultHeight = 900;
    static constexpr int minWidth = 1000;
    static constexpr int minHeight = 650;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};

} // namespace SeshEQ
