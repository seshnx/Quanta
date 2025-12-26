#include "MeterComponent.h"

namespace SeshEQ {

//==============================================================================
// LevelMeter
//==============================================================================

LevelMeter::LevelMeter(Orientation orient)
    : orientation(orient) {
    startTimerHz(20);  // Balanced update rate
}

LevelMeter::~LevelMeter() {
    stopTimer();
}

void LevelMeter::setLevel(float dB) {
    currentLevel = dB;
    
    if (dB > peakLevel) {
        peakLevel = dB;
        peakHoldCounter = peakHoldTime * 20 / 1000;  // Convert ms to frames at 20Hz
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
    // Smooth the level with adaptive smoothing
    const float targetLevel = currentLevel;
    const float diff = std::abs(targetLevel - smoothedLevel);
    
    // Use heavier smoothing when close to target (idle state) to reduce jitter
    // But not so heavy that it becomes unresponsive
    const float adaptiveCoef = (diff < 0.1f) ? 0.92f : ((diff < 0.5f) ? 0.85f : smoothingCoef);
    smoothedLevel = smoothedLevel * adaptiveCoef + targetLevel * (1.0f - adaptiveCoef);
    
    // Only repaint if change is significant (dead zone to prevent jitter)
    static float lastPaintedLevel = -1000.0f;
    const float paintThreshold = 0.1f;  // Reasonable threshold
    if (std::abs(smoothedLevel - lastPaintedLevel) > paintThreshold) {
        lastPaintedLevel = smoothedLevel;
        repaint();
    }
    
    // Decay peak hold
    if (peakHoldEnabled && peakHoldCounter > 0) {
        --peakHoldCounter;
    } else {
        peakLevel = std::max(peakLevel - 0.3f, currentLevel);
    }
}

float LevelMeter::dbToNormalized(float db) const {
    return std::clamp((db - minDb) / (maxDb - minDb), 0.0f, 1.0f);
}

void LevelMeter::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    
    // Use software rendering with high-quality settings
    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    
    // Background
    g.setColour(bgColor);
    g.fillRoundedRectangle(bounds, 2.0f);
    
    // Calculate level position - Always VERTICAL
    const float levelNorm = dbToNormalized(smoothedLevel);
    const float peakNorm = dbToNormalized(peakLevel);
    const float midNorm = dbToNormalized(midThreshold);
    const float highNorm = dbToNormalized(highThreshold);
    
    const float meterHeight = bounds.getHeight() * levelNorm;
    
    // Create gradient for meter (bottom to top)
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
    
    // Peak indicator (horizontal line at peak level)
    if (peakHoldEnabled) {
        const float peakY = bounds.getBottom() - bounds.getHeight() * peakNorm;
        g.setColour(peakColor);
        g.fillRect(bounds.getX(), peakY - 1.0f, bounds.getWidth(), 2.0f);
    }
    
    // Border (cyan for sci-fi theme)
    g.setColour(juce::Colour(0xff00ffff).withAlpha(0.2f));
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
    
    startTimerHz(20);  // Balanced update rate
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
    // Smooth the GR with adaptive smoothing
    const float targetGR = currentGR;
    const float diff = std::abs(targetGR - smoothedGR);
    
    // Use heavier smoothing when close to target (idle state) to reduce jitter
    const float adaptiveCoef = (diff < 0.1f) ? 0.92f : ((diff < 0.5f) ? 0.85f : 0.8f);
    smoothedGR = smoothedGR * adaptiveCoef + targetGR * (1.0f - adaptiveCoef);
    
    // Only repaint if change is significant (dead zone to prevent jitter)
    static float lastPaintedGR = 0.0f;
    const float paintThreshold = 0.1f;  // Reasonable threshold
    if (std::abs(smoothedGR - lastPaintedGR) > paintThreshold) {
        lastPaintedGR = smoothedGR;
        repaint();
    }
    
    // Decay peak hold
    if (peakHoldCounter > 0) {
        --peakHoldCounter;
    } else {
        peakGR = std::min(peakGR + 0.3f, currentGR);
    }
    
    // Update label less frequently to reduce CPU
    static int labelUpdateCounter = 0;
    if (++labelUpdateCounter >= 3) {  // Update label every 3 callbacks (~7Hz)
        juce::String text = (smoothedGR < -0.1f) 
            ? juce::String(smoothedGR, 1) + " dB"
            : "0 dB";
        valueLabel.setText(text, juce::dontSendNotification);
        labelUpdateCounter = 0;
    }
}

void GainReductionMeter::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    auto meterBounds = bounds.removeFromTop(bounds.getHeight() - 18);
    
