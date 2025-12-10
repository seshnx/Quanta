#include "PluginEditor.h"

namespace SeshEQ {

//==============================================================================
// BandControlPanel implementation
//==============================================================================

PluginEditor::BandControlPanel::BandControlPanel(int bandIndex) : band(bandIndex) {
    freqSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    freqSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 14);
    
    gainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 14);
    
    qSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    qSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 14);
    
    typeCombo.addItemList(getFilterTypeNames(), 1);
    
    enableButton.setButtonText(juce::String(bandIndex + 1));
    
    addAndMakeVisible(freqSlider);
    addAndMakeVisible(gainSlider);
    addAndMakeVisible(qSlider);
    addAndMakeVisible(typeCombo);
    addAndMakeVisible(enableButton);
    addAndMakeVisible(freqLabel);
    addAndMakeVisible(gainLabel);
    addAndMakeVisible(qLabel);
    
    for (auto* label : { &freqLabel, &gainLabel, &qLabel }) {
        label->setJustificationType(juce::Justification::centred);
        label->setFont(juce::Font(10.0f));
    }
}

void PluginEditor::BandControlPanel::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    
    // Background with band color
    auto color = SeshLookAndFeel::Colors::bandColors[static_cast<size_t>(band)];
    g.setColour(color.withAlpha(0.1f));
    g.fillRoundedRectangle(bounds, 4.0f);
    
    g.setColour(color.withAlpha(0.3f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);
}

void PluginEditor::BandControlPanel::resized() {
    auto bounds = getLocalBounds().reduced(4);
    
    // Enable button and type selector at top
    auto topRow = bounds.removeFromTop(24);
    enableButton.setBounds(topRow.removeFromLeft(24));
    topRow.removeFromLeft(4);
    typeCombo.setBounds(topRow);
    
    bounds.removeFromTop(4);
    
    // Three rotary sliders
    const int knobHeight = (bounds.getHeight() - 30) / 3;
    
    auto freqArea = bounds.removeFromTop(knobHeight);
    freqLabel.setBounds(freqArea.removeFromTop(12));
    freqSlider.setBounds(freqArea);
    
    bounds.removeFromTop(2);
    
    auto gainArea = bounds.removeFromTop(knobHeight);
    gainLabel.setBounds(gainArea.removeFromTop(12));
    gainSlider.setBounds(gainArea);
    
    bounds.removeFromTop(2);
    
    auto qArea = bounds;
    qLabel.setBounds(qArea.removeFromTop(12));
    qSlider.setBounds(qArea);
}

//==============================================================================
// PluginEditor implementation
//==============================================================================

