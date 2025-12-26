#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_data_structures/juce_data_structures.h>

namespace SeshEQ {

class PresetManager {
public:
    PresetManager(juce::AudioProcessorValueTreeState& apvts);
    ~PresetManager() = default;

    // Preset operations
    void savePreset(const juce::String& presetName);
    void loadPreset(const juce::String& presetName);
    void deletePreset(const juce::String& presetName);

    // Factory presets
    void loadFactoryPreset(int index);
    juce::StringArray getFactoryPresetNames() const;
    int getNumFactoryPresets() const;

    // User presets
    juce::StringArray getUserPresetNames() const;
    int getNumUserPresets() const;

    // All presets combined
    juce::StringArray getAllPresetNames() const;
    int getCurrentPresetIndex() const { return currentPresetIndex; }
    juce::String getCurrentPresetName() const { return currentPresetName; }

    // Preset state
    bool isCurrentPresetModified() const { return presetModified; }
    void setPresetModified(bool modified) { presetModified = modified; }

    // Initialize with default preset
    void initializeDefaultPreset();

    // Get preset directory
    juce::File getUserPresetsDirectory() const;

private:
    void createFactoryPresets();
    juce::XmlElement* createPresetXml(const juce::String& name) const;
    void loadPresetFromXml(const juce::XmlElement& xml);

    juce::AudioProcessorValueTreeState& valueTreeState;

    // Factory presets stored in memory
    struct FactoryPreset {
        juce::String name;
        juce::String category;
        std::unique_ptr<juce::XmlElement> state;
    };
    std::vector<FactoryPreset> factoryPresets;

    juce::String currentPresetName = "Init";
    int currentPresetIndex = 0;
    bool presetModified = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};

} // namespace SeshEQ
