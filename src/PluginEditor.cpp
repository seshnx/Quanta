#include "PluginEditor.h"
#include <cmath>

namespace SeshEQ {

//==============================================================================
// BandControlPanel implementation
//==============================================================================

PluginEditor::BandControlPanel::BandControlPanel(int bandIndex) : band(bandIndex) {
    // EQ controls - larger text boxes for readability
    freqSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    freqSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 16);

    gainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 16);

    qSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    qSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 16);

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
        label->setFont(juce::Font(11.0f));
    }

    // Per-band dynamics controls - larger for readability
    dynThreshSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    dynThreshSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 14);

    dynRatioSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    dynRatioSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 14);

    addAndMakeVisible(dynThreshSlider);
    addAndMakeVisible(dynRatioSlider);
    addAndMakeVisible(dynEnableButton);
    addAndMakeVisible(dynThreshLabel);
    addAndMakeVisible(dynRatioLabel);

    for (auto* label : { &dynThreshLabel, &dynRatioLabel }) {
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

    // Draw GR meter at the bottom of the panel
    const float meterHeight = 8.0f;
    const float meterMargin = 6.0f;
    auto meterBounds = bounds.removeFromBottom(meterHeight + meterMargin).reduced(meterMargin, 0);
    meterBounds = meterBounds.removeFromTop(meterHeight);

    // Meter background
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillRoundedRectangle(meterBounds, 2.0f);

    // GR bar (show reduction from right to left)
    if (gainReductionDb < -0.1f) {
        const float maxGR = -24.0f;
        const float grNorm = std::clamp(gainReductionDb / maxGR, 0.0f, 1.0f);
        const float grWidth = meterBounds.getWidth() * grNorm;

        // Draw from right side (reduction grows left)
        auto grBar = meterBounds;
        grBar.setWidth(grWidth);
        grBar.setX(meterBounds.getRight() - grWidth);

        g.setColour(color.withAlpha(0.9f));
        g.fillRoundedRectangle(grBar, 2.0f);

        // Show GR value text if significant
        if (gainReductionDb < -1.0f) {
            g.setColour(juce::Colours::white.withAlpha(0.8f));
            g.setFont(juce::Font(9.0f));
            g.drawText(juce::String(static_cast<int>(std::round(gainReductionDb))) + "dB",
                       meterBounds.toNearestInt(), juce::Justification::centred);
        }
    }

    // Meter border
    g.setColour(color.withAlpha(0.5f));
    g.drawRoundedRectangle(meterBounds, 2.0f, 1.0f);
}

