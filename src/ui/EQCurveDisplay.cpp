#include "EQCurveDisplay.h"
#include "LookAndFeel.h"
#include <cmath>

namespace SeshEQ {

EQCurveDisplay::EQCurveDisplay() {
    setOpaque(false);
}

void EQCurveDisplay::setEQProcessor(const EQProcessor* processor) {
    eqProcessor = processor;
    
    // Invalidate cached paths
    responsePathValid = false;
    for (auto& valid : bandPathsValid) {
        valid = false;
    }
    
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

void EQCurveDisplay::setBandGainReduction(int bandIndex, float dB) {
    if (bandIndex >= 0 && bandIndex < Constants::numEQBands) {
        // Only update if change is significant (dead zone to prevent jitter)
        static std::array<float, Constants::numEQBands> lastGR = { 0.0f };
        const float diff = std::abs(dB - lastGR[static_cast<size_t>(bandIndex)]);
        if (diff > 0.15f) {  // Reasonable threshold
            bandGainReduction[static_cast<size_t>(bandIndex)] = dB;
            lastGR[static_cast<size_t>(bandIndex)] = dB;
        }
    }
}

void EQCurveDisplay::paint(juce::Graphics& g) {
    // Use software rendering with high-quality settings for smooth curves
    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);

    // Transparent background - spectrum analyzer is visible behind this component
    // Only draw a subtle cyan border around the combined display area
    g.setColour(juce::Colour(0xff00ffff).withAlpha(0.3f));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 4.0f, 1.0f);

    drawGrid(g);
    drawBandCurves(g);
    drawCurve(g);
    drawNodes(g);
    // GR meters are now displayed in band control panels, not on the visualizer
}

void EQCurveDisplay::resized() {
    plotBounds = getLocalBounds().toFloat().reduced(4.0f);
    
    // Invalidate cached paths when resized
    responsePathValid = false;
    for (auto& valid : bandPathsValid) {
        valid = false;
    }
}

void EQCurveDisplay::drawGrid(juce::Graphics& g) {
    g.setColour(gridColor);
    
    const float left = plotBounds.getX();
    const float right = plotBounds.getRight();
    const float top = plotBounds.getY();
    const float bottom = plotBounds.getBottom();
    
    // Center line (0 dB) - brighter cyan
    const float zeroY = dbToY(0.0f);
    g.setColour(juce::Colour(0xff00ffff).withAlpha(0.5f));
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

    // Draw individual band curves for all enabled bands
    for (int i = 0; i < Constants::numEQBands; ++i) {
        auto params = eqProcessor->getBandParameters(i);
        if (!params.enabled) continue;

        // Use the cached path - it's updated in drawCurve when params change
        if (!bandPathsValid[static_cast<size_t>(i)]) {
            cachedBandPaths[static_cast<size_t>(i)] = createBandCurve(i);
            bandPathsValid[static_cast<size_t>(i)] = true;
        }

        juce::Path& bandPath = cachedBandPaths[static_cast<size_t>(i)];

        // Fill with band color (more visible for selected/hovered)
        juce::Path fillPath = bandPath;
        fillPath.lineTo(plotBounds.getRight(), dbToY(0.0f));
        fillPath.lineTo(plotBounds.getX(), dbToY(0.0f));
        fillPath.closeSubPath();

        float fillAlpha = (i == selectedBand || i == hoveredBand) ? 0.2f : 0.08f;
        g.setColour(bandColors[static_cast<size_t>(i)].withAlpha(fillAlpha));
        g.fillPath(fillPath);

        // Stroke outline (brighter for selected/hovered)
        float strokeAlpha = (i == selectedBand || i == hoveredBand) ? 0.8f : 0.4f;
        g.setColour(bandColors[static_cast<size_t>(i)].withAlpha(strokeAlpha));
        g.strokePath(bandPath, juce::PathStrokeType((i == selectedBand || i == hoveredBand) ? 1.5f : 1.0f));
    }
}

