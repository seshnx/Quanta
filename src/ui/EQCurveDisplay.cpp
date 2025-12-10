#include "EQCurveDisplay.h"
#include <cmath>

namespace SeshEQ {

EQCurveDisplay::EQCurveDisplay() {
    setOpaque(false);
}

void EQCurveDisplay::setEQProcessor(const EQProcessor* processor) {
    eqProcessor = processor;
    repaint();
}

void EQCurveDisplay::connectToParameters(juce::AudioProcessorValueTreeState& state) {
    apvts = &state;
}

void EQCurveDisplay::setFrequencyRange(float minHz, float maxHz) {
    minFreq = minHz;
    maxFreq = maxHz;
    repaint();
}

void EQCurveDisplay::setDbRange(float min, float max) {
    minDb = min;
    maxDb = max;
    repaint();
}

void EQCurveDisplay::setSelectedBand(int band) {
    if (selectedBand != band) {
        selectedBand = band;
        repaint();
        if (onBandSelected)
            onBandSelected(band);
    }
}

void EQCurveDisplay::paint(juce::Graphics& g) {
    // Background
    g.setColour(juce::Colour(0x10ffffff));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);
    
    drawGrid(g);
    drawBandCurves(g);
    drawCurve(g);
    drawNodes(g);
}

void EQCurveDisplay::resized() {
    plotBounds = getLocalBounds().toFloat().reduced(4.0f);
}

void EQCurveDisplay::drawGrid(juce::Graphics& g) {
    g.setColour(gridColor);
    
    const float left = plotBounds.getX();
    const float right = plotBounds.getRight();
    const float top = plotBounds.getY();
    const float bottom = plotBounds.getBottom();
    
    // Center line (0 dB)
    const float zeroY = dbToY(0.0f);
    g.setColour(gridColor.brighter(0.3f));
    g.drawHorizontalLine(static_cast<int>(zeroY), left, right);
    
    // Frequency lines
    g.setColour(gridColor);
    const std::array<float, 9> freqLines = { 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 };
    
    for (float freq : freqLines) {
        if (freq >= minFreq && freq <= maxFreq) {
            const float x = frequencyToX(freq);
            g.drawVerticalLine(static_cast<int>(x), top, bottom);
        }
    }
    
    // dB lines
    const std::array<float, 5> dbLines = { -18, -12, -6, 6, 12 };
    for (float db : dbLines) {
        if (db >= minDb && db <= maxDb) {
            const float y = dbToY(db);
            g.drawHorizontalLine(static_cast<int>(y), left, right);
        }
    }
}

void EQCurveDisplay::drawBandCurves(juce::Graphics& g) {
    if (!eqProcessor) return;
    
    // Draw individual band curves (faded)
    for (int i = 0; i < Constants::numEQBands; ++i) {
        auto params = eqProcessor->getBandParameters(i);
        if (!params.enabled) continue;
        
        juce::Path bandPath = createBandCurve(i);
        
        // Fill with band color
        juce::Path fillPath = bandPath;
        fillPath.lineTo(plotBounds.getRight(), dbToY(0.0f));
        fillPath.lineTo(plotBounds.getX(), dbToY(0.0f));
        fillPath.closeSubPath();
        
        g.setColour(bandColors[static_cast<size_t>(i)].withAlpha(0.1f));
        g.fillPath(fillPath);
        
        // Draw outline if selected or hovered
        if (i == selectedBand || i == hoveredBand) {
            g.setColour(bandColors[static_cast<size_t>(i)].withAlpha(0.6f));
            g.strokePath(bandPath, juce::PathStrokeType(1.5f));
        }
    }
}

void EQCurveDisplay::drawCurve(juce::Graphics& g) {
    if (!eqProcessor) return;
    
    juce::Path responsePath = createResponseCurve();
    
    // Fill under curve
    juce::Path fillPath = responsePath;
    fillPath.lineTo(plotBounds.getRight(), dbToY(0.0f));
    fillPath.lineTo(plotBounds.getX(), dbToY(0.0f));
    fillPath.closeSubPath();
    
    // Gradient fill
    juce::ColourGradient gradient(
        fillColor, 0, dbToY(maxDb),
        fillColor.withAlpha(0.0f), 0, dbToY(0.0f),
        false
    );
    g.setGradientFill(gradient);
    g.fillPath(fillPath);
    
    // Draw curve outline
    g.setColour(curveColor);
    g.strokePath(responsePath, juce::PathStrokeType(2.0f));
}

