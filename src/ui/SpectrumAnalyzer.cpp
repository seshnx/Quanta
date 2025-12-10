#include "SpectrumAnalyzer.h"
#include <cmath>

namespace SeshEQ {

SpectrumAnalyzer::SpectrumAnalyzer() {
    setOpaque(false);
    startTimerHz(refreshRateHz);
}

SpectrumAnalyzer::~SpectrumAnalyzer() {
    stopTimer();
}

void SpectrumAnalyzer::setFFTProcessor(FFTProcessor* processor) {
    fftProcessor = processor;
    postFFT = processor;
}

void SpectrumAnalyzer::setFFTProcessors(FFTProcessor* pre, FFTProcessor* post) {
    preFFT = pre;
    postFFT = post;
    fftProcessor = post;
}

void SpectrumAnalyzer::setColors(juce::Colour fill, juce::Colour outline) {
    fillColor = fill;
    outlineColor = outline;
}

void SpectrumAnalyzer::setPrePostColors(juce::Colour pre, juce::Colour post) {
    preColor = pre;
    postColor = post;
}

void SpectrumAnalyzer::setFrequencyRange(float minHz, float maxHz) {
    minFreq = minHz;
    maxFreq = maxHz;
}

void SpectrumAnalyzer::setDbRange(float min, float max) {
    minDb = min;
    maxDb = max;
}

void SpectrumAnalyzer::timerCallback() {
    bool needsRepaint = false;
    
    if (fftProcessor && fftProcessor->isNewDataAvailable())
        needsRepaint = true;
    if (preFFT && preFFT->isNewDataAvailable())
        needsRepaint = true;
    if (postFFT && postFFT->isNewDataAvailable())
        needsRepaint = true;
    
    if (needsRepaint)
        repaint();
}

void SpectrumAnalyzer::paint(juce::Graphics& g) {
    drawBackground(g);
    drawGrid(g);
    
    // Draw pre-EQ spectrum (if available)
    if (showPreSpectrum && preFFT) {
        drawSpectrum(g, preFFT, preColor, false);
    }
    
    // Draw post-EQ spectrum
    if (postFFT) {
        drawSpectrum(g, postFFT, postColor, true);
    } else if (fftProcessor) {
        drawSpectrum(g, fftProcessor, postColor, true);
    }
}

void SpectrumAnalyzer::resized() {
    plotBounds = getLocalBounds().toFloat().reduced(2.0f);
}

void SpectrumAnalyzer::drawBackground(juce::Graphics& g) {
    // Gradient background
    juce::ColourGradient gradient(
        juce::Colour(0xff1a1a2e), 0, 0,
        juce::Colour(0xff0d0d1a), 0, static_cast<float>(getHeight()),
        false
    );
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);
}

void SpectrumAnalyzer::drawGrid(juce::Graphics& g) {
    g.setColour(gridColor);
    
    const float left = plotBounds.getX();
    const float right = plotBounds.getRight();
    const float top = plotBounds.getY();
    const float bottom = plotBounds.getBottom();
    
    // Frequency grid lines (logarithmic)
    const std::array<float, 9> freqLines = { 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 };
    
    g.setFont(juce::Font(10.0f));
    
    for (float freq : freqLines) {
        if (freq >= minFreq && freq <= maxFreq) {
            const float x = frequencyToX(freq);
            
            g.setColour(gridColor);
            g.drawVerticalLine(static_cast<int>(x), top, bottom);
            
            // Label
            g.setColour(textColor);
            juce::String label;
            if (freq >= 1000)
                label = juce::String(static_cast<int>(freq / 1000)) + "k";
            else
                label = juce::String(static_cast<int>(freq));
            
            g.drawText(label, 
                       static_cast<int>(x) - 15, static_cast<int>(bottom) - 15, 
                       30, 12, 
                       juce::Justification::centred);
        }
    }
    
    // dB grid lines
    const std::array<float, 5> dbLines = { 0, -12, -24, -48, -72 };
    
    for (float db : dbLines) {
        if (db >= minDb && db <= maxDb) {
            const float y = dbToY(db);
            
            g.setColour(gridColor);
            g.drawHorizontalLine(static_cast<int>(y), left, right);
            
            // Label
            g.setColour(textColor);
            g.drawText(juce::String(static_cast<int>(db)) + " dB",
                       static_cast<int>(left) + 2, static_cast<int>(y) - 6,
                       40, 12,
                       juce::Justification::left);
        }
    }
}

