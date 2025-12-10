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
    
    // Colors
    juce::Colour curveColor { 0xffffffff };
    juce::Colour fillColor { 0x20ffffff };
    juce::Colour gridColor { 0x30ffffff };
    juce::Colour textColor { 0x80ffffff };
    juce::Colour nodeColor { 0xffffffff };
    juce::Colour nodeSelectedColor { 0xff00ff88 };
    juce::Colour nodeHoverColor { 0xffffcc00 };
    
    // Band colors for individual curves
    std::array<juce::Colour, Constants::numEQBands> bandColors = {
        juce::Colour(0xffff6b6b),  // Red
        juce::Colour(0xffffa94d),  // Orange
        juce::Colour(0xffffd43b),  // Yellow
        juce::Colour(0xff69db7c),  // Green
        juce::Colour(0xff4dabf7),  // Blue
        juce::Colour(0xff9775fa),  // Purple
        juce::Colour(0xfff06595),  // Pink
        juce::Colour(0xff20c997),  // Teal
    };
    
    // Node radius
    static constexpr float nodeRadius = 8.0f;
    static constexpr float nodeHitRadius = 12.0f;
    
    // Cached bounds
    juce::Rectangle<float> plotBounds;
    
    // Callback
    std::function<void(int)> onBandSelected;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EQCurveDisplay)
};

} // namespace SeshEQ
