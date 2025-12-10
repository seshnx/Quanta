#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace SeshEQ {

/**
 * @brief Custom look and feel for SeshEQ
 * 
 * Modern dark theme with accent colors for different sections.
 */
class SeshLookAndFeel : public juce::LookAndFeel_V4 {
public:
    SeshLookAndFeel();
    ~SeshLookAndFeel() override = default;
    
    // Color scheme
    struct Colors {
        // Base colors
        static inline const juce::Colour background      { 0xff1a1a2e };
        static inline const juce::Colour backgroundLight { 0xff2d2d44 };
        static inline const juce::Colour backgroundDark  { 0xff0d0d1a };
        
        // Text colors
        static inline const juce::Colour textPrimary   { 0xffffffff };
        static inline const juce::Colour textSecondary { 0xffb0b0b0 };
        static inline const juce::Colour textDim       { 0xff707070 };
        
        // Accent colors
        static inline const juce::Colour accent        { 0xff00ff88 };  // Green
        static inline const juce::Colour accentAlt     { 0xff4dabf7 };  // Blue
        static inline const juce::Colour warning       { 0xffffcc00 };  // Yellow
        static inline const juce::Colour danger        { 0xffff4444 };  // Red
        
        // Component colors
        static inline const juce::Colour knobFill      { 0xff3d3d5c };
        static inline const juce::Colour knobOutline   { 0xff505070 };
        static inline const juce::Colour knobPointer   { 0xff00ff88 };
        
        // Band colors
        static inline const std::array<juce::Colour, 8> bandColors = {
            juce::Colour(0xffff6b6b),  // Band 1 - Red
            juce::Colour(0xffffa94d),  // Band 2 - Orange
            juce::Colour(0xffffd43b),  // Band 3 - Yellow
            juce::Colour(0xff69db7c),  // Band 4 - Green
            juce::Colour(0xff4dabf7),  // Band 5 - Blue
            juce::Colour(0xff9775fa),  // Band 6 - Purple
            juce::Colour(0xfff06595),  // Band 7 - Pink
            juce::Colour(0xff20c997),  // Band 8 - Teal
        };
    };
    
    // Slider customization
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override;
    
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          juce::Slider::SliderStyle style, juce::Slider& slider) override;
    
    // Button customization
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;
    
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;
    
    // ComboBox customization
    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override;
    
    // Label customization
    void drawLabel(juce::Graphics& g, juce::Label& label) override;
    
    // Scrollbar
    void drawScrollbar(juce::Graphics& g, juce::ScrollBar& scrollbar, int x, int y,
                       int width, int height, bool isScrollbarVertical, int thumbStartPosition,
                       int thumbSize, bool isMouseOver, bool isMouseDown) override;
    
    // Popup menu
    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                           bool isSeparator, bool isActive, bool isHighlighted,
                           bool isTicked, bool hasSubMenu, const juce::String& text,
                           const juce::String& shortcutKeyText, const juce::Drawable* icon,
                           const juce::Colour* textColour) override;
    
    // Fonts
    juce::Font getLabelFont(juce::Label& label) override;
    juce::Font getPopupMenuFont() override;
    juce::Font getComboBoxFont(juce::ComboBox& box) override;
    
private:
    juce::Font mainFont;
    juce::Font boldFont;
};

} // namespace SeshEQ