void EQCurveDisplay::drawCurve(juce::Graphics& g) {
    if (!eqProcessor) return;

    // Check if we need to recalculate the path (only when parameters change)
    bool needsRecalc = !responsePathValid;

    // Check if any band parameters changed significantly
    for (int i = 0; i < Constants::numEQBands; ++i) {
        auto currentParams = eqProcessor->getBandParameters(i);
        auto& lastParams = lastBandParams[static_cast<size_t>(i)];

        // Only recalc if significant change (larger threshold to prevent jitter)
        if (std::abs(currentParams.frequency - lastParams.frequency) > 2.0f ||
            std::abs(currentParams.gain - lastParams.gain) > 0.3f ||
            std::abs(currentParams.q - lastParams.q) > 0.1f ||
            currentParams.type != lastParams.type ||
            currentParams.enabled != lastParams.enabled) {
            needsRecalc = true;
        }
    }

    // Recalculate path only if needed
    if (needsRecalc) {
        // Update all cached params at once
        for (int i = 0; i < Constants::numEQBands; ++i) {
            lastBandParams[static_cast<size_t>(i)] = eqProcessor->getBandParameters(i);
            bandPathsValid[static_cast<size_t>(i)] = false;  // Invalidate band paths too
        }
        cachedResponsePath = createResponseCurve();
        responsePathValid = true;
    }

    // Fill under curve
    juce::Path fillPath = cachedResponsePath;
    fillPath.lineTo(plotBounds.getRight(), dbToY(0.0f));
    fillPath.lineTo(plotBounds.getX(), dbToY(0.0f));
    fillPath.closeSubPath();

    // Gradient fill using brand blue
    juce::ColourGradient gradient(
        fillColor, 0, dbToY(maxDb),
        fillColor.withAlpha(0.0f), 0, dbToY(0.0f),
        false
    );
    g.setGradientFill(gradient);
    g.fillPath(fillPath);

    // Simplified glow effect (fewer passes for better performance)
    const auto glowColor = SeshLookAndFeel::Colors::accent;
    g.setColour(glowColor.withAlpha(0.15f));
    g.strokePath(cachedResponsePath, juce::PathStrokeType(6.0f));
    g.setColour(glowColor.withAlpha(0.25f));
    g.strokePath(cachedResponsePath, juce::PathStrokeType(3.0f));

    // Draw main curve outline using brand blue
    g.setColour(SeshLookAndFeel::Colors::accent);
    g.strokePath(cachedResponsePath, juce::PathStrokeType(2.0f));
}

void EQCurveDisplay::drawNodes(juce::Graphics& g) {
    if (!eqProcessor) return;

    for (int i = 0; i < Constants::numEQBands; ++i) {
        auto params = eqProcessor->getBandParameters(i);
        if (!params.enabled) continue;

        juce::Point<float> pos = getNodePosition(i);

        // Determine node color and state
        juce::Colour color = bandColors[static_cast<size_t>(i)];
        float radius = nodeRadius;
        bool isActive = false;

        if (i == draggingBand) {
            color = nodeSelectedColor;
            radius = nodeRadius * 1.3f;
            isActive = true;
        } else if (i == selectedBand) {
            color = nodeSelectedColor;
            radius = nodeRadius * 1.2f;
            isActive = true;
        } else if (i == hoveredBand) {
            color = nodeHoverColor;
            radius = nodeRadius * 1.1f;
            isActive = true;
        }

        // Simple glow for active/hovered nodes
        if (isActive) {
            g.setColour(color.withAlpha(0.2f));
            g.fillEllipse(pos.x - radius - 4, pos.y - radius - 4,
                          (radius + 4) * 2, (radius + 4) * 2);
        }

        // Draw node fill
        g.setColour(color);
        g.fillEllipse(pos.x - radius, pos.y - radius, radius * 2, radius * 2);

        // Draw node outline
        g.setColour(isActive ? juce::Colours::white : juce::Colours::white.withAlpha(0.8f));
        g.drawEllipse(pos.x - radius, pos.y - radius, radius * 2, radius * 2, isActive ? 2.0f : 1.5f);

        // Draw band number with shadow
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.setFont(juce::Font(10.0f).boldened());
        g.drawText(juce::String(i + 1),
                   static_cast<int>(pos.x - radius + 1), static_cast<int>(pos.y - radius + 1),
                   static_cast<int>(radius * 2), static_cast<int>(radius * 2),
                   juce::Justification::centred);

        g.setColour(juce::Colours::white);
        g.drawText(juce::String(i + 1),
                   static_cast<int>(pos.x - radius), static_cast<int>(pos.y - radius),
                   static_cast<int>(radius * 2), static_cast<int>(radius * 2),
                   juce::Justification::centred);
    }
}

