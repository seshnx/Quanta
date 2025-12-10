#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "utils/FFTProcessor.h"

namespace SeshEQ {

/**
 * @brief Real-time spectrum analyzer display
 * 
 * Displays FFT spectrum with:
 * - Logarithmic frequency scale
 * - dB magnitude scale
 * - Pre/Post comparison
 * - Gradient fill
 */
class SpectrumAnalyzer : public juce::Component,
                          private juce::Timer {
public:
    SpectrumAnalyzer();
    ~SpectrumAnalyzer() override;
    
    /**
     * @brief Set the FFT processor to visualize
     */
    void setFFTProcessor(FFTProcessor* processor);
    
    /**
     * @brief Set pre and post FFT processors for comparison
     */
    void setFFTProcessors(FFTProcessor* pre, FFTProcessor* post);
    
    /**
     * @brief Set display colors
     */
    void setColors(juce::Colour fill, juce::Colour outline);
    void setPrePostColors(juce::Colour preColor, juce::Colour postColor);
    
    /**
     * @brief Show/hide pre-EQ spectrum
     */
    void setShowPreSpectrum(bool show) { showPreSpectrum = show; }
    
    /**
     * @brief Set the frequency range to display
     */
    void setFrequencyRange(float minHz, float maxHz);
    
    /**
     * @brief Set the dB range to display
     */
    void setDbRange(float minDb, float maxDb);
    
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;
    
private:
    void timerCallback() override;
    
    // Coordinate conversion
    float frequencyToX(float frequency) const;
    float xToFrequency(float x) const;
    float dbToY(float db) const;
    float yToDb(float y) const;
    
    // Draw helpers
    void drawBackground(juce::Graphics& g);
    void drawGrid(juce::Graphics& g);
    void drawSpectrum(juce::Graphics& g, FFTProcessor* fft, juce::Colour color, bool fill);
    juce::Path createSpectrumPath(FFTProcessor* fft);
    
    // FFT processors
    FFTProcessor* fftProcessor = nullptr;
    FFTProcessor* preFFT = nullptr;
    FFTProcessor* postFFT = nullptr;
    
    // Display settings
    float minFreq = 20.0f;
    float maxFreq = 20000.0f;
    float minDb = -90.0f;
    float maxDb = 6.0f;
    
    // Colors
    juce::Colour fillColor { 0x40ffffff };
    juce::Colour outlineColor { 0xffffffff };
    juce::Colour preColor { 0x60808080 };
    juce::Colour postColor { 0x8000ff88 };
    juce::Colour gridColor { 0x30ffffff };
    juce::Colour textColor { 0x80ffffff };
    
    bool showPreSpectrum = true;
    
    // Cached bounds
    juce::Rectangle<float> plotBounds;
    
    static constexpr int refreshRateHz = 30;
};

} // namespace SeshEQ
