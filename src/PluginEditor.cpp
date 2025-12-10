#include "PluginEditor.h"

namespace SeshEQ {

PluginEditor::PluginEditor(PluginProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    auto& apvts = processorRef.getAPVTS();
    
    //==========================================================================
    // Setup global controls
    setupSlider(inputGainSlider);
    setupSlider(outputGainSlider);
    setupSlider(dryWetSlider);
    
    addAndMakeVisible(inputGainSlider);
    addAndMakeVisible(outputGainSlider);
    addAndMakeVisible(dryWetSlider);
    addAndMakeVisible(bypassButton);
    
    setupLabel(inputGainLabel, "Input");
    setupLabel(outputGainLabel, "Output");
    setupLabel(dryWetLabel, "Mix");
    
    addAndMakeVisible(inputGainLabel);
    addAndMakeVisible(outputGainLabel);
    addAndMakeVisible(dryWetLabel);
    
    // Attachments
    inputGainAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::inputGain, inputGainSlider);
    outputGainAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::outputGain, outputGainSlider);
    dryWetAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::dryWet, dryWetSlider);
    bypassAttach = std::make_unique<ButtonAttachment>(apvts, ParamIDs::bypass, bypassButton);
    
    //==========================================================================
    // Setup EQ band controls
    for (int i = 0; i < Constants::numEQBands; ++i) {
        auto& band = bandControls[static_cast<size_t>(i)];
        auto& attach = bandAttachments[static_cast<size_t>(i)];
        
        setupSlider(band.freqSlider);
        setupSlider(band.gainSlider);
        setupSlider(band.qSlider);
        
        band.typeCombo.addItemList(getFilterTypeNames(), 1);
        band.enableButton.setButtonText(juce::String(i + 1));
        
        addAndMakeVisible(band.freqSlider);
        addAndMakeVisible(band.gainSlider);
        addAndMakeVisible(band.qSlider);
        addAndMakeVisible(band.typeCombo);
        addAndMakeVisible(band.enableButton);
        
        setupLabel(band.label, "Band " + juce::String(i + 1));
        addAndMakeVisible(band.label);
        
        // Attachments
        attach.freq = std::make_unique<SliderAttachment>(
            apvts, ParamIDs::getBandParamID(i, ParamIDs::bandFreq), band.freqSlider);
        attach.gain = std::make_unique<SliderAttachment>(
            apvts, ParamIDs::getBandParamID(i, ParamIDs::bandGain), band.gainSlider);
        attach.q = std::make_unique<SliderAttachment>(
            apvts, ParamIDs::getBandParamID(i, ParamIDs::bandQ), band.qSlider);
        attach.type = std::make_unique<ComboAttachment>(
            apvts, ParamIDs::getBandParamID(i, ParamIDs::bandType), band.typeCombo);
        attach.enable = std::make_unique<ButtonAttachment>(
            apvts, ParamIDs::getBandParamID(i, ParamIDs::bandEnable), band.enableButton);
    }
    
    //==========================================================================
    // Setup compressor controls
    setupSlider(compThresholdSlider);
    setupSlider(compRatioSlider);
    setupSlider(compAttackSlider);
    setupSlider(compReleaseSlider);
    setupSlider(compKneeSlider);
    setupSlider(compMakeupSlider);
    setupSlider(compMixSlider);
    
    addAndMakeVisible(compThresholdSlider);
    addAndMakeVisible(compRatioSlider);
    addAndMakeVisible(compAttackSlider);
    addAndMakeVisible(compReleaseSlider);
    addAndMakeVisible(compKneeSlider);
    addAndMakeVisible(compMakeupSlider);
    addAndMakeVisible(compMixSlider);
    addAndMakeVisible(compEnableButton);
    
    compThresholdAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::compThreshold, compThresholdSlider);
    compRatioAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::compRatio, compRatioSlider);
    compAttackAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::compAttack, compAttackSlider);
    compReleaseAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::compRelease, compReleaseSlider);
    compKneeAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::compKnee, compKneeSlider);
    compMakeupAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::compMakeup, compMakeupSlider);
    compMixAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::compMix, compMixSlider);
    compEnableAttach = std::make_unique<ButtonAttachment>(apvts, ParamIDs::compEnable, compEnableButton);
    
    //==========================================================================
    // Setup gate controls
    setupSlider(gateThresholdSlider);
    setupSlider(gateRatioSlider);
    setupSlider(gateAttackSlider);
    setupSlider(gateHoldSlider);
    setupSlider(gateReleaseSlider);
    setupSlider(gateRangeSlider);
    
    addAndMakeVisible(gateThresholdSlider);
    addAndMakeVisible(gateRatioSlider);
    addAndMakeVisible(gateAttackSlider);
    addAndMakeVisible(gateHoldSlider);
    addAndMakeVisible(gateReleaseSlider);
    addAndMakeVisible(gateRangeSlider);
    addAndMakeVisible(gateEnableButton);
    
    gateThresholdAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::gateThreshold, gateThresholdSlider);
    gateRatioAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::gateRatio, gateRatioSlider);
    gateAttackAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::gateAttack, gateAttackSlider);
    gateHoldAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::gateHold, gateHoldSlider);
    gateReleaseAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::gateRelease, gateReleaseSlider);
    gateRangeAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::gateRange, gateRangeSlider);
    gateEnableAttach = std::make_unique<ButtonAttachment>(apvts, ParamIDs::gateEnable, gateEnableButton);
    
    //==========================================================================
    // Setup limiter controls
    setupSlider(limiterCeilingSlider);
    setupSlider(limiterReleaseSlider);
    
    addAndMakeVisible(limiterCeilingSlider);
    addAndMakeVisible(limiterReleaseSlider);
    addAndMakeVisible(limiterEnableButton);
    
    limiterCeilingAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::limiterCeiling, limiterCeilingSlider);
    limiterReleaseAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::limiterRelease, limiterReleaseSlider);
    limiterEnableAttach = std::make_unique<ButtonAttachment>(apvts, ParamIDs::limiterEnable, limiterEnableButton);
    
    //==========================================================================
    // Setup metering labels
    inputLevelLabel.setJustificationType(juce::Justification::centred);
    outputLevelLabel.setJustificationType(juce::Justification::centred);
    gainReductionLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(inputLevelLabel);
    addAndMakeVisible(outputLevelLabel);
    addAndMakeVisible(gainReductionLabel);
    
    //==========================================================================
    // Set window size
    setSize(1000, 600);
    
    // Start timer for metering updates
    startTimerHz(30);
}