    // Use software rendering with high-quality settings
    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    
    // Background
    g.setColour(bgColor);
    g.fillRoundedRectangle(meterBounds, 3.0f);
    
    // Meter - VERTICAL (fills from bottom up to show reduction)
    if (smoothedGR < -0.1f) {
        const float grNorm = std::clamp(-smoothedGR / -maxRange, 0.0f, 1.0f);
        const float meterHeight = meterBounds.getHeight() * grNorm;
        
        // Gradient for gain reduction meter (bottom to top)
        juce::ColourGradient gradient(
            meterColor.withAlpha(0.7f), 0, meterBounds.getBottom(),
            meterColor, 0, meterBounds.getBottom() - meterHeight,
            false
        );
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(
            meterBounds.getX(), meterBounds.getBottom() - meterHeight,
            meterBounds.getWidth(), meterHeight,
            3.0f
        );
    }
    
    // Peak indicator - VERTICAL (horizontal line at peak level)
    if (peakGR < -0.1f) {
        const float peakNorm = std::clamp(-peakGR / -maxRange, 0.0f, 1.0f);
        const float peakY = meterBounds.getBottom() - meterBounds.getHeight() * peakNorm;
        
        g.setColour(juce::Colours::white);
        g.fillRect(meterBounds.getX(), peakY - 1.0f, meterBounds.getWidth(), 2.0f);
    }
    
    // Border (cyan for sci-fi theme)
    g.setColour(juce::Colour(0xff00ffff).withAlpha(0.2f));
    g.drawRoundedRectangle(meterBounds, 3.0f, 1.0f);
}

void GainReductionMeter::resized() {
    auto bounds = getLocalBounds();
    valueLabel.setBounds(bounds.removeFromBottom(18));
}

//==============================================================================
// TruePeakMeter
//==============================================================================

TruePeakMeter::TruePeakMeter() {
    addAndMakeVisible(valueLabel);
    addAndMakeVisible(titleLabel);
    
    valueLabel.setJustificationType(juce::Justification::centred);
    valueLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00ffff)); // Cyan
    valueLabel.setFont(juce::Font(11.0f).boldened());
    valueLabel.setText("-∞", juce::dontSendNotification);
    
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00ffff)); // Cyan
    titleLabel.setFont(juce::Font(9.0f));
    
    startTimerHz(20);  // Balanced update rate
}

TruePeakMeter::~TruePeakMeter() {
    stopTimer();
}

void TruePeakMeter::setTruePeak(float dB) {
    currentLevel = dB;
    
    if (dB > peakLevel) {
        peakLevel = dB;
        peakHoldCounter = peakHoldTime;
    }
}

void TruePeakMeter::setRange(float min, float max) {
    minDb = min;
    maxDb = max;
}

void TruePeakMeter::timerCallback() {
    // Smooth the level with adaptive smoothing
    const float targetLevel = currentLevel;
    const float diff = std::abs(targetLevel - smoothedLevel);
    
    // Use heavier smoothing when close to target (idle state) to reduce jitter
    const float adaptiveCoef = (diff < 0.1f) ? 0.92f : ((diff < 0.5f) ? 0.85f : smoothingCoef);
    smoothedLevel = smoothedLevel * adaptiveCoef + targetLevel * (1.0f - adaptiveCoef);
    
    // Only repaint if change is significant (dead zone to prevent jitter)
    static float lastPaintedLevel = -1000.0f;
    const float paintThreshold = 0.1f;  // Reasonable threshold
    if (std::abs(smoothedLevel - lastPaintedLevel) > paintThreshold) {
        lastPaintedLevel = smoothedLevel;
        repaint();
    }
    
    if (peakHoldCounter > 0) {
        --peakHoldCounter;
    } else {
        peakLevel = std::max(peakLevel - 0.3f, currentLevel);
    }
    
    // Update label less frequently to reduce CPU
    static int labelUpdateCounter = 0;
    if (++labelUpdateCounter >= 3) {  // Update label every 3 callbacks (~7Hz)
        if (smoothedLevel < minDb) {
            valueLabel.setText("-∞", juce::dontSendNotification);
        } else {
            valueLabel.setText(juce::String(smoothedLevel, 1) + " dB", juce::dontSendNotification);
        }
        labelUpdateCounter = 0;
    }
}

