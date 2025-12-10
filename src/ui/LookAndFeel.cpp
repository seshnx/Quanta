#include "LookAndFeel.h"

namespace SeshEQ {

SeshLookAndFeel::SeshLookAndFeel() {
    // Set default colors
    setColour(juce::ResizableWindow::backgroundColourId, Colors::background);
    setColour(juce::PopupMenu::backgroundColourId, Colors::backgroundLight);
    setColour(juce::PopupMenu::textColourId, Colors::textPrimary);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, Colors::accent.withAlpha(0.3f));
    setColour(juce::PopupMenu::highlightedTextColourId, Colors::textPrimary);
    
    setColour(juce::ComboBox::backgroundColourId, Colors::backgroundLight);
    setColour(juce::ComboBox::textColourId, Colors::textPrimary);
    setColour(juce::ComboBox::outlineColourId, Colors::knobOutline);
    setColour(juce::ComboBox::arrowColourId, Colors::textSecondary);
    
    setColour(juce::TextButton::buttonColourId, Colors::backgroundLight);
    setColour(juce::TextButton::textColourOffId, Colors::textPrimary);
    setColour(juce::TextButton::textColourOnId, Colors::accent);
    
    setColour(juce::Slider::thumbColourId, Colors::accent);
    setColour(juce::Slider::trackColourId, Colors::knobOutline);
    setColour(juce::Slider::textBoxTextColourId, Colors::textPrimary);
    setColour(juce::Slider::textBoxBackgroundColourId, Colors::backgroundDark);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    
    setColour(juce::Label::textColourId, Colors::textPrimary);
    
    setColour(juce::ScrollBar::thumbColourId, Colors::knobOutline);
    setColour(juce::ScrollBar::trackColourId, Colors::backgroundDark);
    
    // Fonts
    mainFont = juce::Font(13.0f);
    boldFont = juce::Font(13.0f).boldened();
}

void SeshLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPosProportional, float rotaryStartAngle,
                                        float rotaryEndAngle, juce::Slider& slider) {
    const float radius = static_cast<float>(juce::jmin(width / 2, height / 2)) - 4.0f;
    const float centreX = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
    const float centreY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
    const float rx = centreX - radius;
    const float ry = centreY - radius;
    const float rw = radius * 2.0f;
    const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    
    const bool isEnabled = slider.isEnabled();
    
    // Draw background circle
    g.setColour(Colors::knobFill.withAlpha(isEnabled ? 1.0f : 0.5f));
    g.fillEllipse(rx, ry, rw, rw);
    
    // Draw outline
    g.setColour(Colors::knobOutline.withAlpha(isEnabled ? 1.0f : 0.5f));
    g.drawEllipse(rx, ry, rw, rw, 2.0f);
    
    // Draw arc (value indicator)
    if (isEnabled) {
        juce::Path arcPath;
        const float arcRadius = radius - 4.0f;
        
        arcPath.addCentredArc(centreX, centreY, arcRadius, arcRadius,
                              0.0f, rotaryStartAngle, angle, true);
        
        g.setColour(Colors::accent);
        g.strokePath(arcPath, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                                    juce::PathStrokeType::rounded));
    }
    
    // Draw pointer
    juce::Path pointer;
    const float pointerLength = radius * 0.6f;
    const float pointerThickness = 3.0f;
    
    pointer.addRectangle(-pointerThickness * 0.5f, -radius + 6.0f, pointerThickness, pointerLength);
    
    g.setColour(isEnabled ? Colors::knobPointer : Colors::textDim);
    g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centreX, centreY));
    
    // Draw center dot
    g.setColour(Colors::backgroundLight);
    g.fillEllipse(centreX - 4.0f, centreY - 4.0f, 8.0f, 8.0f);
}

void SeshLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                                        juce::Slider::SliderStyle style, juce::Slider& slider) {
    const bool isEnabled = slider.isEnabled();
    const bool isHorizontal = (style == juce::Slider::LinearHorizontal ||
                               style == juce::Slider::LinearBar);
    
    auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                          static_cast<float>(width), static_cast<float>(height));
    
    // Track
    float trackThickness = isHorizontal ? 4.0f : 4.0f;
    juce::Rectangle<float> track;
    
    if (isHorizontal) {
        track = bounds.withSizeKeepingCentre(bounds.getWidth(), trackThickness);
    } else {
        track = bounds.withSizeKeepingCentre(trackThickness, bounds.getHeight());
    }
    
    g.setColour(Colors::knobOutline.withAlpha(isEnabled ? 1.0f : 0.5f));
    g.fillRoundedRectangle(track, 2.0f);
    
    // Filled portion
    if (isEnabled) {
        juce::Rectangle<float> filled;
        
        if (isHorizontal) {
            filled = track.withWidth(sliderPos - track.getX());
        } else {
            const float fillHeight = track.getBottom() - sliderPos;
            filled = track.withTop(sliderPos).withHeight(fillHeight);
        }
        
        g.setColour(Colors::accent);
        g.fillRoundedRectangle(filled, 2.0f);
    }
    
    // Thumb
    const float thumbSize = 12.0f;
    juce::Point<float> thumbPos;
    
    if (isHorizontal) {
        thumbPos = { sliderPos, bounds.getCentreY() };
    } else {
        thumbPos = { bounds.getCentreX(), sliderPos };
    }
    
    g.setColour(isEnabled ? Colors::accent : Colors::textDim);
    g.fillEllipse(thumbPos.x - thumbSize / 2, thumbPos.y - thumbSize / 2,
                  thumbSize, thumbSize);
    
    g.setColour(Colors::textPrimary);
    g.drawEllipse(thumbPos.x - thumbSize / 2, thumbPos.y - thumbSize / 2,
                  thumbSize, thumbSize, 1.5f);
}

void SeshLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                            const juce::Colour& /*backgroundColour*/,
                                            bool shouldDrawButtonAsHighlighted,
                                            bool shouldDrawButtonAsDown) {
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    
    juce::Colour baseColor = Colors::backgroundLight;
    
    if (shouldDrawButtonAsDown)
        baseColor = Colors::accent.withAlpha(0.3f);
    else if (shouldDrawButtonAsHighlighted)
        baseColor = baseColor.brighter(0.1f);
    
    if (button.getToggleState())
        baseColor = Colors::accent.withAlpha(0.2f);
    
    g.setColour(baseColor);
    g.fillRoundedRectangle(bounds, 4.0f);
    
    g.setColour(button.getToggleState() ? Colors::accent : Colors::knobOutline);
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
}

void SeshLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                        bool shouldDrawButtonAsHighlighted,
                                        bool /*shouldDrawButtonAsDown*/) {
    const float fontSize = 12.0f;
    const float tickWidth = fontSize * 1.5f;
    
    auto bounds = button.getLocalBounds().toFloat();
    
    // Draw toggle indicator
    auto tickBounds = bounds.removeFromLeft(tickWidth).reduced(4.0f);
    tickBounds = tickBounds.withSizeKeepingCentre(tickBounds.getWidth(), tickBounds.getWidth());
    
    g.setColour(Colors::backgroundLight);
    g.fillRoundedRectangle(tickBounds, 3.0f);
    
    g.setColour(button.getToggleState() ? Colors::accent : Colors::knobOutline);
    g.drawRoundedRectangle(tickBounds, 3.0f, 1.5f);
    
    if (button.getToggleState()) {
        g.setColour(Colors::accent);
        g.fillRoundedRectangle(tickBounds.reduced(3.0f), 2.0f);
    }
    
    // Draw text
    g.setColour(shouldDrawButtonAsHighlighted ? Colors::textPrimary : Colors::textSecondary);
    g.setFont(mainFont);
    
    g.drawText(button.getButtonText(), bounds.reduced(4, 0),
               juce::Justification::centredLeft, true);
}

void SeshLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                                    int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                                    juce::ComboBox& box) {
    auto bounds = juce::Rectangle<float>(0, 0, static_cast<float>(width), static_cast<float>(height));
    
    g.setColour(Colors::backgroundLight);
    g.fillRoundedRectangle(bounds, 4.0f);
    
    g.setColour(isButtonDown ? Colors::accent : Colors::knobOutline);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);
    
    // Draw arrow
    juce::Path arrow;
    const float arrowSize = 6.0f;
    const float arrowX = static_cast<float>(width) - 15.0f;
    const float arrowY = static_cast<float>(height) / 2.0f;
    
    arrow.addTriangle(arrowX - arrowSize, arrowY - arrowSize / 2,
                      arrowX + arrowSize, arrowY - arrowSize / 2,
                      arrowX, arrowY + arrowSize / 2);
    
    g.setColour(box.isEnabled() ? Colors::textSecondary : Colors::textDim);
    g.fillPath(arrow);
}

void SeshLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label) {
    g.fillAll(label.findColour(juce::Label::backgroundColourId));
    
    if (!label.isBeingEdited()) {
        const juce::Font font(getLabelFont(label));
        g.setColour(label.findColour(juce::Label::textColourId));
        g.setFont(font);
        
        auto textArea = label.getBorderSize().subtractedFrom(label.getLocalBounds());
        
        g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                         juce::jmax(1, static_cast<int>(static_cast<float>(textArea.getHeight()) / font.getHeight())),
                         label.getMinimumHorizontalScale());
    }
}

void SeshLookAndFeel::drawScrollbar(juce::Graphics& g, juce::ScrollBar& scrollbar,
                                     int x, int y, int width, int height,
                                     bool isScrollbarVertical, int thumbStartPosition,
                                     int thumbSize, bool isMouseOver, bool isMouseDown) {
    juce::ignoreUnused(scrollbar);
    
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    
    // Track
    g.setColour(Colors::backgroundDark);
    g.fillRoundedRectangle(bounds, 3.0f);
    
    // Thumb
    juce::Rectangle<float> thumbBounds;
    if (isScrollbarVertical) {
        thumbBounds = bounds.withPosition(bounds.getX(),
                                          static_cast<float>(thumbStartPosition))
                           .withHeight(static_cast<float>(thumbSize));
    } else {
        thumbBounds = bounds.withPosition(static_cast<float>(thumbStartPosition),
                                          bounds.getY())
                           .withWidth(static_cast<float>(thumbSize));
    }
    
    juce::Colour thumbColor = Colors::knobOutline;
    if (isMouseDown)
        thumbColor = Colors::accent;
    else if (isMouseOver)
        thumbColor = thumbColor.brighter(0.2f);
    
    g.setColour(thumbColor);
    g.fillRoundedRectangle(thumbBounds.reduced(2.0f), 2.0f);
}

void SeshLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                         bool isSeparator, bool isActive, bool isHighlighted,
                                         bool isTicked, bool /*hasSubMenu*/, const juce::String& text,
                                         const juce::String& /*shortcutKeyText*/, const juce::Drawable* /*icon*/,
                                         const juce::Colour* /*textColour*/) {
    if (isSeparator) {
        auto r = area.reduced(5, 0).toFloat();
        g.setColour(Colors::knobOutline.withAlpha(0.3f));
        g.fillRect(r.withHeight(1.0f).withY(r.getCentreY()));
        return;
    }
    
    auto r = area.reduced(1);
    
    if (isHighlighted && isActive) {
        g.setColour(Colors::accent.withAlpha(0.2f));
        g.fillRoundedRectangle(r.toFloat(), 3.0f);
    }
    
    g.setColour(isActive ? Colors::textPrimary : Colors::textDim);
    g.setFont(getPopupMenuFont());
    
    auto textArea = r.reduced(10, 0);
    
    if (isTicked) {
        g.setColour(Colors::accent);
        const float tickSize = 8.0f;
        g.fillEllipse(static_cast<float>(r.getX()) + 6.0f,
                      static_cast<float>(r.getCentreY()) - tickSize / 2.0f,
                      tickSize, tickSize);
        textArea = textArea.withTrimmedLeft(static_cast<int>(tickSize) + 8);
    }
    
    g.setColour(isActive ? Colors::textPrimary : Colors::textDim);
    g.drawFittedText(text, textArea, juce::Justification::centredLeft, 1);
}

juce::Font SeshLookAndFeel::getLabelFont(juce::Label& /*label*/) {
    return mainFont;
}

juce::Font SeshLookAndFeel::getPopupMenuFont() {
    return mainFont;
}

juce::Font SeshLookAndFeel::getComboBoxFont(juce::ComboBox& /*box*/) {
    return mainFont;
}

} // namespace SeshEQ
