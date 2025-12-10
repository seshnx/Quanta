#include "PresetBrowser.h"

namespace SeshEQ {

//==============================================================================
// PresetSelector Implementation

PresetSelector::PresetSelector(PresetManager& pm)
    : presetManager(pm)
{
    addAndMakeVisible(presetCombo);
    addAndMakeVisible(prevButton);
    addAndMakeVisible(nextButton);
    addAndMakeVisible(saveButton);
    addAndMakeVisible(browserButton);
    
    presetCombo.addListener(this);
    prevButton.addListener(this);
    nextButton.addListener(this);
    saveButton.addListener(this);
    browserButton.addListener(this);
    
    presetCombo.setTextWhenNothingSelected("Select Preset...");
    presetCombo.setTooltip("Select a preset");
    
    prevButton.setTooltip("Previous preset");
    nextButton.setTooltip("Next preset");
    saveButton.setTooltip("Save current settings");
    browserButton.setTooltip("Open preset browser");
    
    refreshPresetList();
    
    // Register for preset changes
    presetManager.setOnPresetListChanged([this]() {
        juce::MessageManager::callAsync([this]() { refreshPresetList(); });
    });
    
    presetManager.setOnPresetChanged([this](const PresetInfo& /*preset*/) {
        juce::MessageManager::callAsync([this]() { refreshPresetList(); });
    });
}

PresetSelector::~PresetSelector()
{
    presetCombo.removeListener(this);
    prevButton.removeListener(this);
    nextButton.removeListener(this);
    saveButton.removeListener(this);
    browserButton.removeListener(this);
}

void PresetSelector::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2a2a2a));
}

void PresetSelector::resized()
{
    auto bounds = getLocalBounds().reduced(2);
    
    auto buttonWidth = 30;
    auto saveWidth = 50;
    auto browserWidth = 30;
    
    prevButton.setBounds(bounds.removeFromLeft(buttonWidth));
    bounds.removeFromLeft(2);
    
    browserButton.setBounds(bounds.removeFromRight(browserWidth));
    bounds.removeFromRight(2);
    
    saveButton.setBounds(bounds.removeFromRight(saveWidth));
    bounds.removeFromRight(2);
    
    nextButton.setBounds(bounds.removeFromRight(buttonWidth));
    bounds.removeFromRight(2);
    
    presetCombo.setBounds(bounds);
}

void PresetSelector::refreshPresetList()
{
    auto currentName = presetManager.getCurrentPreset().name;
    
    presetCombo.clear(juce::dontSendNotification);
    
    const auto& presets = presetManager.getAllPresets();
    int selectedId = 0;
    int id = 1;
    
    for (const auto& preset : presets) {
        juce::String displayName = preset.name;
        if (preset.isFactory)
            displayName = "[F] " + displayName;
        
        presetCombo.addItem(displayName, id);
        
        if (preset.name == currentName)
            selectedId = id;
        
        id++;
    }
    
    if (selectedId > 0)
        presetCombo.setSelectedId(selectedId, juce::dontSendNotification);
}

void PresetSelector::buttonClicked(juce::Button* button)
{
    const auto& presets = presetManager.getAllPresets();
    
    if (button == &prevButton) {
        if (presets.empty()) return;
        
        currentPresetIndex--;
        if (currentPresetIndex < 0)
            currentPresetIndex = static_cast<int>(presets.size()) - 1;
        
        presetManager.loadPreset(presets[static_cast<size_t>(currentPresetIndex)]);
    }
    else if (button == &nextButton) {
        if (presets.empty()) return;
        
        currentPresetIndex++;
        if (currentPresetIndex >= static_cast<int>(presets.size()))
            currentPresetIndex = 0;
        
        presetManager.loadPreset(presets[static_cast<size_t>(currentPresetIndex)]);
    }
    else if (button == &saveButton) {
        showSaveDialog();
    }
    else if (button == &browserButton) {
        showPresetBrowser();
    }
}