void PluginEditor::BandControlPanel::resized() {
    auto bounds = getLocalBounds().reduced(6);

    // Reserve space for GR meter at bottom
    bounds.removeFromBottom(16);

    // Enable button and type selector at top
    auto topRow = bounds.removeFromTop(28);
    enableButton.setBounds(topRow.removeFromLeft(28));
    topRow.removeFromLeft(6);
    typeCombo.setBounds(topRow);

    bounds.removeFromTop(6);

    // Split into left (EQ) and right (Dynamics) sections
    const int dynWidth = 65;
    auto dynBounds = bounds.removeFromRight(dynWidth);
    bounds.removeFromRight(4);

    // EQ controls - Three rotary sliders with proper spacing
    const int knobHeight = (bounds.getHeight() - 42) / 3;

    auto freqArea = bounds.removeFromTop(knobHeight);
    freqLabel.setBounds(freqArea.removeFromTop(14));
    freqSlider.setBounds(freqArea);

    bounds.removeFromTop(4);

    auto gainArea = bounds.removeFromTop(knobHeight);
    gainLabel.setBounds(gainArea.removeFromTop(14));
    gainSlider.setBounds(gainArea);

    bounds.removeFromTop(4);

    auto qArea = bounds;
    qLabel.setBounds(qArea.removeFromTop(14));
    qSlider.setBounds(qArea);

    // Dynamics controls - right column
    dynEnableButton.setBounds(dynBounds.removeFromTop(24));
    dynBounds.removeFromTop(4);

    const int dynKnobHeight = (dynBounds.getHeight() - 24) / 2;

    auto dynThreshArea = dynBounds.removeFromTop(dynKnobHeight);
    dynThreshLabel.setBounds(dynThreshArea.removeFromTop(12));
    dynThreshSlider.setBounds(dynThreshArea);

    dynBounds.removeFromTop(4);

    auto dynRatioArea = dynBounds;
    dynRatioLabel.setBounds(dynRatioArea.removeFromTop(12));
    dynRatioSlider.setBounds(dynRatioArea);
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
    
    // Only show post-FFT (final output) - not pre-EQ
    spectrumAnalyzer.setFFTProcessor(&processorRef.getPostFFT());
    spectrumAnalyzer.setShowPreSpectrum(false);
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

        // Per-band dynamics attachments
        attach.dynThresh = std::make_unique<SliderAttachment>(
            apvts, ParamIDs::getBandParamID(i, ParamIDs::bandDynThreshold), panel.dynThreshSlider);
        attach.dynRatio = std::make_unique<SliderAttachment>(
            apvts, ParamIDs::getBandParamID(i, ParamIDs::bandDynRatio), panel.dynRatioSlider);
        attach.dynEnable = std::make_unique<ButtonAttachment>(
            apvts, ParamIDs::getBandParamID(i, ParamIDs::bandDynEnable), panel.dynEnableButton);

        // Set custom text functions AFTER attachments (so they override parameter formatting)
        panel.freqSlider.textFromValueFunction = [](double value) {
            if (value >= 1000.0)
                return juce::String(static_cast<int>(std::round(value))) + " Hz";
            return juce::String(static_cast<int>(std::round(value))) + " Hz";
        };
        panel.gainSlider.textFromValueFunction = [](double value) {
            return juce::String(value, 1) + " dB";
        };
        panel.qSlider.textFromValueFunction = [](double value) {
            return juce::String(value, 2);
        };
        panel.dynThreshSlider.textFromValueFunction = [](double value) {
            return juce::String(static_cast<int>(std::round(value))) + "dB";
        };
        panel.dynRatioSlider.textFromValueFunction = [](double value) {
            return juce::String(value, 1) + ":1";
        };
        // Force update slider text
        panel.freqSlider.updateText();
        panel.gainSlider.updateText();
        panel.qSlider.updateText();
        panel.dynThreshSlider.updateText();
        panel.dynRatioSlider.updateText();
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
    // Limiter controls (True Peak Limiter)
    addAndMakeVisible(limiterGroup);
    limiterGroup.setColour(juce::GroupComponent::outlineColourId, SeshLookAndFeel::Colors::bandColors[4].withAlpha(0.5f));
    limiterGroup.setColour(juce::GroupComponent::textColourId, SeshLookAndFeel::Colors::textPrimary);

    for (auto* slider : { &limiterThresholdSlider, &limiterCeilingSlider, &limiterReleaseSlider }) {
        setupSlider(*slider);
        addAndMakeVisible(slider);
    }

    addAndMakeVisible(limiterEnableButton);

    for (auto* label : { &limThreshLabel, &limCeilingLabel, &limReleaseLabel }) {
        setupLabel(*label, label->getText());
        addAndMakeVisible(label);
    }

    limiterThresholdAttach = std::make_unique<SliderAttachment>(apvts, ParamIDs::limiterThreshold, limiterThresholdSlider);
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
    // Advanced mode toggles
    addAndMakeVisible(linearPhaseButton);
    addAndMakeVisible(midSideButton);
    addAndMakeVisible(dynamicEQButton);

    linearPhaseAttach = std::make_unique<ButtonAttachment>(apvts, ParamIDs::linearPhaseMode, linearPhaseButton);
    midSideAttach = std::make_unique<ButtonAttachment>(apvts, ParamIDs::midSideMode, midSideButton);
    dynamicEQAttach = std::make_unique<ButtonAttachment>(apvts, ParamIDs::dynamicEQMode, dynamicEQButton);

    //==========================================================================
    // Oversampling control
    oversamplingCombo.addItemList(getOversamplingNames(), 1);
    setupLabel(oversamplingLabel, "OVERSAMPLE");
    addAndMakeVisible(oversamplingCombo);
    addAndMakeVisible(oversamplingLabel);

    oversamplingAttach = std::make_unique<ComboAttachment>(apvts, ParamIDs::oversamplingFactor, oversamplingCombo);

    //==========================================================================
    // Preset controls
    setupLabel(presetLabel, "PRESET");
    addAndMakeVisible(presetLabel);
    addAndMakeVisible(presetCombo);
    addAndMakeVisible(savePresetButton);
    addAndMakeVisible(latencyLabel);

    latencyLabel.setFont(juce::Font(10.0f));
    latencyLabel.setColour(juce::Label::textColourId, SeshLookAndFeel::Colors::textSecondary);

    refreshPresetList();

    presetCombo.onChange = [this] {
        const int selected = presetCombo.getSelectedItemIndex();
        if (selected >= 0) {
            auto& presetMgr = processorRef.getPresetManager();
            auto names = presetMgr.getAllPresetNames();
            if (selected < names.size() && names[selected] != "---") {
                presetMgr.loadPreset(names[selected]);
            }
        }
    };

    savePresetButton.onClick = [this] {
        saveCurrentPreset();
    };

    //==========================================================================
    // Meters
    addAndMakeVisible(meterPanel);
    
    //==========================================================================
    // Company branding
    loadLogo();
    logoComponent.setImage(logoImage);
    logoComponent.setImagePlacement(juce::RectanglePlacement::centred);
    logoComponent.setInterceptsMouseClicks(false, false);  // Don't intercept mouse events
    addAndMakeVisible(logoComponent);
    
    websiteLink.setButtonText("seshnx.com");
    websiteLink.setColour(juce::TextButton::textColourOffId, SeshLookAndFeel::Colors::accent);  // Brand Blue
    websiteLink.setColour(juce::TextButton::textColourOnId, SeshLookAndFeel::Colors::accentAlt);  // Brand Dark Accent
    websiteLink.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    websiteLink.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    websiteLink.setConnectedEdges(juce::TextButton::ConnectedEdgeFlags::ConnectedOnLeft | 
                                   juce::TextButton::ConnectedEdgeFlags::ConnectedOnRight);
    
    // Note: TextButton doesn't have setFont() method
    // Font will be controlled by LookAndFeel
    // To use Striker font, it would need to be set in the LookAndFeel class
    
    websiteLink.onClick = [this] {
        juce::URL("https://seshnx.com").launchInDefaultBrowser();
    };
    addAndMakeVisible(websiteLink);
    
    //==========================================================================
    // Window setup
    setResizable(true, true);
    setResizeLimits(minWidth, minHeight, 2000, 1400);
    setSize(defaultWidth, defaultHeight);
    
    // Start update timer
    startTimerHz(20);  // Balanced update rate
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

void PluginEditor::loadLogo() {
    // Try to load logo from file (relative to plugin location or executable directory)
    // Look for the new SeshNx logo
    juce::File logoFile;
    
    // Try common logo filenames
    const std::array<juce::String, 4> logoNames = {
        "SeshNx_Logo.png",
        "SeshNx-Logo.png",
        "logo.png",
        "SeshNx.png"
    };
    
    // First try in executable directory (for deployed plugin)
    auto exeDir = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory();
    for (const auto& name : logoNames) {
        logoFile = exeDir.getChildFile(name);
        if (logoFile.existsAsFile()) {
            break;
        }
    }
    
    // If not found, try relative to source directory (for development)
    if (!logoFile.existsAsFile()) {
        auto sourceDir = juce::File(__FILE__).getParentDirectory().getParentDirectory().getParentDirectory();
        for (const auto& name : logoNames) {
            logoFile = sourceDir.getChildFile(name);
            if (logoFile.existsAsFile()) {
                break;
            }
        }
    }
    
    if (logoFile.existsAsFile()) {
        logoImage = juce::ImageFileFormat::loadFrom(logoFile);
    }
}

void PluginEditor::timerCallback() {
    // Update meters - let the meters handle their own smoothing and dead zones
    meterPanel.setInputLevel(processorRef.getInputLevel());
    meterPanel.setOutputLevel(processorRef.getOutputLevel());
    meterPanel.setCompressorGR(processorRef.getCompressorGainReduction());
    meterPanel.setGateGR(processorRef.getGateGainReduction());
    meterPanel.setLimiterGR(processorRef.getLimiterGainReduction());
    meterPanel.setTruePeak(processorRef.getTruePeak());
    
    // Update per-band GR meters in band control panels
    for (int i = 0; i < Constants::numEQBands; ++i) {
        const float currentGR = processorRef.getBandGainReduction(i);
        bandPanels[static_cast<size_t>(i)]->setGainReduction(currentGR);
    }

    // Periodic repaint for EQ curve and latency update
    static int repaintCounter = 0;
    if (++repaintCounter >= 4) {  // Repaint every 4 callbacks (5Hz)
        eqCurveDisplay.repaint();
        updateLatencyDisplay();
        repaintCounter = 0;
    }
}

void PluginEditor::paint(juce::Graphics& g) {
    // Use software rendering with high-quality settings
    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);

    // Pure black background (sci-fi theme)
    g.fillAll(SeshLookAndFeel::Colors::background);

    // Header area background
    auto headerArea = getLocalBounds().removeFromTop(55);
    g.setColour(SeshLookAndFeel::Colors::background.brighter(0.05f));
    g.fillRect(headerArea);

    // Header border
    g.setColour(SeshLookAndFeel::Colors::accent.withAlpha(0.3f));
    g.drawLine(0.0f, 55.0f, static_cast<float>(getWidth()), 55.0f, 1.5f);

    // Title - Just "QUANTA" on the left (no brand)
    g.setColour(SeshLookAndFeel::Colors::accent);
    g.setFont(juce::Font(26.0f).boldened());
    g.drawText("QUANTA", 20, 12, 150, 30, juce::Justification::centredLeft);

    // Subtitle
    g.setColour(SeshLookAndFeel::Colors::textSecondary);
    g.setFont(juce::Font(10.0f));
    g.drawText("Multiband Dynamic EQ", 20, 38, 150, 14, juce::Justification::centredLeft);
}