PluginEditor::PluginEditor(PluginProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setLookAndFeel(&lookAndFeel);
    
    auto& apvts = processorRef.getAPVTS();
    
    //==========================================================================
    // Spectrum and EQ display
    addAndMakeVisible(spectrumAnalyzer);
    addAndMakeVisible(eqCurveDisplay);
    
    spectrumAnalyzer.setFFTProcessors(&processorRef.getPreFFT(), &processorRef.getPostFFT());
    eqCurveDisplay.setEQProcessor(&processorRef.getEQProcessor());
    eqCurveDisplay.connectToParameters(apvts);
    
    //==========================================================================
    // EQ band panels
    for (int i = 0; i < Constants::numEQBands; ++i) {
        bandPanels[static_cast<size_t>(i)] = std::make_unique<BandControlPanel>(i);
        addAndMakeVisible(*bandPanels[static_cast<size_t>(i)]);
        
        auto& panel = *bandPanels[static_cast<size_t>(i)];
        auto& attach = bandAttachments[static_cast<size_t>(i)];
        
        attach.freq = std::make_unique<SliderAttachment>(
            apvts, ParamIDs::getBandParamID(i, ParamIDs::bandFreq), panel.freqSlider);
        attach.gain = std::make_unique<SliderAttachment>(
            apvts, ParamIDs::getBandParamID(i, ParamIDs::bandGain), panel.gainSlider);
        attach.q = std::make_unique<SliderAttachment>(
            apvts, ParamIDs::getBandParamID(i, ParamIDs::bandQ), panel.qSlider);
        attach.type = std::make_unique<ComboAttachment>(
            apvts, ParamIDs::getBandParamID(i, ParamIDs::bandType), panel.typeCombo);
        attach.enable = std::make_unique<ButtonAttachment>(
            apvts, ParamIDs::getBandParamID(i, ParamIDs::bandEnable), panel.enableButton);
    }
    
    //==========================================================================
    // Compressor controls
    addAndMakeVisible(compressorGroup);
    compressorGroup.setColour(juce::GroupComponent::outlineColourId, SeshLookAndFeel::Colors::bandColors[0].withAlpha(0.5f));
    compressorGroup.setColour(juce::GroupComponent::textColourId, SeshLookAndFeel::Colors::textPrimary);
    
    for (auto* slider : { &compThresholdSlider, &compRatioSlider, &compAttackSlider,
                          &compReleaseSlider, &compKneeSlider, &compMakeupSlider, &compMixSlider }) {
        setupSlider(*slider);
        addAndMakeVisible(slider);
    }
    
    addAndMakeVisible(compEnableButton);
    
    for (auto* label : { &compThreshLabel, &compRatioLabel, &compAttackLabel, &compReleaseLabel,
                         &compKneeLabel, &compMakeupLabel, &compMixLabel }) {
        setupLabel(*label, label->getText());
        addAndMakeVisible(label);
    }
    
    compThresholdAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::compThreshold, compThresholdSlider);
    compRatioAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::compRatio, compRatioSlider);
    compAttackAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::compAttack, compAttackSlider);
    compReleaseAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::compRelease, compReleaseSlider);
    compKneeAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::compKnee, compKneeSlider);
    compMakeupAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::compMakeup, compMakeupSlider);
    compMixAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::compMix, compMixSlider);
    compEnableAttach = std::make_unique<ButtonAttachment>(apvts, ParamIDs::compEnable, compEnableButton);
    
    //==========================================================================
    // Gate controls
    addAndMakeVisible(gateGroup);
    gateGroup.setColour(juce::GroupComponent::outlineColourId, SeshLookAndFeel::Colors::bandColors[2].withAlpha(0.5f));
    gateGroup.setColour(juce::GroupComponent::textColourId, SeshLookAndFeel::Colors::textPrimary);
    
    for (auto* slider : { &gateThresholdSlider, &gateRatioSlider, &gateAttackSlider,
                          &gateHoldSlider, &gateReleaseSlider, &gateRangeSlider }) {
        setupSlider(*slider);
        addAndMakeVisible(slider);
    }
    
    addAndMakeVisible(gateEnableButton);
    
    for (auto* label : { &gateThreshLabel, &gateRatioLabel, &gateAttackLabel,
                         &gateHoldLabel, &gateReleaseLabel, &gateRangeLabel }) {
        setupLabel(*label, label->getText());
        addAndMakeVisible(label);
    }
    
    gateThresholdAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::gateThreshold, gateThresholdSlider);
    gateRatioAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::gateRatio, gateRatioSlider);
    gateAttackAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::gateAttack, gateAttackSlider);
    gateHoldAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::gateHold, gateHoldSlider);
    gateReleaseAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::gateRelease, gateReleaseSlider);
    gateRangeAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::gateRange, gateRangeSlider);
    gateEnableAttach = std::make_unique<ButtonAttachment>(apvts, ParamIDs::gateEnable, gateEnableButton);
    
    //==========================================================================
    // Limiter controls
    addAndMakeVisible(limiterGroup);
    limiterGroup.setColour(juce::GroupComponent::outlineColourId, SeshLookAndFeel::Colors::bandColors[4].withAlpha(0.5f));
    limiterGroup.setColour(juce::GroupComponent::textColourId, SeshLookAndFeel::Colors::textPrimary);
    
    for (auto* slider : { &limiterCeilingSlider, &limiterReleaseSlider }) {
        setupSlider(*slider);
        addAndMakeVisible(slider);
    }
    
    addAndMakeVisible(limiterEnableButton);
    
    for (auto* label : { &limCeilingLabel, &limReleaseLabel }) {
        setupLabel(*label, label->getText());
        addAndMakeVisible(label);
    }
    
    limiterCeilingAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::limiterCeiling, limiterCeilingSlider);
    limiterReleaseAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::limiterRelease, limiterReleaseSlider);
    limiterEnableAttach = std::make_unique<ButtonAttachment>(apvts, ParamIDs::limiterEnable, limiterEnableButton);
    
    //==========================================================================
    // Global controls
    setupSlider(inputGainSlider);
    setupSlider(outputGainSlider);
    setupSlider(dryWetSlider);
    
    addAndMakeVisible(inputGainSlider);
    addAndMakeVisible(outputGainSlider);
    addAndMakeVisible(dryWetSlider);
    addAndMakeVisible(bypassButton);
    
    setupLabel(inputLabel, "IN");
    setupLabel(outputLabel, "OUT");
    setupLabel(mixLabel, "MIX");
    
    addAndMakeVisible(inputLabel);
    addAndMakeVisible(outputLabel);
    addAndMakeVisible(mixLabel);
    
    inputGainAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::inputGain, inputGainSlider);
    outputGainAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::outputGain, outputGainSlider);
    dryWetAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::dryWet, dryWetSlider);
    bypassAttach = std::make_unique<ButtonAttachment>(apvts, ParamIDs::bypass, bypassButton);
    
    //==========================================================================
    // Meters
    addAndMakeVisible(meterPanel);
    
    //==========================================================================
    // Window setup
    setResizable(true, true);
    setResizeLimits(minWidth, minHeight, 2000, 1400);
    setSize(defaultWidth, defaultHeight);
    
    // Start update timer
    startTimerHz(30);
}