void EQCurveDisplay::drawNodes(juce::Graphics& g) {
    if (!eqProcessor) return;
    
    for (int i = 0; i < Constants::numEQBands; ++i) {
        auto params = eqProcessor->getBandParameters(i);
        if (!params.enabled) continue;
        
        juce::Point<float> pos = getNodePosition(i);
        
        // Determine node color
        juce::Colour color = bandColors[static_cast<size_t>(i)];
        float radius = nodeRadius;
        
        if (i == draggingBand) {
            color = nodeSelectedColor;
            radius = nodeRadius * 1.3f;
        } else if (i == selectedBand) {
            color = nodeSelectedColor;
            radius = nodeRadius * 1.2f;
        } else if (i == hoveredBand) {
            color = nodeHoverColor;
            radius = nodeRadius * 1.1f;
        }
        
        // Draw node shadow
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.fillEllipse(pos.x - radius + 1, pos.y - radius + 1, radius * 2, radius * 2);
        
        // Draw node fill
        g.setColour(color);
        g.fillEllipse(pos.x - radius, pos.y - radius, radius * 2, radius * 2);
        
        // Draw node outline
        g.setColour(juce::Colours::white);
        g.drawEllipse(pos.x - radius, pos.y - radius, radius * 2, radius * 2, 1.5f);
        
        // Draw band number
        g.setColour(juce::Colours::black);
        g.setFont(juce::Font(10.0f).boldened());
        g.drawText(juce::String(i + 1), 
                   static_cast<int>(pos.x - radius), static_cast<int>(pos.y - radius),
                   static_cast<int>(radius * 2), static_cast<int>(radius * 2),
                   juce::Justification::centred);
    }
}

juce::Path EQCurveDisplay::createResponseCurve() const {
    juce::Path path;
    
    const int numPoints = static_cast<int>(plotBounds.getWidth());
    bool started = false;
    
    for (int i = 0; i < numPoints; ++i) {
        const float x = plotBounds.getX() + static_cast<float>(i);
        const float freq = xToFrequency(x);
        
        // Get combined magnitude from EQ processor
        const float mag = eqProcessor ? eqProcessor->getMagnitudeAtFrequency(freq) : 1.0f;
        const float db = 20.0f * std::log10(std::max(mag, 0.0001f));
        const float y = dbToY(db);
        
        if (!started) {
            path.startNewSubPath(x, y);
            started = true;
        } else {
            path.lineTo(x, y);
        }
    }
    
    return path;
}

juce::Path EQCurveDisplay::createBandCurve(int bandIndex) const {
    juce::Path path;
    
    const int numPoints = static_cast<int>(plotBounds.getWidth());
    bool started = false;
    
    for (int i = 0; i < numPoints; ++i) {
        const float x = plotBounds.getX() + static_cast<float>(i);
        const float freq = xToFrequency(x);
        
        const float mag = eqProcessor ? eqProcessor->getBandMagnitudeAtFrequency(bandIndex, freq) : 1.0f;
        const float db = 20.0f * std::log10(std::max(mag, 0.0001f));
        const float y = dbToY(db);
        
        if (!started) {
            path.startNewSubPath(x, y);
            started = true;
        } else {
            path.lineTo(x, y);
        }
    }
    
    return path;
}

juce::Point<float> EQCurveDisplay::getNodePosition(int bandIndex) const {
    if (!eqProcessor) return { 0, 0 };
    
    auto params = eqProcessor->getBandParameters(bandIndex);
    return { frequencyToX(params.frequency), dbToY(params.gain) };
}

int EQCurveDisplay::getNodeAtPosition(juce::Point<float> pos) const {
    if (!eqProcessor) return -1;
    
    for (int i = 0; i < Constants::numEQBands; ++i) {
        auto params = eqProcessor->getBandParameters(i);
        if (!params.enabled) continue;
        
        juce::Point<float> nodePos = getNodePosition(i);
        if (pos.getDistanceFrom(nodePos) <= nodeHitRadius) {
            return i;
        }
    }
    
    return -1;
}