void PluginEditor::paintOverChildren(juce::Graphics& g) {
    // Draw logo centered in header (on top of all components)
    if (!logoImage.isValid())
        return;

    const int headerHeight = 55;
    const float logoHeight = 35.0f;
    const float logoAspect = static_cast<float>(logoImage.getWidth()) / static_cast<float>(logoImage.getHeight());
    const float logoWidth = logoHeight * logoAspect;

    // Center logo horizontally in header
    const float logoX = (getWidth() - logoWidth) * 0.5f;
    const float logoY = (headerHeight - logoHeight) * 0.5f;

    juce::Rectangle<float> logoBounds(logoX, logoY, logoWidth, logoHeight);
    g.drawImage(logoImage, logoBounds, juce::RectanglePlacement::centred);
}

void PluginEditor::resized() {
    auto bounds = getLocalBounds();
    const int padding = 12;
    const int headerHeight = 55;
    const int meterPanelHeight = 90;
    const int eqBandHeight = 240;  // Increased for larger knobs
    const int dynamicsHeight = 170;

    // Header area (not reduced by padding)
    auto headerArea = bounds.removeFromTop(headerHeight);
    headerArea.reduce(padding, 8);

    // Preset controls on the RIGHT side of header (like Aetheri)
    auto presetArea = headerArea.removeFromRight(280);
    presetArea.reduce(5, 4);
    auto presetRow = presetArea.removeFromTop(24);
    presetCombo.setBounds(presetRow.removeFromLeft(140).reduced(2, 0));
    presetRow.removeFromLeft(4);
    savePresetButton.setBounds(presetRow.removeFromLeft(50).reduced(2, 0));
    presetRow.removeFromLeft(4);
    bypassButton.setBounds(presetRow.removeFromLeft(65).reduced(2, 0));
    latencyLabel.setBounds(presetArea.removeFromTop(16).reduced(4, 0));
    presetLabel.setVisible(false);  // Hide the preset label (redundant)

    // Logo is drawn centered via paintOverChildren - hide the logoComponent
    logoComponent.setVisible(false);

    // Website link below header on right
    websiteLink.setBounds(headerArea.removeFromRight(100).reduced(2, 4));

    // Title area on left is drawn via paint() - skip that space
    headerArea.removeFromLeft(180);

    // Advanced mode toggles in remaining header center
    auto modesArea = headerArea;
    const int toggleWidth = 85;
    const int toggleHeight = 20;

    auto toggleRow = modesArea.removeFromTop(toggleHeight).reduced(0, 2);
    linearPhaseButton.setBounds(toggleRow.removeFromLeft(toggleWidth).reduced(2, 0));
    midSideButton.setBounds(toggleRow.removeFromLeft(toggleWidth).reduced(2, 0));
    dynamicEQButton.setBounds(toggleRow.removeFromLeft(toggleWidth).reduced(2, 0));

    // Oversampling dropdown on second row
    auto osRow = modesArea.removeFromTop(toggleHeight).reduced(0, 2);
    oversamplingLabel.setBounds(osRow.removeFromLeft(70).reduced(2, 0));
    oversamplingCombo.setBounds(osRow.removeFromLeft(70).reduced(2, 0));

    // Main content area
    bounds.reduce(padding, 0);

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

    const int globalKnobWidth = 55;

    // Input/Output/Mix knobs (bypass moved to header)
    auto knobRow = globalArea;
    
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
    
    // Limiter (True Peak Limiter with Threshold + Ceiling + Release)
    auto limArea = dynamicsArea.reduced(2, 0);
    limiterGroup.setBounds(limArea);

    auto limInner = limArea.reduced(8, 18);
    limiterEnableButton.setBounds(limInner.removeFromTop(22));
    limInner.removeFromTop(4);

    auto limRow = limInner;
    const int limKnobW = limRow.getWidth() / 3;

    auto limThreshArea = limRow.removeFromLeft(limKnobW);
    limThreshLabel.setBounds(limThreshArea.removeFromTop(12));
    limiterThresholdSlider.setBounds(limThreshArea);

    auto limCeilingArea = limRow.removeFromLeft(limKnobW);
    limCeilingLabel.setBounds(limCeilingArea.removeFromTop(12));
    limiterCeilingSlider.setBounds(limCeilingArea);

    auto limReleaseArea = limRow;
    limReleaseLabel.setBounds(limReleaseArea.removeFromTop(12));
    limiterReleaseSlider.setBounds(limReleaseArea);
}