PluginEditor::~PluginEditor() {
    stopTimer();
    setLookAndFeel(nullptr);
}

void PluginEditor::setupSlider(juce::Slider& slider, juce::Slider::SliderStyle style) {
    slider.setSliderStyle(style);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 55, 14);
}

void PluginEditor::setupLabel(juce::Label& label, const juce::String& text) {
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(10.0f));
}

void PluginEditor::timerCallback() {
    // Update meters
    meterPanel.setInputLevel(processorRef.getInputLevel());
    meterPanel.setOutputLevel(processorRef.getOutputLevel());
    meterPanel.setCompressorGR(processorRef.getCompressorGainReduction());
    meterPanel.setGateGR(processorRef.getGateGainReduction());
    meterPanel.setLimiterGR(processorRef.getLimiterGainReduction());
    
    // Repaint EQ curve
    eqCurveDisplay.repaint();
}

void PluginEditor::paint(juce::Graphics& g) {
    // Dark gradient background
    juce::ColourGradient gradient(
        SeshLookAndFeel::Colors::background, 0, 0,
        SeshLookAndFeel::Colors::backgroundDark, 0, static_cast<float>(getHeight()),
        false
    );
    g.setGradientFill(gradient);
    g.fillAll();
    
    // Title
    g.setColour(SeshLookAndFeel::Colors::textPrimary);
    g.setFont(juce::Font(28.0f).boldened());
    g.drawText("SESH", 15, 10, 100, 30, juce::Justification::centredLeft);
    
    g.setColour(SeshLookAndFeel::Colors::accent);
    g.setFont(juce::Font(28.0f).boldened());
    g.drawText("EQ", 75, 10, 50, 30, juce::Justification::centredLeft);
    
    // Section labels
    g.setColour(SeshLookAndFeel::Colors::textSecondary);
    g.setFont(juce::Font(11.0f));
    g.drawText("EQUALIZER", 15, 45, 100, 16, juce::Justification::centredLeft);
}