void SpectrumAnalyzer::drawSpectrum(juce::Graphics& g, FFTProcessor* fft, juce::Colour color, bool fill) {
    if (!fft) return;
    
    juce::Path path = createSpectrumPath(fft);
    
    if (fill) {
        // Create filled version
        juce::Path fillPath = path;
        fillPath.lineTo(plotBounds.getRight(), plotBounds.getBottom());
        fillPath.lineTo(plotBounds.getX(), plotBounds.getBottom());
        fillPath.closeSubPath();
        
        // Gradient fill
        juce::ColourGradient gradient(
            color, 0, plotBounds.getY(),
            color.withAlpha(0.0f), 0, plotBounds.getBottom(),
            false
        );
        g.setGradientFill(gradient);
        g.fillPath(fillPath);
    }
    
    // Draw outline
    g.setColour(color.withAlpha(0.8f));
    g.strokePath(path, juce::PathStrokeType(1.5f));
}

juce::Path SpectrumAnalyzer::createSpectrumPath(FFTProcessor* fft) {
    juce::Path path;
    
    const auto& magnitudes = fft->getMagnitudes();
    const float nyquist = static_cast<float>(fft->getSampleRate()) / 2.0f;
    
    bool started = false;
    
    // Draw from left to right using logarithmic frequency scale
    const int numPoints = static_cast<int>(plotBounds.getWidth());
    
    for (int i = 0; i < numPoints; ++i) {
        const float x = plotBounds.getX() + static_cast<float>(i);
        const float freq = xToFrequency(x);
        
        if (freq < minFreq || freq > std::min(maxFreq, nyquist))
            continue;
        
        // Get magnitude at this frequency (interpolate between bins)
        const int bin = fft->getBinForFrequency(freq);
        if (bin < 0 || bin >= FFTProcessor::numBins - 1)
            continue;
        
        // Linear interpolation between bins
        const float binFreq = fft->getFrequencyForBin(bin);
        const float nextBinFreq = fft->getFrequencyForBin(bin + 1);
        const float t = (freq - binFreq) / (nextBinFreq - binFreq + 0.001f);
        
        const float mag = magnitudes[static_cast<size_t>(bin)] * (1.0f - t) + 
                          magnitudes[static_cast<size_t>(bin + 1)] * t;
        
        const float y = dbToY(mag);
        
        if (!started) {
            path.startNewSubPath(x, y);
            started = true;
        } else {
            path.lineTo(x, y);
        }
    }
    
    return path;
}

float SpectrumAnalyzer::frequencyToX(float frequency) const {
    // Logarithmic mapping
    const float logMin = std::log10(minFreq);
    const float logMax = std::log10(maxFreq);
    const float logFreq = std::log10(std::max(frequency, minFreq));
    
    const float normalized = (logFreq - logMin) / (logMax - logMin);
    return plotBounds.getX() + normalized * plotBounds.getWidth();
}

float SpectrumAnalyzer::xToFrequency(float x) const {
    const float normalized = (x - plotBounds.getX()) / plotBounds.getWidth();
    
    const float logMin = std::log10(minFreq);
    const float logMax = std::log10(maxFreq);
    const float logFreq = logMin + normalized * (logMax - logMin);
    
    return std::pow(10.0f, logFreq);
}

float SpectrumAnalyzer::dbToY(float db) const {
    const float normalized = (db - minDb) / (maxDb - minDb);
    return plotBounds.getBottom() - normalized * plotBounds.getHeight();
}

float SpectrumAnalyzer::yToDb(float y) const {
    const float normalized = (plotBounds.getBottom() - y) / plotBounds.getHeight();
    return minDb + normalized * (maxDb - minDb);
}

} // namespace SeshEQ
