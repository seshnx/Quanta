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
    
    // Color scheme - Sci-Fi Theme (Black/Cyan/White/Holographic)
    struct Colors {
        // Base colors (Sci-Fi Dark)
        static inline const juce::Colour background      { 0xff000000 };  // Pure Black
        static inline const juce::Colour backgroundLight { 0xff0a0a0a };  // Slightly lighter black
        static inline const juce::Colour backgroundDark  { 0xff000000 };  // Pure Black
        
        // Text colors (High Contrast)
        static inline const juce::Colour textPrimary   { 0xffffffff };  // Pure White
        static inline const juce::Colour textSecondary { 0xff00ffff };  // Cyan
        static inline const juce::Colour textDim       { 0xff666666 };  // Gray
        
        // Sci-Fi Accent colors
        static inline const juce::Colour accent        { 0xff00ffff };  // Cyan (primary)
        static inline const juce::Colour accentAlt     { 0xff00cccc };  // Darker Cyan
        static inline const juce::Colour holographic   { 0xff88ffff };  // Light Cyan (holographic effect)
        static inline const juce::Colour warning       { 0xffffff00 };  // Yellow
        static inline const juce::Colour danger        { 0xffff0000 };  // Red
        
        // Component colors
        static inline const juce::Colour knobFill      { 0xff0a0a0a };  // Dark
        static inline const juce::Colour knobOutline   { 0xff00ffff };  // Cyan outline
        static inline const juce::Colour knobPointer   { 0xff00ffff };  // Cyan pointer
        
        // Band colors (Cyan variations with holographic accents)
        static inline const std::array<juce::Colour, 8> bandColors = {
            juce::Colour(0xff00ffff),  // Band 1 - Pure Cyan
            juce::Colour(0xff00cccc),  // Band 2 - Dark Cyan
            juce::Colour(0xff88ffff),  // Band 3 - Light Cyan (holographic)
            juce::Colour(0xff00ffcc),  // Band 4 - Cyan-Green
            juce::Colour(0xff00ccff),  // Band 5 - Cyan-Blue
            juce::Colour(0xffccffff),  // Band 6 - Very Light Cyan
            juce::Colour(0xff66ffff),  // Band 7 - Medium Cyan
            juce::Colour(0xff00ffff),  // Band 8 - Pure Cyan
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