void PluginEditor::resized() {
    auto bounds = getLocalBounds();
    const int padding = 10;
    const int headerHeight = 50;
    const int meterPanelHeight = 80;
    const int eqBandHeight = 180;
    const int dynamicsHeight = 150;
    
    bounds.reduce(padding, padding);
    
    // Header (title area)
    bounds.removeFromTop(headerHeight - padding);
    
    //==========================================================================
    // Top section: Spectrum + EQ Curve
    auto spectrumArea = bounds.removeFromTop(bounds.getHeight() - eqBandHeight - dynamicsHeight - meterPanelHeight - padding * 3);
    
    // Spectrum analyzer as background
    spectrumAnalyzer.setBounds(spectrumArea);
    
    // EQ curve overlay (same position)
    eqCurveDisplay.setBounds(spectrumArea);
    
    bounds.removeFromTop(padding);
    
    //==========================================================================
    // EQ band controls
    auto eqBandArea = bounds.removeFromTop(eqBandHeight);
    const int bandWidth = eqBandArea.getWidth() / Constants::numEQBands;
    
    for (int i = 0; i < Constants::numEQBands; ++i) {
        auto bandArea = eqBandArea.removeFromLeft(bandWidth).reduced(2, 0);
        bandPanels[static_cast<size_t>(i)]->setBounds(bandArea);
    }
    
    bounds.removeFromTop(padding);
    
    //==========================================================================
    // Dynamics and global controls
    auto bottomArea = bounds.removeFromTop(dynamicsHeight);
    
    // Meter panel on the right
    auto meterArea = bottomArea.removeFromRight(200);
    meterPanel.setBounds(meterArea);
    
    // Global controls next to meters
    auto globalArea = bottomArea.removeFromRight(200).reduced(5, 0);
    
    auto globalRow = globalArea;
    const int globalKnobWidth = 55;
    
    // Bypass button
    bypassButton.setBounds(globalRow.removeFromTop(30).reduced(5, 2));
    globalRow.removeFromTop(5);
    
    // Input/Output/Mix knobs
    auto knobRow = globalRow;
    
    auto inputArea = knobRow.removeFromLeft(globalKnobWidth);
    inputLabel.setBounds(inputArea.removeFromTop(14));
    inputGainSlider.setBounds(inputArea);
    
    knobRow.removeFromLeft(5);
    
    auto outputArea = knobRow.removeFromLeft(globalKnobWidth);
    outputLabel.setBounds(outputArea.removeFromTop(14));
    outputGainSlider.setBounds(outputArea);
    
    knobRow.removeFromLeft(5);
    
    auto mixArea = knobRow.removeFromLeft(globalKnobWidth);
    mixLabel.setBounds(mixArea.removeFromTop(14));
    dryWetSlider.setBounds(mixArea);
    
    //==========================================================================
    // Dynamics controls (remaining space)
    auto dynamicsArea = bottomArea;
    const int dynamicsSectionWidth = dynamicsArea.getWidth() / 3;
    
    // Compressor
    auto compArea = dynamicsArea.removeFromLeft(dynamicsSectionWidth).reduced(2, 0);
    compressorGroup.setBounds(compArea);
    
    auto compInner = compArea.reduced(8, 18);
    compEnableButton.setBounds(compInner.removeFromTop(22));
    compInner.removeFromTop(4);
    
    auto compRow1 = compInner.removeFromTop(compInner.getHeight() / 2);
    const int compKnobW = compRow1.getWidth() / 4;
    
    auto compThreshArea = compRow1.removeFromLeft(compKnobW);
    compThreshLabel.setBounds(compThreshArea.removeFromTop(12));
    compThresholdSlider.setBounds(compThreshArea);
    
    auto compRatioArea = compRow1.removeFromLeft(compKnobW);
    compRatioLabel.setBounds(compRatioArea.removeFromTop(12));
    compRatioSlider.setBounds(compRatioArea);
    
    auto compAttackArea = compRow1.removeFromLeft(compKnobW);
    compAttackLabel.setBounds(compAttackArea.removeFromTop(12));
    compAttackSlider.setBounds(compAttackArea);
    
    auto compReleaseArea = compRow1;
    compReleaseLabel.setBounds(compReleaseArea.removeFromTop(12));
    compReleaseSlider.setBounds(compReleaseArea);
    
    auto compRow2 = compInner;
    const int compKnobW2 = compRow2.getWidth() / 3;
    
    auto compKneeArea = compRow2.removeFromLeft(compKnobW2);
    compKneeLabel.setBounds(compKneeArea.removeFromTop(12));
    compKneeSlider.setBounds(compKneeArea);
    
    auto compMakeupArea = compRow2.removeFromLeft(compKnobW2);
    compMakeupLabel.setBounds(compMakeupArea.removeFromTop(12));
    compMakeupSlider.setBounds(compMakeupArea);
    
    auto compMixArea = compRow2;
    compMixLabel.setBounds(compMixArea.removeFromTop(12));
    compMixSlider.setBounds(compMixArea);
    
    // Gate
    auto gateArea = dynamicsArea.removeFromLeft(dynamicsSectionWidth).reduced(2, 0);
    gateGroup.setBounds(gateArea);
    
    auto gateInner = gateArea.reduced(8, 18);
    gateEnableButton.setBounds(gateInner.removeFromTop(22));
    gateInner.removeFromTop(4);
    
    auto gateRow1 = gateInner.removeFromTop(gateInner.getHeight() / 2);
    const int gateKnobW = gateRow1.getWidth() / 3;
    
    auto gateThreshArea = gateRow1.removeFromLeft(gateKnobW);
    gateThreshLabel.setBounds(gateThreshArea.removeFromTop(12));
    gateThresholdSlider.setBounds(gateThreshArea);
    
    auto gateRatioArea = gateRow1.removeFromLeft(gateKnobW);
    gateRatioLabel.setBounds(gateRatioArea.removeFromTop(12));
    gateRatioSlider.setBounds(gateRatioArea);
    
    auto gateRangeArea = gateRow1;
    gateRangeLabel.setBounds(gateRangeArea.removeFromTop(12));
    gateRangeSlider.setBounds(gateRangeArea);
    
    auto gateRow2 = gateInner;
    const int gateKnobW2 = gateRow2.getWidth() / 3;
    
    auto gateAttackArea = gateRow2.removeFromLeft(gateKnobW2);
    gateAttackLabel.setBounds(gateAttackArea.removeFromTop(12));
    gateAttackSlider.setBounds(gateAttackArea);
    
    auto gateHoldArea = gateRow2.removeFromLeft(gateKnobW2);
    gateHoldLabel.setBounds(gateHoldArea.removeFromTop(12));
    gateHoldSlider.setBounds(gateHoldArea);
    
    auto gateReleaseArea = gateRow2;
    gateReleaseLabel.setBounds(gateReleaseArea.removeFromTop(12));
    gateReleaseSlider.setBounds(gateReleaseArea);
    
    // Limiter
    auto limArea = dynamicsArea.reduced(2, 0);
    limiterGroup.setBounds(limArea);
    
    auto limInner = limArea.reduced(8, 18);
    limiterEnableButton.setBounds(limInner.removeFromTop(22));
    limInner.removeFromTop(4);
    
    auto limRow = limInner;
    const int limKnobW = limRow.getWidth() / 2;
    
    auto limCeilingArea = limRow.removeFromLeft(limKnobW);
    limCeilingLabel.setBounds(limCeilingArea.removeFromTop(12));
    limiterCeilingSlider.setBounds(limCeilingArea);
    
    auto limReleaseArea = limRow;
    limReleaseLabel.setBounds(limReleaseArea.removeFromTop(12));
    limiterReleaseSlider.setBounds(limReleaseArea);
}

} // namespace SeshEQ
