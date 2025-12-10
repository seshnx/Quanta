#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../presets/PresetManager.h"

namespace SeshEQ {

/**
 * @brief Compact preset selector with dropdown
 */
class PresetSelector : public juce::Component,
                       private juce::Button::Listener,
                       private juce::ComboBox::Listener {
public:
    PresetSelector(PresetManager& pm);
    ~PresetSelector() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void refreshPresetList();

private:
    void buttonClicked(juce::Button* button) override;
    void comboBoxChanged(juce::ComboBox* comboBox) override;
    
    void showSaveDialog();
    void showPresetBrowser();
    
    PresetManager& presetManager;
    
    juce::ComboBox presetCombo;
    juce::TextButton prevButton { "<" };
    juce::TextButton nextButton { ">" };
    juce::TextButton saveButton { "Save" };
    juce::TextButton browserButton { "..." };
    
    int currentPresetIndex = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetSelector)
};

/**
 * @brief A/B comparison control panel
 */
class ABComparisonPanel : public juce::Component,
                          private juce::Button::Listener {
public:
    ABComparisonPanel(PresetManager& pm);
    ~ABComparisonPanel() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void updateState();

private:
    void buttonClicked(juce::Button* button) override;
    
    PresetManager& presetManager;
    
    juce::TextButton abToggleButton { "A" };
    juce::TextButton copyABButton { "A>B" };
    juce::TextButton copyBAButton { "B>A" };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ABComparisonPanel)
};

/**
 * @brief Save Preset Dialog
 */
class SavePresetDialog : public juce::Component,
                         private juce::Button::Listener {
public:
    SavePresetDialog(PresetManager& pm, std::function<void()> onComplete);
    ~SavePresetDialog() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void show();

private:
    void buttonClicked(juce::Button* button) override;
    bool validateAndSave();
    
    PresetManager& presetManager;
    std::function<void()> onSaveComplete;
    
    juce::Label nameLabel { {}, "Name:" };
    juce::Label categoryLabel { {}, "Category:" };
    juce::Label authorLabel { {}, "Author:" };
    juce::Label descriptionLabel { {}, "Description:" };
    
    juce::TextEditor nameEditor;
    juce::ComboBox categoryCombo;
    juce::TextEditor authorEditor;
    juce::TextEditor descriptionEditor;
    
    juce::TextButton saveButton { "Save" };
    juce::TextButton cancelButton { "Cancel" };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SavePresetDialog)
};

/**
 * @brief Full preset browser panel with categories
 */
class PresetBrowserPanel : public juce::Component,
                           private juce::ListBoxModel,
                           private juce::Button::Listener {
public:
    PresetBrowserPanel(PresetManager& pm, std::function<void()> onClose);
    ~PresetBrowserPanel() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // ListBoxModel
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g,
                          int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent& e) override;
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent& e) override;
    
    void refreshPresetList();

private:
    void buttonClicked(juce::Button* button) override;
    void filterByCategory(const juce::String& category);
    
    PresetManager& presetManager;
    std::function<void()> onCloseCallback;
    
    juce::Label titleLabel { {}, "Preset Browser" };
    
    // Category buttons
    juce::OwnedArray<juce::TextButton> categoryButtons;
    juce::String currentCategory;
    
    // Preset list
    juce::ListBox presetList { "Presets" };
    std::vector<PresetInfo> filteredPresets;
    
    // Search
    juce::TextEditor searchBox;
    
    // Actions
    juce::TextButton loadButton { "Load" };
    juce::TextButton deleteButton { "Delete" };
    juce::TextButton closeButton { "Close" };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowserPanel)
};

} // namespace SeshEQ
