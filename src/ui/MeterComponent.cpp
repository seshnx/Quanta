#include "MeterComponent.h"

namespace SeshEQ {

//==============================================================================
// LevelMeter
//==============================================================================

LevelMeter::LevelMeter(Orientation orient)
    : orientation(orient) {
    startTimerHz(30);
}

LevelMeter::~LevelMeter() {
    stopTimer();
}

void LevelMeter::setLevel(float dB) {
    currentLevel = dB;
    
    if (dB > peakLevel) {
        peakLevel = dB;
        peakHoldCounter = peakHoldTime * 30 / 1000;  // Convert ms to frames at 30Hz
    }
}

void LevelMeter::setRange(float min, float max) {
    minDb = min;
    maxDb = max;
}

void LevelMeter::setColors(juce::Colour background, juce::Colour low, 
                            juce::Colour mid, juce::Colour high, juce::Colour peak) {
    bgColor = background;
    lowColor = low;
    midColor = mid;
    highColor = high;
    peakColor = peak;
}

void LevelMeter::setPeakHold(bool enable, int holdTimeMs) {
    peakHoldEnabled = enable;
    peakHoldTime = holdTimeMs;
}

void LevelMeter::timerCallback() {
    // Smooth the level
    smoothedLevel = smoothedLevel * smoothingCoef + currentLevel * (1.0f - smoothingCoef);
    
    // Decay peak hold
    if (peakHoldEnabled && peakHoldCounter > 0) {
        --peakHoldCounter;
    } else {
        peakLevel = std::max(peakLevel - 1.0f, currentLevel);
    }
    
    repaint();
}

float LevelMeter::dbToNormalized(float db) const {
    return std::clamp((db - minDb) / (maxDb - minDb), 0.0f, 1.0f);
}

void LevelMeter::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    
    // Background
    g.setColour(bgColor);
    g.fillRoundedRectangle(bounds, 2.0f);
    
    // Calculate level position
    const float levelNorm = dbToNormalized(smoothedLevel);
    const float peakNorm = dbToNormalized(peakLevel);
    const float midNorm = dbToNormalized(midThreshold);
    const float highNorm = dbToNormalized(highThreshold);
    
    if (orientation == Orientation::Vertical) {
        const float meterHeight = bounds.getHeight() * levelNorm;
        
        // Create gradient for meter
        juce::ColourGradient gradient(
            lowColor, 0, bounds.getBottom(),
            highColor, 0, bounds.getY(),
            false
        );
        gradient.addColour(midNorm, midColor);
        gradient.addColour(highNorm, highColor);
        
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(bounds.getX(), bounds.getBottom() - meterHeight,
                               bounds.getWidth(), meterHeight, 2.0f);
        
        // Peak indicator
        if (peakHoldEnabled) {
            const float peakY = bounds.getBottom() - bounds.getHeight() * peakNorm;
            g.setColour(peakColor);
            g.fillRect(bounds.getX(), peakY - 2, bounds.getWidth(), 2.0f);
        }
    } else {
        const float meterWidth = bounds.getWidth() * levelNorm;
        
        juce::ColourGradient gradient(
            lowColor, bounds.getX(), 0,
            highColor, bounds.getRight(), 0,
            false
        );
        gradient.addColour(midNorm, midColor);
        gradient.addColour(highNorm, highColor);
        
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(bounds.getX(), bounds.getY(),
                               meterWidth, bounds.getHeight(), 2.0f);
        
        // Peak indicator
        if (peakHoldEnabled) {
            const float peakX = bounds.getX() + bounds.getWidth() * peakNorm;
            g.setColour(peakColor);
            g.fillRect(peakX - 1, bounds.getY(), 2.0f, bounds.getHeight());
        }
    }
    
    // Border
    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.drawRoundedRectangle(bounds, 2.0f, 1.0f);
}

void LevelMeter::resized() {
    // Nothing to do
}

//==============================================================================
// StereoMeter
//==============================================================================

StereoMeter::StereoMeter() {
    addAndMakeVisible(leftMeter);
    addAndMakeVisible(rightMeter);
}

void StereoMeter::setLevels(float leftDb, float rightDb) {
    leftMeter.setLevel(leftDb);
    rightMeter.setLevel(rightDb);
}

void StereoMeter::setRange(float minDb, float maxDb) {
    leftMeter.setRange(minDb, maxDb);
    rightMeter.setRange(minDb, maxDb);
}

void StereoMeter::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff1a1a2e));
}

void StereoMeter::resized() {
    auto bounds = getLocalBounds();
    const int gap = 2;
    const int meterWidth = (bounds.getWidth() - gap) / 2;
    
    leftMeter.setBounds(bounds.removeFromLeft(meterWidth));
    bounds.removeFromLeft(gap);
    rightMeter.setBounds(bounds);
}

//==============================================================================
// GainReductionMeter
//==============================================================================

GainReductionMeter::GainReductionMeter() {
    addAndMakeVisible(valueLabel);
    valueLabel.setJustificationType(juce::Justification::centred);
    valueLabel.setColour(juce::Label::textColourId, textColor);
    valueLabel.setFont(juce::Font(11.0f));
    
    startTimerHz(30);
}

GainReductionMeter::~GainReductionMeter() {
    stopTimer();
}

void GainReductionMeter::setGainReduction(float dB) {
    currentGR = std::min(0.0f, dB);  // GR should be negative or zero
    
    if (currentGR < peakGR) {
        peakGR = currentGR;
        peakHoldCounter = peakHoldTime;
    }
}

