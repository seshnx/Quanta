#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "dsp/EQProcessor.h"
#include "utils/Parameters.h"
#include <functional>

namespace SeshEQ {

/**
 * @brief Interactive EQ curve display with draggable band nodes
 * 
 * Features:
 * - Displays combined EQ response curve
 * - Draggable nodes for each band
 * - Frequency/dB grid overlay
 * - Mouse wheel for Q adjustment
 * - Double-click to reset band
 */
class EQCurveDisplay : public juce::Component {
public:
    EQCurveDisplay();
    
    /**
     * @brief Set the EQ processor to visualize
     */
    void setEQProcessor(const EQProcessor* processor);
    
    /**
     * @brief Connect to APVTS for parameter control
     */
    void connectToParameters(juce::AudioProcessorValueTreeState& apvts);
    
    /**
     * @brief Set the frequency range
     */
    void setFrequencyRange(float minHz, float maxHz);
    
    /**
     * @brief Set the dB range
     */
    void setDbRange(float minDb, float maxDb);
    
    /**
     * @brief Set callback for when a band is selected
     */
    void setOnBandSelected(std::function<void(int)> callback) { onBandSelected = callback; }
    
    /**
     * @brief Get currently selected band (-1 if none)
     */
    int getSelectedBand() const { return selectedBand; }
    
    /**
     * @brief Set selected band
     */
    void setSelectedBand(int band);
    
    /**
     * @brief Set gain reduction for a band (for metering)
     */
    void setBandGainReduction(int bandIndex, float dB);
    
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    
private:
    // Coordinate conversion
    float frequencyToX(float frequency) const;
    float xToFrequency(float x) const;
    float dbToY(float db) const;
    float yToDb(float y) const;
    
    // Drawing helpers
    void drawGrid(juce::Graphics& g);
    void drawCurve(juce::Graphics& g);
    void drawBandCurves(juce::Graphics& g);
    void drawNodes(juce::Graphics& g);
    void drawBandGRMeters(juce::Graphics& g);
    juce::Path createResponseCurve() const;
    juce::Path createBandCurve(int bandIndex) const;
    
    // Node hit testing
    int getNodeAtPosition(juce::Point<float> pos) const;
    juce::Point<float> getNodePosition(int bandIndex) const;
    
    // EQ processor reference
    const EQProcessor* eqProcessor = nullptr;
    
    // Parameter state
    juce::AudioProcessorValueTreeState* apvts = nullptr;
    
    // Display settings
    float minFreq = 20.0f;
    float maxFreq = 20000.0f;
    float minDb = -24.0f;
    float maxDb = 24.0f;
    
    // Interaction state
    int selectedBand = -1;
    int hoveredBand = -1;
    int draggingBand = -1;
    juce::Point<float> dragStartPos;
    float dragStartFreq = 0.0f;
    float dragStartGain = 0.0f;
    
    // Colors (sci-fi theme)
    juce::Colour curveColor { 0xffffffff };  // White
    juce::Colour fillColor { 0x2000ffff };  // Cyan with transparency
    juce::Colour gridColor { 0x3000ffff };  // Cyan grid
    juce::Colour textColor { 0x8000ffff };  // Cyan text
    juce::Colour nodeColor { 0xffffffff };  // White
    juce::Colour nodeSelectedColor { 0xff00ffff };  // Cyan
    juce::Colour nodeHoverColor { 0xff88ffff };  // Light Cyan
    
    // Band colors for individual curves (sci-fi cyan theme)
    std::array<juce::Colour, Constants::numEQBands> bandColors = {
        juce::Colour(0xff00ffff),  // Band 1 - Pure Cyan
        juce::Colour(0xff00cccc),  // Band 2 - Dark Cyan
        juce::Colour(0xff88ffff),  // Band 3 - Light Cyan
        juce::Colour(0xff00ffcc),  // Band 4 - Cyan-Green
        juce::Colour(0xff00ccff),  // Band 5 - Cyan-Blue
        juce::Colour(0xffccffff),  // Band 6 - Very Light Cyan
        juce::Colour(0xff66ffff),  // Band 7 - Medium Cyan
        juce::Colour(0xff00ffff),  // Band 8 - Pure Cyan
    };
    
    // Per-band gain reduction values (for metering)
    std::array<float, Constants::numEQBands> bandGainReduction = { 0.0f };
    
    // Node radius
    static constexpr float nodeRadius = 8.0f;
    static constexpr float nodeHitRadius = 12.0f;
    
    // Cached bounds
    juce::Rectangle<float> plotBounds;
    
    // Cached paths to avoid recalculating on every repaint
    mutable juce::Path cachedResponsePath;
    mutable std::array<juce::Path, Constants::numEQBands> cachedBandPaths;
    mutable bool responsePathValid = false;
    mutable std::array<bool, Constants::numEQBands> bandPathsValid = { false };
    mutable std::array<EQProcessor::BandParams, Constants::numEQBands> lastBandParams;
    
    // Callback
    std::function<void(int)> onBandSelected;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EQCurveDisplay)
};

} // namespace SeshEQ