void PresetSelector::comboBoxChanged(juce::ComboBox* comboBox)
{
    if (comboBox == &presetCombo) {
        auto selectedIndex = presetCombo.getSelectedItemIndex();
        if (selectedIndex >= 0) {
            const auto& presets = presetManager.getAllPresets();
            if (selectedIndex < static_cast<int>(presets.size())) {
                currentPresetIndex = selectedIndex;
                presetManager.loadPreset(presets[static_cast<size_t>(selectedIndex)]);
            }
        }
    }
}

void PresetSelector::showSaveDialog()
{
    auto* dialog = new SavePresetDialog(presetManager, [this]() {
        refreshPresetList();
    });
    
    auto* window = new juce::DialogWindow("Save Preset", juce::Colours::darkgrey, true);
    window->setContentOwned(dialog, true);
    window->centreAroundComponent(this, 400, 300);
    window->setVisible(true);
    window->setResizable(false, false);
}

void PresetSelector::showPresetBrowser()
{
    auto* browser = new PresetBrowserPanel(presetManager, []() {});
    
    auto* window = new juce::DialogWindow("Preset Browser", juce::Colours::darkgrey, true);
    window->setContentOwned(browser, true);
    window->centreAroundComponent(this, 600, 500);
    window->setVisible(true);
    window->setResizable(true, true);
}

//==============================================================================
// ABComparisonPanel Implementation

ABComparisonPanel::ABComparisonPanel(PresetManager& pm)
    : presetManager(pm)
{
    addAndMakeVisible(abToggleButton);
    addAndMakeVisible(copyABButton);
    addAndMakeVisible(copyBAButton);
    
    abToggleButton.addListener(this);
    copyABButton.addListener(this);
    copyBAButton.addListener(this);
    
    abToggleButton.setTooltip("Toggle between A and B states");
    copyABButton.setTooltip("Copy A settings to B");
    copyBAButton.setTooltip("Copy B settings to A");
    
    updateState();
}

ABComparisonPanel::~ABComparisonPanel()
{
    abToggleButton.removeListener(this);
    copyABButton.removeListener(this);
    copyBAButton.removeListener(this);
}

void ABComparisonPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2a2a2a));
    
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.drawRect(getLocalBounds());
}

void ABComparisonPanel::resized()
{
    auto bounds = getLocalBounds().reduced(4);
    auto buttonWidth = bounds.getWidth() / 3 - 2;
    
    abToggleButton.setBounds(bounds.removeFromLeft(buttonWidth));
    bounds.removeFromLeft(3);
    copyABButton.setBounds(bounds.removeFromLeft(buttonWidth));
    bounds.removeFromLeft(3);
    copyBAButton.setBounds(bounds);
}

void ABComparisonPanel::updateState()
{
    abToggleButton.setButtonText(presetManager.isShowingA() ? "A" : "B");
    
    // Highlight the active state
    auto activeColour = juce::Colour(0xff4a90d9);
    auto inactiveColour = juce::Colour(0xff3a3a3a);
    
    abToggleButton.setColour(juce::TextButton::buttonColourId, 
                              presetManager.isShowingA() ? activeColour : juce::Colour(0xffd94a4a));
}

void ABComparisonPanel::buttonClicked(juce::Button* button)
{
    if (button == &abToggleButton) {
        presetManager.toggleAB();
        updateState();
    }
    else if (button == &copyABButton) {
        presetManager.copyAToB();
    }
    else if (button == &copyBAButton) {
        presetManager.copyBToA();
    }
}

//==============================================================================
// SavePresetDialog Implementation