void EQCurveDisplay::mouseDown(const juce::MouseEvent& e) {
    int clickedNode = getNodeAtPosition(e.position);
    
    if (clickedNode >= 0) {
        draggingBand = clickedNode;
        setSelectedBand(clickedNode);
        
        auto params = eqProcessor->getBandParameters(clickedNode);
        dragStartPos = e.position;
        dragStartFreq = params.frequency;
        dragStartGain = params.gain;
    } else {
        setSelectedBand(-1);
    }
    
    repaint();
}

void EQCurveDisplay::mouseDrag(const juce::MouseEvent& e) {
    if (draggingBand < 0 || !apvts) return;
    
    // Calculate new frequency and gain from position
    float newFreq = xToFrequency(e.position.x);
    float newGain = yToDb(e.position.y);
    
    // Clamp values
    newFreq = std::clamp(newFreq, minFreq, maxFreq);
    newGain = std::clamp(newGain, minDb, maxDb);
    
    // Update parameters
    auto freqParam = apvts->getParameter(ParamIDs::getBandParamID(draggingBand, ParamIDs::bandFreq));
    auto gainParam = apvts->getParameter(ParamIDs::getBandParamID(draggingBand, ParamIDs::bandGain));
    
    if (freqParam) {
        freqParam->setValueNotifyingHost(freqParam->convertTo0to1(newFreq));
    }
    if (gainParam) {
        gainParam->setValueNotifyingHost(gainParam->convertTo0to1(newGain));
    }
    
    repaint();
}

void EQCurveDisplay::mouseUp(const juce::MouseEvent& /*e*/) {
    draggingBand = -1;
    repaint();
}

void EQCurveDisplay::mouseMove(const juce::MouseEvent& e) {
    int newHovered = getNodeAtPosition(e.position);
    
    if (newHovered != hoveredBand) {
        hoveredBand = newHovered;
        
        if (hoveredBand >= 0)
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
        else
            setMouseCursor(juce::MouseCursor::NormalCursor);
        
        repaint();
    }
}

void EQCurveDisplay::mouseDoubleClick(const juce::MouseEvent& e) {
    int clickedNode = getNodeAtPosition(e.position);
    
    if (clickedNode >= 0 && apvts) {
        // Reset gain to 0
        auto gainParam = apvts->getParameter(ParamIDs::getBandParamID(clickedNode, ParamIDs::bandGain));
        if (gainParam) {
            gainParam->setValueNotifyingHost(gainParam->convertTo0to1(0.0f));
        }
        repaint();
    }
}

void EQCurveDisplay::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) {
    int nodeUnderMouse = getNodeAtPosition(e.position);
    
    if (nodeUnderMouse >= 0 && apvts) {
        // Adjust Q with mouse wheel
        auto qParam = apvts->getParameter(ParamIDs::getBandParamID(nodeUnderMouse, ParamIDs::bandQ));
        if (qParam) {
            float currentQ = qParam->getValue();
            float delta = wheel.deltaY * 0.1f;
            float newQ = std::clamp(currentQ + delta, 0.0f, 1.0f);
            qParam->setValueNotifyingHost(newQ);
        }
        repaint();
    }
}

float EQCurveDisplay::frequencyToX(float frequency) const {
    const float logMin = std::log10(minFreq);
    const float logMax = std::log10(maxFreq);
    const float logFreq = std::log10(std::max(frequency, minFreq));
    
    const float normalized = (logFreq - logMin) / (logMax - logMin);
    return plotBounds.getX() + normalized * plotBounds.getWidth();
}

float EQCurveDisplay::xToFrequency(float x) const {
    const float normalized = (x - plotBounds.getX()) / plotBounds.getWidth();
    
    const float logMin = std::log10(minFreq);
    const float logMax = std::log10(maxFreq);
    const float logFreq = logMin + normalized * (logMax - logMin);
    
    return std::pow(10.0f, logFreq);
}

float EQCurveDisplay::dbToY(float db) const {
    const float normalized = (db - minDb) / (maxDb - minDb);
    return plotBounds.getBottom() - normalized * plotBounds.getHeight();
}

float EQCurveDisplay::yToDb(float y) const {
    const float normalized = (plotBounds.getBottom() - y) / plotBounds.getHeight();
    return minDb + normalized * (maxDb - minDb);
}

} // namespace SeshEQ