void EQCurveDisplay::drawBandGRMeters(juce::Graphics& g) {
    if (!eqProcessor) return;

    // Enhanced meter size for better visibility
    const float meterWidth = 8.0f;
    const float meterHeight = 40.0f;
    const float maxGR = -24.0f; // Maximum gain reduction to display

    for (int i = 0; i < Constants::numEQBands; ++i) {
        auto params = eqProcessor->getBandParameters(i);
        if (!params.enabled) continue;

        juce::Point<float> nodePos = getNodePosition(i);
        const float gr = bandGainReduction[static_cast<size_t>(i)];

        // Only draw if there's significant gain reduction
        if (gr > -0.5f) continue;

        // Position meter to the right of the node
        const float meterX = nodePos.x + nodeRadius + 6.0f;
        const float meterY = nodePos.y - meterHeight * 0.5f;

        // Normalize GR (0 = no reduction, 1 = max reduction)
        const float grNorm = std::clamp((gr - maxGR) / (0.0f - maxGR), 0.0f, 1.0f);
        const float grHeight = meterHeight * grNorm;

        // Background with subtle gradient
        g.setColour(juce::Colour(0xff000000).withAlpha(0.6f));
        g.fillRoundedRectangle(meterX, meterY, meterWidth, meterHeight, 2.0f);

        // GR bar (simplified for performance)
        if (grHeight > 0.0f) {
            const auto bandColor = bandColors[static_cast<size_t>(i)];

            // Main GR bar
            g.setColour(bandColor.withAlpha(0.9f));
            g.fillRoundedRectangle(meterX, meterY + meterHeight - grHeight,
                                   meterWidth, grHeight, 2.0f);
        }

        // Border with cyan accent
        g.setColour(juce::Colour(0xff00ffff).withAlpha(0.4f));
        g.drawRoundedRectangle(meterX, meterY, meterWidth, meterHeight, 2.0f, 1.0f);

        // Small GR value text below meter (optional - for very active reduction)
        if (gr < -3.0f) {
            g.setColour(juce::Colour(0xff00ffff).withAlpha(0.7f));
            g.setFont(juce::Font(8.0f));
            g.drawText(juce::String(static_cast<int>(gr)),
                       static_cast<int>(meterX - 4),
                       static_cast<int>(meterY + meterHeight + 2),
                       static_cast<int>(meterWidth + 8), 10,
                       juce::Justification::centred);
        }
    }
}

juce::Path EQCurveDisplay::createResponseCurve() const {
    juce::Path path;
    
    // Optimize: calculate fewer points for smoother performance
    // Use adaptive sampling - more points in critical frequency ranges
    const int numPoints = std::min(static_cast<int>(plotBounds.getWidth()), 400);
    bool started = false;
    
    for (int i = 0; i < numPoints; ++i) {
        const float normalized = static_cast<float>(i) / static_cast<float>(numPoints - 1);
        const float x = plotBounds.getX() + normalized * plotBounds.getWidth();
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
    
    // Optimize: calculate fewer points for smoother performance
    const int numPoints = std::min(static_cast<int>(plotBounds.getWidth()), 400);
    bool started = false;
    
    for (int i = 0; i < numPoints; ++i) {
        const float normalized = static_cast<float>(i) / static_cast<float>(numPoints - 1);
        const float x = plotBounds.getX() + normalized * plotBounds.getWidth();
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