SavePresetDialog::SavePresetDialog(PresetManager& pm, std::function<void()> onComplete)
    : presetManager(pm), onSaveComplete(onComplete)
{
    addAndMakeVisible(nameLabel);
    addAndMakeVisible(categoryLabel);
    addAndMakeVisible(authorLabel);
    addAndMakeVisible(descriptionLabel);
    
    addAndMakeVisible(nameEditor);
    addAndMakeVisible(categoryCombo);
    addAndMakeVisible(authorEditor);
    addAndMakeVisible(descriptionEditor);
    
    addAndMakeVisible(saveButton);
    addAndMakeVisible(cancelButton);
    
    saveButton.addListener(this);
    cancelButton.addListener(this);
    
    // Setup category combo
    auto categories = PresetCategories::getAll();
    for (int i = 0; i < categories.size(); ++i) {
        categoryCombo.addItem(categories[i], i + 1);
    }
    categoryCombo.setSelectedId(categories.indexOf(PresetCategories::User) + 1);
    
    // Pre-fill with current preset info if available
    const auto& current = presetManager.getCurrentPreset();
    if (current.name.isNotEmpty()) {
        nameEditor.setText(current.name);
        authorEditor.setText(current.author);
        descriptionEditor.setText(current.description);
        
        auto catIndex = categories.indexOf(current.category);
        if (catIndex >= 0)
            categoryCombo.setSelectedId(catIndex + 1);
    }
    
    // Description is multiline
    descriptionEditor.setMultiLine(true);
    descriptionEditor.setReturnKeyStartsNewLine(true);
    
    setSize(400, 300);
}

SavePresetDialog::~SavePresetDialog()
{
    saveButton.removeListener(this);
    cancelButton.removeListener(this);
}

void SavePresetDialog::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2a2a2a));
}

void SavePresetDialog::resized()
{
    auto bounds = getLocalBounds().reduced(20);
    auto labelWidth = 100;
    auto rowHeight = 30;
    auto gap = 10;
    
    // Name row
    auto row = bounds.removeFromTop(rowHeight);
    nameLabel.setBounds(row.removeFromLeft(labelWidth));
    nameEditor.setBounds(row);
    bounds.removeFromTop(gap);
    
    // Category row
    row = bounds.removeFromTop(rowHeight);
    categoryLabel.setBounds(row.removeFromLeft(labelWidth));
    categoryCombo.setBounds(row);
    bounds.removeFromTop(gap);
    
    // Author row
    row = bounds.removeFromTop(rowHeight);
    authorLabel.setBounds(row.removeFromLeft(labelWidth));
    authorEditor.setBounds(row);
    bounds.removeFromTop(gap);
    
    // Description row (larger)
    descriptionLabel.setBounds(bounds.removeFromTop(20).withWidth(labelWidth));
    auto descBounds = bounds.removeFromTop(80);
    descriptionEditor.setBounds(descBounds);
    bounds.removeFromTop(gap);
    
    // Buttons
    auto buttonRow = bounds.removeFromBottom(35);
    auto buttonWidth = 80;
    
    saveButton.setBounds(buttonRow.removeFromRight(buttonWidth));
    buttonRow.removeFromRight(10);
    cancelButton.setBounds(buttonRow.removeFromRight(buttonWidth));
}

void SavePresetDialog::show()
{
    setVisible(true);
    nameEditor.grabKeyboardFocus();
}

void SavePresetDialog::buttonClicked(juce::Button* button)
{
    if (button == &saveButton) {
        if (validateAndSave()) {
            if (onSaveComplete)
                onSaveComplete();
            
            if (auto* window = findParentComponentOfClass<juce::DialogWindow>())
                window->closeButtonPressed();
        }
    }
    else if (button == &cancelButton) {
        if (auto* window = findParentComponentOfClass<juce::DialogWindow>())
            window->closeButtonPressed();
    }
}

bool SavePresetDialog::validateAndSave()
{
    auto name = nameEditor.getText().trim();
    if (name.isEmpty()) {
        juce::AlertWindow::showMessageBoxAsync(
            juce::MessageBoxIconType::WarningIcon,
            "Invalid Name",
            "Please enter a preset name.");
        return false;
    }
    
    auto categories = PresetCategories::getAll();
    auto category = categories[categoryCombo.getSelectedItemIndex()];
    auto author = authorEditor.getText().trim();
    auto description = descriptionEditor.getText().trim();
    
    return presetManager.savePreset(name, category, author, description);
}

//==============================================================================
// PresetBrowserPanel Implementation

