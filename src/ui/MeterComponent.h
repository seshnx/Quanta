#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <array>

namespace SeshEQ {

/**
 * @brief Level meter component with peak hold
 */
class LevelMeter : public juce::Component,
                   private juce::Timer {
public:
    enum class Orientation { Vertical, Horizontal };
    
    LevelMeter(Orientation orient = Orientation::Vertical);
    ~LevelMeter() override;
    
    /**
     * @brief Set the current level in dB
     */
    void setLevel(float dB);
    
    /**
     * @brief Set the range in dB
     */
    void setRange(float minDb, float maxDb);
    
    /**
     * @brief Set meter colors
     */
    void setColors(juce::Colour background, juce::Colour low, 
                   juce::Colour mid, juce::Colour high, juce::Colour peak);
    
    /**
     * @brief Enable/disable peak hold
     */
    void setPeakHold(bool enable, int holdTimeMs = 2000);
    
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;
    
private:
    void timerCallback() override;
    float dbToNormalized(float db) const;
    
    Orientation orientation;
    
    float currentLevel = -100.0f;
    float peakLevel = -100.0f;
    float minDb = -60.0f;
    float maxDb = 6.0f;
    
    bool peakHoldEnabled = true;
    int peakHoldTime = 2000;
    int peakHoldCounter = 0;
    
    // Smoothing (increased for smoother display)
    float smoothedLevel = -100.0f;
    float smoothingCoef = 0.7f;  // Increased for smoother, less jittery updates
    
    // Colors
    juce::Colour bgColor { 0xff1a1a2e };
    juce::Colour lowColor { 0xff00ff88 };
    juce::Colour midColor { 0xffffff00 };
    juce::Colour highColor { 0xffff4444 };
    juce::Colour peakColor { 0xffffffff };
    
    // Threshold for color transitions (in dB)
    float midThreshold = -12.0f;
    float highThreshold = -3.0f;
};

/**
 * @brief Stereo level meter (two meters side by side)
 */
class StereoMeter : public juce::Component {
public:
    StereoMeter();
    
    void setLevels(float leftDb, float rightDb);
    void setRange(float minDb, float maxDb);
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
private:
    LevelMeter leftMeter;
    LevelMeter rightMeter;
};

/**
 * @brief Gain reduction meter (shows compression amount)
 */
class GainReductionMeter : public juce::Component,
                           private juce::Timer {
public:
    GainReductionMeter();
    ~GainReductionMeter() override;
    
    /**
     * @brief Set current gain reduction in dB (negative value)
     */
    void setGainReduction(float dB);
    
    /**
     * @brief Set the maximum gain reduction to display
     */
    void setRange(float maxReductionDb);
    
    /**
     * @brief Set meter color
     */
    void setColor(juce::Colour color);
    
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;
    
private:
    void timerCallback() override;
    
    float currentGR = 0.0f;
    float peakGR = 0.0f;
    float smoothedGR = 0.0f;
    float maxRange = -24.0f;  // Max reduction to show
    
    int peakHoldCounter = 0;
    int peakHoldTime = 60;  // frames at 30Hz
    
    juce::Colour meterColor { 0xffff6b6b };
    juce::Colour bgColor { 0xff2d2d44 };
    juce::Colour textColor { 0xffffffff };
    
    juce::Label valueLabel;
};

/**
 * @brief True Peak meter component
 */
class TruePeakMeter : public juce::Component,
                      private juce::Timer {
public:
    TruePeakMeter();
    ~TruePeakMeter() override;
    
    /**
     * @brief Set current True Peak level in dB
     */
    void setTruePeak(float dB);
    
    /**
     * @brief Set the range in dB
     */
    void setRange(float minDb, float maxDb);
    
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;
    
private:
    void timerCallback() override;
    
    float currentLevel = -100.0f;
    float peakLevel = -100.0f;
    float minDb = -60.0f;
    float maxDb = 6.0f;
    
    int peakHoldCounter = 0;
    int peakHoldTime = 60;  // frames at 30Hz
    
    float smoothedLevel = -100.0f;
    float smoothingCoef = 0.7f;  // Increased for smoother, less jittery updates
    
    juce::Label valueLabel;
    juce::Label titleLabel { {}, "TRUE PEAK" };
};

/**
 * @brief Combined dynamics meter panel
 */
class DynamicsMeterPanel : public juce::Component {
public:
    DynamicsMeterPanel();
    
    void setCompressorGR(float dB);
    void setGateGR(float dB);
    void setLimiterGR(float dB);
    void setInputLevel(float dB);
    void setOutputLevel(float dB);
    void setTruePeak(float dB);
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
private:
    GainReductionMeter compMeter;
    GainReductionMeter gateMeter;
    GainReductionMeter limiterMeter;
    LevelMeter inputMeter;
    LevelMeter outputMeter;
    TruePeakMeter truePeakMeter;
    
    juce::Label compLabel { {}, "COMP" };
    juce::Label gateLabel { {}, "GATE" };
    juce::Label limiterLabel { {}, "LIM" };
    juce::Label inputLabel { {}, "IN" };
    juce::Label outputLabel { {}, "OUT" };
};

} // namespace SeshEQ