void GainReductionMeter::setRange(float maxReductionDb) {
    maxRange = maxReductionDb;
}

void GainReductionMeter::setColor(juce::Colour color) {
    meterColor = color;
}

void GainReductionMeter::timerCallback() {
    // Smooth the GR
    smoothedGR = smoothedGR * 0.7f + currentGR * 0.3f;
    
    // Decay peak hold
    if (peakHoldCounter > 0) {
        --peakHoldCounter;
    } else {
        peakGR = std::min(peakGR + 0.5f, currentGR);
    }
    
    // Update label
    juce::String text = (currentGR < -0.1f) 
        ? juce::String(currentGR, 1) + " dB"
        : "0 dB";
    valueLabel.setText(text, juce::dontSendNotification);
    
    repaint();
}

void GainReductionMeter::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    auto meterBounds = bounds.removeFromTop(bounds.getHeight() - 18);
    
    // Background
    g.setColour(bgColor);
    g.fillRoundedRectangle(meterBounds, 3.0f);
    
    // Meter (fills from right to left to show reduction)
    if (smoothedGR < -0.1f) {
        const float grNorm = std::clamp(-smoothedGR / -maxRange, 0.0f, 1.0f);
        const float meterWidth = meterBounds.getWidth() * grNorm;
        
        g.setColour(meterColor);
        g.fillRoundedRectangle(
            meterBounds.getRight() - meterWidth, meterBounds.getY(),
            meterWidth, meterBounds.getHeight(),
            3.0f
        );
    }
    
    // Peak indicator
    if (peakGR < -0.1f) {
        const float peakNorm = std::clamp(-peakGR / -maxRange, 0.0f, 1.0f);
        const float peakX = meterBounds.getRight() - meterBounds.getWidth() * peakNorm;
        
        g.setColour(juce::Colours::white);
        g.fillRect(peakX - 1, meterBounds.getY(), 2.0f, meterBounds.getHeight());
    }
    
    // Border
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.drawRoundedRectangle(meterBounds, 3.0f, 1.0f);
}

void GainReductionMeter::resized() {
    auto bounds = getLocalBounds();
    valueLabel.setBounds(bounds.removeFromBottom(18));
}

//==============================================================================
// DynamicsMeterPanel
//==============================================================================

DynamicsMeterPanel::DynamicsMeterPanel() {
    addAndMakeVisible(compMeter);
    addAndMakeVisible(gateMeter);
    addAndMakeVisible(limiterMeter);
    addAndMakeVisible(inputMeter);
    addAndMakeVisible(outputMeter);
    
    compMeter.setColor(juce::Colour(0xffff6b6b));
    gateMeter.setColor(juce::Colour(0xffffd43b));
    limiterMeter.setColor(juce::Colour(0xff4dabf7));
    
    for (auto* label : { &compLabel, &gateLabel, &limiterLabel, &inputLabel, &outputLabel }) {
        addAndMakeVisible(label);
        label->setJustificationType(juce::Justification::centred);
        label->setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.7f));
        label->setFont(juce::Font(10.0f));
    }
}

void DynamicsMeterPanel::setCompressorGR(float dB) {
    compMeter.setGainReduction(dB);
}

void DynamicsMeterPanel::setGateGR(float dB) {
    gateMeter.setGainReduction(dB);
}

void DynamicsMeterPanel::setLimiterGR(float dB) {
    limiterMeter.setGainReduction(dB);
}

void DynamicsMeterPanel::setInputLevel(float dB) {
    inputMeter.setLevel(dB);
}

void DynamicsMeterPanel::setOutputLevel(float dB) {
    outputMeter.setLevel(dB);
}

void DynamicsMeterPanel::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff1a1a2e));
    
    // Section dividers
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    
    auto bounds = getLocalBounds();
    const int sectionWidth = bounds.getWidth() / 5;
    
    for (int i = 1; i < 5; ++i) {
        g.drawVerticalLine(sectionWidth * i, 0.0f, static_cast<float>(bounds.getHeight()));
    }
}

void DynamicsMeterPanel::resized() {
    auto bounds = getLocalBounds().reduced(4);
    const int sectionWidth = bounds.getWidth() / 5;
    const int labelHeight = 14;
    const int meterHeight = bounds.getHeight() - labelHeight - 4;
    
    // Input meter
    auto inputArea = bounds.removeFromLeft(sectionWidth).reduced(2);
    inputLabel.setBounds(inputArea.removeFromBottom(labelHeight));
    inputMeter.setBounds(inputArea);
    
    // Compressor GR
    auto compArea = bounds.removeFromLeft(sectionWidth).reduced(2);
    compLabel.setBounds(compArea.removeFromBottom(labelHeight));
    compMeter.setBounds(compArea);
    
    // Gate GR
    auto gateArea = bounds.removeFromLeft(sectionWidth).reduced(2);
    gateLabel.setBounds(gateArea.removeFromBottom(labelHeight));
    gateMeter.setBounds(gateArea);
    
    // Limiter GR
    auto limArea = bounds.removeFromLeft(sectionWidth).reduced(2);
    limiterLabel.setBounds(limArea.removeFromBottom(labelHeight));
    limiterMeter.setBounds(limArea);
    
    // Output meter
    auto outputArea = bounds.reduced(2);
    outputLabel.setBounds(outputArea.removeFromBottom(labelHeight));
    outputMeter.setBounds(outputArea);
}

} // namespace SeshEQ