void TruePeakMeter::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    
    // Use software rendering with high-quality settings
    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    
    // Background
    g.setColour(juce::Colour(0xff000000)); // Black
    g.fillRoundedRectangle(bounds, 3.0f);
    
    // Border (cyan)
    g.setColour(juce::Colour(0xff00ffff).withAlpha(0.5f));
    g.drawRoundedRectangle(bounds, 3.0f, 1.5f);
    
    // Meter bar - VERTICAL orientation (bottom to top)
    const float levelNorm = std::clamp((smoothedLevel - minDb) / (maxDb - minDb), 0.0f, 1.0f);
    const float meterHeight = bounds.getHeight() * levelNorm;
    
    if (meterHeight > 0.0f) {
        // Gradient from cyan (bottom) to white (top)
        juce::ColourGradient gradient(
            juce::Colour(0xff00ffff), 0, bounds.getBottom(),
            juce::Colour(0xffffffff), 0, bounds.getY(),
            false
        );
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(bounds.getX(), bounds.getBottom() - meterHeight, 
                               bounds.getWidth(), meterHeight, 2.0f);
    }
    
    // Peak indicator - VERTICAL (horizontal line at peak level)
    if (peakHoldCounter > 0) {
        const float peakNorm = std::clamp((peakLevel - minDb) / (maxDb - minDb), 0.0f, 1.0f);
        const float peakY = bounds.getBottom() - bounds.getHeight() * peakNorm;
        g.setColour(juce::Colour(0xffffffff));
        g.fillRect(bounds.getX(), peakY - 1.0f, bounds.getWidth(), 2.0f);
    }
}

void TruePeakMeter::resized() {
    auto bounds = getLocalBounds();
    titleLabel.setBounds(bounds.removeFromTop(14));
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
    addAndMakeVisible(truePeakMeter);
    
    // Set meter colors (sci-fi theme - cyan variations)
    compMeter.setColor(juce::Colour(0xff00ffff)); // Cyan
    gateMeter.setColor(juce::Colour(0xff88ffff)); // Light Cyan
    limiterMeter.setColor(juce::Colour(0xff00ccff)); // Cyan-Blue
    truePeakMeter.setRange(-60.0f, 6.0f);
    
    // Ensure all level meters are vertical
    inputMeter.setRange(-60.0f, 6.0f);
    outputMeter.setRange(-60.0f, 6.0f);
    
    for (auto* label : { &compLabel, &gateLabel, &limiterLabel, &inputLabel, &outputLabel }) {
        addAndMakeVisible(label);
        label->setJustificationType(juce::Justification::centred);
        label->setColour(juce::Label::textColourId, juce::Colour(0xff00ffff).withAlpha(0.8f)); // Cyan
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

void DynamicsMeterPanel::setTruePeak(float dB) {
    truePeakMeter.setTruePeak(dB);
}

void DynamicsMeterPanel::paint(juce::Graphics& g) {
    // Use software rendering with high-quality settings
    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    
    g.fillAll(juce::Colour(0xff000000)); // Black background
    
    // Section dividers (cyan)
    g.setColour(juce::Colour(0xff00ffff).withAlpha(0.2f));
    
    auto bounds = getLocalBounds();
    const int sectionWidth = bounds.getWidth() / 6; // 6 sections now (added True Peak)
    
    for (int i = 1; i < 6; ++i) {
        g.drawVerticalLine(sectionWidth * i, 0.0f, static_cast<float>(bounds.getHeight()));
    }
}

void DynamicsMeterPanel::resized() {
    auto bounds = getLocalBounds().reduced(4);
    const int sectionWidth = bounds.getWidth() / 6; // 6 sections now
    const int labelHeight = 14;
    
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
    auto outputArea = bounds.removeFromLeft(sectionWidth).reduced(2);
    outputLabel.setBounds(outputArea.removeFromBottom(labelHeight));
    outputMeter.setBounds(outputArea);
    
    // True Peak meter
    auto truePeakArea = bounds.reduced(2);
    truePeakMeter.setBounds(truePeakArea);
}

} // namespace SeshEQ