void PluginEditor::refreshPresetList() {
    presetCombo.clear(juce::dontSendNotification);

    auto& presetMgr = processorRef.getPresetManager();
    auto names = presetMgr.getAllPresetNames();

    int itemId = 1;
    for (const auto& name : names) {
        if (name == "---") {
            presetCombo.addSeparator();
        } else {
            presetCombo.addItem(name, itemId);
        }
        itemId++;
    }

    // Select current preset
    const auto currentName = presetMgr.getCurrentPresetName();
    for (int i = 0; i < names.size(); ++i) {
        if (names[i] == currentName) {
            presetCombo.setSelectedItemIndex(i, juce::dontSendNotification);
            break;
        }
    }
}

void PluginEditor::saveCurrentPreset() {
    auto& presetMgr = processorRef.getPresetManager();

    // Show save dialog
    auto nameEditor = std::make_unique<juce::AlertWindow>(
        "Save Preset",
        "Enter preset name:",
        juce::MessageBoxIconType::NoIcon);

    nameEditor->addTextEditor("presetName", presetMgr.getCurrentPresetName(), "Name:");
    nameEditor->addButton("Save", 1);
    nameEditor->addButton("Cancel", 0);

    nameEditor->enterModalState(true, juce::ModalCallbackFunction::create(
        [this, &presetMgr](int result) {
            if (result == 1) {
                auto* window = dynamic_cast<juce::AlertWindow*>(juce::Component::getCurrentlyModalComponent());
                if (window) {
                    auto name = window->getTextEditorContents("presetName");
                    if (name.isNotEmpty()) {
                        presetMgr.savePreset(name);
                        refreshPresetList();
                    }
                }
            }
        }), true);
}

void PluginEditor::updateLatencyDisplay() {
    const int latency = processorRef.getLatencySamples();
    if (latency > 0) {
        latencyLabel.setText(juce::String(latency) + " samples", juce::dontSendNotification);
    } else {
        latencyLabel.setText("0 samples", juce::dontSendNotification);
    }
}

} // namespace SeshEQ