PresetBrowserPanel::PresetBrowserPanel(PresetManager& pm, std::function<void()> onClose)
    : presetManager(pm), onCloseCallback(onClose)
{
    addAndMakeVisible(titleLabel);
    titleLabel.setFont(juce::Font(20.0f));
    titleLabel.setJustificationType(juce::Justification::centred);
    
    // Create category buttons
    categoryButtons.add(new juce::TextButton("All"));
    for (const auto& cat : PresetCategories::getAll()) {
        categoryButtons.add(new juce::TextButton(cat));
    }
    categoryButtons.add(new juce::TextButton("Factory"));
    categoryButtons.add(new juce::TextButton("User"));
    
    for (auto* btn : categoryButtons) {
        addAndMakeVisible(btn);
        btn->addListener(this);
    }
    
    addAndMakeVisible(searchBox);
    searchBox.setTextToShowWhenEmpty("Search presets...", juce::Colours::grey);
    searchBox.onTextChange = [this]() {
        auto search = searchBox.getText();
        if (search.isEmpty()) {
            filterByCategory(currentCategory);
        } else {
            filteredPresets = presetManager.searchPresets(search);
            presetList.updateContent();
        }
    };
    
    addAndMakeVisible(presetList);
    presetList.setModel(this);
    presetList.setRowHeight(28);
    presetList.setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff1a1a1a));
    
    addAndMakeVisible(loadButton);
    addAndMakeVisible(deleteButton);
    addAndMakeVisible(closeButton);
    
    loadButton.addListener(this);
    deleteButton.addListener(this);
    closeButton.addListener(this);
    
    // Start with "All" selected
    currentCategory = "";
    filterByCategory("");
    
    setSize(600, 500);
}

PresetBrowserPanel::~PresetBrowserPanel()
{
    for (auto* btn : categoryButtons)
        btn->removeListener(this);
    
    loadButton.removeListener(this);
    deleteButton.removeListener(this);
    closeButton.removeListener(this);
}

void PresetBrowserPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2a2a2a));
}

void PresetBrowserPanel::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    
    // Title
    titleLabel.setBounds(bounds.removeFromTop(35));
    bounds.removeFromTop(10);
    
    // Category buttons
    auto catRow = bounds.removeFromTop(30);
    auto buttonWidth = catRow.getWidth() / categoryButtons.size();
    for (auto* btn : categoryButtons) {
        btn->setBounds(catRow.removeFromLeft((int)buttonWidth).reduced(1));
    }
    bounds.removeFromTop(10);
    
    // Search box
    searchBox.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(10);
    
    // Bottom buttons
    auto buttonRow = bounds.removeFromBottom(35);
    auto btnWidth = 80;
    closeButton.setBounds(buttonRow.removeFromRight(btnWidth));
    buttonRow.removeFromRight(10);
    deleteButton.setBounds(buttonRow.removeFromRight(btnWidth));
    buttonRow.removeFromRight(10);
    loadButton.setBounds(buttonRow.removeFromRight(btnWidth));
    
    bounds.removeFromBottom(10);
    
    // Preset list
    presetList.setBounds(bounds);
}

int PresetBrowserPanel::getNumRows()
{
    return (int)filteredPresets.size();
}

void PresetBrowserPanel::paintListBoxItem(int rowNumber, juce::Graphics& g,
                                           int width, int height, bool rowIsSelected)
{
    if (rowNumber < 0 || rowNumber >= static_cast<int>(filteredPresets.size()))
        return;
    
    const auto& preset = filteredPresets[static_cast<size_t>(rowNumber)];
    
    // Background
    if (rowIsSelected) {
        g.fillAll(juce::Colour(0xff4a90d9));
    } else if (rowNumber % 2 == 0) {
        g.fillAll(juce::Colour(0xff252525));
    } else {
        g.fillAll(juce::Colour(0xff1f1f1f));
    }
    
    // Icon for factory vs user
    g.setColour(preset.isFactory ? juce::Colour(0xff90d94a) : juce::Colour(0xffd9d94a));
    g.fillEllipse(8.0f, (float)(height - 8) / 2.0f, 8.0f, 8.0f);
    
    // Preset name
    g.setColour(rowIsSelected ? juce::Colours::white : juce::Colours::lightgrey);
    g.setFont(juce::Font(14.0f));
    g.drawText(preset.name, 24, 0, width / 2 - 24, height, juce::Justification::centredLeft);
    
    // Category
    g.setColour(rowIsSelected ? juce::Colours::white.withAlpha(0.8f) : juce::Colours::grey);
    g.setFont(juce::Font(12.0f));
    g.drawText(preset.category, width / 2, 0, width / 4, height, juce::Justification::centredLeft);
    
    // Author
    if (preset.author.isNotEmpty()) {
        g.drawText(preset.author, width * 3 / 4, 0, width / 4 - 10, height, juce::Justification::centredRight);
    }
}