PluginEditor::~PluginEditor() {
    stopTimer();
}

void PluginEditor::setupSlider(juce::Slider& slider, juce::Slider::SliderStyle style) {
    slider.setSliderStyle(style);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
}

void PluginEditor::setupLabel(juce::Label& label, const juce::String& text) {
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
}

void PluginEditor::timerCallback() {
    // Update metering displays
    const float inputDb = processorRef.getInputLevel();
    const float outputDb = processorRef.getOutputLevel();
    const float compGr = processorRef.getCompressorGainReduction();
    const float gateGr = processorRef.getGateGainReduction();
    const float limGr = processorRef.getLimiterGainReduction();
    
    inputLevelLabel.setText("In: " + juce::String(inputDb, 1) + " dB", juce::dontSendNotification);
    outputLevelLabel.setText("Out: " + juce::String(outputDb, 1) + " dB", juce::dontSendNotification);
    
    // Show total gain reduction
    const float totalGr = compGr + gateGr + limGr;
    gainReductionLabel.setText("GR: " + juce::String(totalGr, 1) + " dB", juce::dontSendNotification);
}

void PluginEditor::paint(juce::Graphics& g) {
    // Dark background
    g.fillAll(juce::Colour(0xff1a1a2e));
    
    // Title
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(24.0f));
    g.drawText("SeshEQ", getLocalBounds().removeFromTop(40), juce::Justification::centred);
    
    // Section backgrounds
    g.setColour(juce::Colour(0xff2d2d44));
    
    // EQ section background
    g.fillRoundedRectangle(10.0f, 50.0f, getWidth() - 20.0f, 200.0f, 8.0f);
    
    // Dynamics section background
    g.fillRoundedRectangle(10.0f, 260.0f, getWidth() - 20.0f, 220.0f, 8.0f);
    
    // Global section background
    g.fillRoundedRectangle(10.0f, 490.0f, getWidth() - 20.0f, 100.0f, 8.0f);
    
    // Section labels
    g.setColour(juce::Colours::white.withAlpha(0.8f));
    g.setFont(juce::Font(14.0f));
    g.drawText("EQUALIZER", 20, 55, 100, 20, juce::Justification::left);
    g.drawText("DYNAMICS", 20, 265, 100, 20, juce::Justification::left);
    g.drawText("GLOBAL", 20, 495, 100, 20, juce::Justification::left);
}

