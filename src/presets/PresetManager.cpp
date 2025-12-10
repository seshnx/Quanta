#include "PresetManager.h"
#include "FactoryPresets.h"

namespace SeshEQ {

PresetManager::PresetManager(juce::AudioProcessorValueTreeState& apvts_)
    : apvts(apvts_)
{
    // Initialize A/B states with current state
    stateA = apvts.copyState();
    stateB = apvts.copyState();
    
    // Create preset directories if they don't exist
    getUserPresetsDirectory().createDirectory();
    getFactoryPresetsDirectory().createDirectory();
    
    // Install factory presets if not present
    if (!areFactoryPresetsInstalled()) {
        installFactoryPresets();
    }
    
    // Scan for presets
    refreshPresetList();
}

//==============================================================================
// Preset Operations

bool PresetManager::savePreset(const juce::String& name,
                                const juce::String& category,
                                const juce::String& author,
                                const juce::String& description)
{
    if (name.isEmpty())
        return false;
    
    auto presetFile = getUserPresetsDirectory()
                        .getChildFile(name + presetExtension);
    
    // Create XML document
    auto state = apvts.copyState();
    
    // Add metadata
    state.setProperty("presetName", name, nullptr);
    state.setProperty("presetAuthor", author, nullptr);
    state.setProperty("presetCategory", category, nullptr);
    state.setProperty("presetDescription", description, nullptr);
    state.setProperty("presetVersion", "1.0", nullptr);
    state.setProperty("dateCreated", juce::Time::getCurrentTime().toISO8601(true), nullptr);
    
    // Convert to XML
    auto xml = state.createXml();
    if (xml == nullptr)
        return false;
    
    // Write to file
    if (!xml->writeTo(presetFile)) {
        return false;
    }
    
    // Update current preset info
    currentPreset.name = name;
    currentPreset.author = author;
    currentPreset.category = category;
    currentPreset.description = description;
    currentPreset.file = presetFile;
    currentPreset.isFactory = false;
    currentPreset.dateModified = juce::Time::getCurrentTime();
    
    // Refresh preset list
    refreshPresetList();
    
    if (onPresetChanged)
        onPresetChanged(currentPreset);
    
    return true;
}

bool PresetManager::loadPreset(const juce::File& presetFile)
{
    if (!presetFile.existsAsFile())
        return false;
    
    // Parse XML
    auto xml = juce::XmlDocument::parse(presetFile);
    if (xml == nullptr)
        return false;
    
    // Convert to ValueTree
    auto state = juce::ValueTree::fromXml(*xml);
    if (!state.isValid())
        return false;
    
    // Apply state to APVTS
    apvts.replaceState(state);
    
    // Update current preset info
    currentPreset = parsePresetFile(presetFile);
    
    if (onPresetChanged)
        onPresetChanged(currentPreset);
    
    return true;
}

bool PresetManager::loadPreset(const juce::String& name)
{
    for (const auto& preset : presets) {
        if (preset.name == name) {
            return loadPreset(preset.file);
        }
    }
    return false;
}

bool PresetManager::loadPreset(const PresetInfo& preset)
{
    return loadPreset(preset.file);
}

bool PresetManager::deletePreset(const PresetInfo& preset)
{
    if (preset.isFactory)
        return false; // Can't delete factory presets
    
    if (!preset.file.existsAsFile())
        return false;
    
    if (preset.file.deleteFile()) {
        refreshPresetList();
        return true;
    }
    
    return false;
}

bool PresetManager::renamePreset(const PresetInfo& preset, const juce::String& newName)
{
    if (preset.isFactory || newName.isEmpty())
        return false;
    
    auto newFile = preset.file.getParentDirectory()
                    .getChildFile(newName + presetExtension);
    
    if (newFile.exists())
        return false; // Don't overwrite
    
    // Load the preset XML
    auto xml = juce::XmlDocument::parse(preset.file);
    if (xml == nullptr)
        return false;
    
    // Update name in XML
    xml->setAttribute("presetName", newName);
    
    // Write to new file
    if (!xml->writeTo(newFile))
        return false;
    
    // Delete old file
    preset.file.deleteFile();
    
    refreshPresetList();
    return true;
}

//==============================================================================
// Preset Browsing

std::vector<PresetInfo> PresetManager::getPresetsByCategory(const juce::String& category) const
{
    std::vector<PresetInfo> filtered;
    for (const auto& preset : presets) {
        if (preset.category == category) {
            filtered.push_back(preset);
        }
    }
    return filtered;
}

std::vector<PresetInfo> PresetManager::getFactoryPresets() const
{
    std::vector<PresetInfo> factory;
    for (const auto& preset : presets) {
        if (preset.isFactory) {
            factory.push_back(preset);
        }
    }
    return factory;
}

std::vector<PresetInfo> PresetManager::getUserPresets() const
{
    std::vector<PresetInfo> user;
    for (const auto& preset : presets) {
        if (!preset.isFactory) {
            user.push_back(preset);
        }
    }
    return user;
}

std::vector<PresetInfo> PresetManager::searchPresets(const juce::String& searchTerm) const
{
    std::vector<PresetInfo> results;
    auto lowerSearch = searchTerm.toLowerCase();
    
    for (const auto& preset : presets) {
        if (preset.name.toLowerCase().contains(lowerSearch) ||
            preset.category.toLowerCase().contains(lowerSearch) ||
            preset.description.toLowerCase().contains(lowerSearch)) {
            results.push_back(preset);
        }
    }
    return results;
}

void PresetManager::refreshPresetList()
{
    presets.clear();
    
    // Scan factory presets
    scanDirectory(getFactoryPresetsDirectory(), true);
    
    // Scan user presets
    scanDirectory(getUserPresetsDirectory(), false);
    
    // Sort by name
    std::sort(presets.begin(), presets.end(), [](const PresetInfo& a, const PresetInfo& b) {
        // Factory presets first, then alphabetically
        if (a.isFactory != b.isFactory)
            return a.isFactory;
        return a.name.compareIgnoreCase(b.name) < 0;
    });
    
    if (onPresetListChanged)
        onPresetListChanged();
}

bool PresetManager::hasUnsavedChanges() const
{
    if (currentPreset.file == juce::File())
        return true; // No preset loaded
    
    // Compare current state with loaded preset
    auto currentState = apvts.copyState();
    
    auto xml = juce::XmlDocument::parse(currentPreset.file);
    if (xml == nullptr)
        return true;
    
    auto savedState = juce::ValueTree::fromXml(*xml);
    
    // Simple comparison - could be more sophisticated
    return !currentState.isEquivalentTo(savedState);
}

//==============================================================================
// A/B Comparison

void PresetManager::storeStateA()
{
    stateA = apvts.copyState();
}

void PresetManager::storeStateB()
{
    stateB = apvts.copyState();
}

void PresetManager::recallStateA()
{
    apvts.replaceState(stateA.createCopy());
    showingA = true;
}

void PresetManager::recallStateB()
{
    apvts.replaceState(stateB.createCopy());
    showingA = false;
}

void PresetManager::copyAToB()
{
    stateB = stateA.createCopy();
}

void PresetManager::copyBToA()
{
    stateA = stateB.createCopy();
}

void PresetManager::toggleAB()
{
    if (showingA) {
        storeStateA();
        recallStateB();
    } else {
        storeStateB();
        recallStateA();
    }
}

//==============================================================================
// Paths

juce::File PresetManager::getFactoryPresetsDirectory() const
{
#if JUCE_MAC
    return juce::File::getSpecialLocation(juce::File::commonApplicationDataDirectory)
            .getChildFile("Application Support")
            .getChildFile("SeshEQ")
            .getChildFile("Factory Presets");
#elif JUCE_WINDOWS
    return juce::File::getSpecialLocation(juce::File::commonApplicationDataDirectory)
            .getChildFile("SeshEQ")
            .getChildFile("Factory Presets");
#else
    return juce::File::getSpecialLocation(juce::File::commonApplicationDataDirectory)
            .getChildFile("SeshEQ")
            .getChildFile("Factory Presets");
#endif
}

juce::File PresetManager::getUserPresetsDirectory() const
{
#if JUCE_MAC
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("Application Support")
            .getChildFile("SeshEQ")
            .getChildFile("User Presets");
#elif JUCE_WINDOWS
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("SeshEQ")
            .getChildFile("User Presets");
#else
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("SeshEQ")
            .getChildFile("User Presets");
#endif
}

//==============================================================================
// Factory Preset Installation

bool PresetManager::areFactoryPresetsInstalled() const
{
    auto factoryDir = getFactoryPresetsDirectory();
    return factoryDir.exists() && 
           factoryDir.getNumberOfChildFiles(juce::File::findFiles, "*" + presetExtension) > 0;
}

void PresetManager::installFactoryPresets()
{
    auto factoryDir = getFactoryPresetsDirectory();
    factoryDir.createDirectory();
    
    // Install all factory presets from embedded data
    FactoryPresets::installAllPresets(factoryDir);
}

//==============================================================================
// Private Methods

PresetInfo PresetManager::parsePresetFile(const juce::File& file) const
{
    PresetInfo info;
    info.file = file;
    info.dateModified = file.getLastModificationTime();
    
    auto xml = juce::XmlDocument::parse(file);
    if (xml != nullptr) {
        auto state = juce::ValueTree::fromXml(*xml);
        info.name = state.getProperty("presetName", file.getFileNameWithoutExtension());
        info.author = state.getProperty("presetAuthor", "");
        info.category = state.getProperty("presetCategory", PresetCategories::User);
        info.description = state.getProperty("presetDescription", "");
    } else {
        info.name = file.getFileNameWithoutExtension();
        info.category = PresetCategories::User;
    }
    
    // Determine if factory based on path
    info.isFactory = file.getFullPathName().contains("Factory Presets");
    
    return info;
}

void PresetManager::scanDirectory(const juce::File& directory, bool isFactory)
{
    if (!directory.exists())
        return;
    
    for (const auto& entry : juce::RangedDirectoryIterator(directory, false, "*" + presetExtension)) {
        auto info = parsePresetFile(entry.getFile());
        info.isFactory = isFactory;
        presets.push_back(info);
    }
}

} // namespace SeshEQ