void PresetBrowserPanel::listBoxItemClicked(int row, const juce::MouseEvent& /*e*/)
{
    // Enable/disable delete button based on selection
    if (row >= 0 && row < static_cast<int>(filteredPresets.size())) {
        deleteButton.setEnabled(!filteredPresets[static_cast<size_t>(row)].isFactory);
    }
}

void PresetBrowserPanel::listBoxItemDoubleClicked(int row, const juce::MouseEvent& /*e*/)
{
    if (row >= 0 && row < static_cast<int>(filteredPresets.size())) {
        presetManager.loadPreset(filteredPresets[static_cast<size_t>(row)]);
    }
}

void PresetBrowserPanel::refreshPresetList()
{
    presetManager.refreshPresetList();
    filterByCategory(currentCategory);
}

void PresetBrowserPanel::buttonClicked(juce::Button* button)
{
    if (button == &loadButton) {
        auto selectedRow = presetList.getSelectedRow();
        if (selectedRow >= 0 && selectedRow < static_cast<int>(filteredPresets.size())) {
            presetManager.loadPreset(filteredPresets[static_cast<size_t>(selectedRow)]);
        }
    }
    else if (button == &deleteButton) {
        auto selectedRow = presetList.getSelectedRow();
        if (selectedRow >= 0 && selectedRow < static_cast<int>(filteredPresets.size())) {
            const auto& preset = filteredPresets[static_cast<size_t>(selectedRow)];
            if (!preset.isFactory) {
                if (juce::AlertWindow::showOkCancelBox(
                    juce::MessageBoxIconType::QuestionIcon,
                    "Delete Preset",
                    "Are you sure you want to delete \"" + preset.name + "\"?",
                    "Delete", "Cancel", this, nullptr)) {
                    presetManager.deletePreset(preset);
                    refreshPresetList();
                }
            }
        }
    }
    else if (button == &closeButton) {
        if (onCloseCallback)
            onCloseCallback();
        
        if (auto* window = findParentComponentOfClass<juce::DialogWindow>())
            window->closeButtonPressed();
    }
    else {
        // Category button
        for (int i = 0; i < categoryButtons.size(); ++i) {
            if (button == categoryButtons[i]) {
                auto text = button->getButtonText();
                
                if (text == "All") {
                    currentCategory = "";
                    filterByCategory("");
                }
                else if (text == "Factory") {
                    filteredPresets = presetManager.getFactoryPresets();
                    presetList.updateContent();
                }
                else if (text == "User") {
                    filteredPresets = presetManager.getUserPresets();
                    presetList.updateContent();
                }
                else {
                    currentCategory = text;
                    filterByCategory(text);
                }
                
                // Highlight selected button
                for (auto* btn : categoryButtons) {
                    btn->setToggleState(btn == button, juce::dontSendNotification);
                }
                
                break;
            }
        }
    }
}

void PresetBrowserPanel::filterByCategory(const juce::String& category)
{
    if (category.isEmpty()) {
        filteredPresets = std::vector<PresetInfo>(
            presetManager.getAllPresets().begin(),
            presetManager.getAllPresets().end());
    } else {
        filteredPresets = presetManager.getPresetsByCategory(category);
    }
    presetList.updateContent();
}

} // namespace SeshEQ