void PluginEditor::resized() {
    auto bounds = getLocalBounds();
    
    //==========================================================================
    // EQ section (top area)
    auto eqArea = bounds.removeFromTop(250).reduced(20, 0);
    eqArea.removeFromTop(60); // Skip title and padding
    
    const int bandWidth = eqArea.getWidth() / Constants::numEQBands;
    
    for (int i = 0; i < Constants::numEQBands; ++i) {
        auto& band = bandControls[static_cast<size_t>(i)];
        auto bandArea = eqArea.removeFromLeft(bandWidth).reduced(5, 0);
        
        band.label.setBounds(bandArea.removeFromTop(20));
        band.enableButton.setBounds(bandArea.removeFromTop(25));
        band.typeCombo.setBounds(bandArea.removeFromTop(25));
        band.freqSlider.setBounds(bandArea.removeFromTop(50));
        band.gainSlider.setBounds(bandArea.removeFromTop(50));
        band.qSlider.setBounds(bandArea.removeFromTop(50));
    }
    
    //==========================================================================
    // Dynamics section
    auto dynArea = bounds.removeFromTop(230).reduced(20, 0);
    dynArea.removeFromTop(20); // Skip label
    
    // Compressor (left third)
    auto compArea = dynArea.removeFromLeft(dynArea.getWidth() / 3).reduced(5);
    compEnableButton.setBounds(compArea.removeFromTop(25));
    
    auto compRow1 = compArea.removeFromTop(80);
    const int compKnobWidth = compRow1.getWidth() / 4;
    compThresholdSlider.setBounds(compRow1.removeFromLeft(compKnobWidth));
    compRatioSlider.setBounds(compRow1.removeFromLeft(compKnobWidth));
    compAttackSlider.setBounds(compRow1.removeFromLeft(compKnobWidth));
    compReleaseSlider.setBounds(compRow1);
    
    auto compRow2 = compArea.removeFromTop(80);
    const int compKnobWidth2 = compRow2.getWidth() / 3;
    compKneeSlider.setBounds(compRow2.removeFromLeft(compKnobWidth2));
    compMakeupSlider.setBounds(compRow2.removeFromLeft(compKnobWidth2));
    compMixSlider.setBounds(compRow2);
    
    // Gate (middle third)
    auto gateArea = dynArea.removeFromLeft(dynArea.getWidth() / 2).reduced(5);
    gateEnableButton.setBounds(gateArea.removeFromTop(25));
    
    auto gateRow1 = gateArea.removeFromTop(80);
    const int gateKnobWidth = gateRow1.getWidth() / 3;
    gateThresholdSlider.setBounds(gateRow1.removeFromLeft(gateKnobWidth));
    gateRatioSlider.setBounds(gateRow1.removeFromLeft(gateKnobWidth));
    gateRangeSlider.setBounds(gateRow1);
    
    auto gateRow2 = gateArea.removeFromTop(80);
    const int gateKnobWidth2 = gateRow2.getWidth() / 3;
    gateAttackSlider.setBounds(gateRow2.removeFromLeft(gateKnobWidth2));
    gateHoldSlider.setBounds(gateRow2.removeFromLeft(gateKnobWidth2));
    gateReleaseSlider.setBounds(gateRow2);
    
    // Limiter (right third)
    auto limArea = dynArea.reduced(5);
    limiterEnableButton.setBounds(limArea.removeFromTop(25));
    
    auto limRow = limArea.removeFromTop(80);
    limiterCeilingSlider.setBounds(limRow.removeFromLeft(limRow.getWidth() / 2));
    limiterReleaseSlider.setBounds(limRow);
    
    //==========================================================================
    // Global section (bottom)
    auto globalArea = bounds.reduced(20, 10);
    globalArea.removeFromTop(10); // Skip label
    
    const int globalKnobWidth = 80;
    const int meterWidth = 100;
    
    auto row = globalArea;
    
    bypassButton.setBounds(row.removeFromLeft(80).reduced(5));
    
    auto inputArea = row.removeFromLeft(globalKnobWidth);
    inputGainLabel.setBounds(inputArea.removeFromTop(20));
    inputGainSlider.setBounds(inputArea);
    
    auto outputArea = row.removeFromLeft(globalKnobWidth);
    outputGainLabel.setBounds(outputArea.removeFromTop(20));
    outputGainSlider.setBounds(outputArea);
    
    auto mixArea = row.removeFromLeft(globalKnobWidth);
    dryWetLabel.setBounds(mixArea.removeFromTop(20));
    dryWetSlider.setBounds(mixArea);
    
    // Metering labels
    row.removeFromLeft(20); // Spacing
    inputLevelLabel.setBounds(row.removeFromLeft(meterWidth));
    outputLevelLabel.setBounds(row.removeFromLeft(meterWidth));
    gainReductionLabel.setBounds(row.removeFromLeft(meterWidth));
}

} // namespace SeshEQ
