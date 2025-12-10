#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_data_structures/juce_data_structures.h>
#include <vector>
#include <functional>

namespace SeshEQ {

/**
 * @brief Preset metadata structure
 */
struct PresetInfo {
    juce::String name;
    juce::String author;
    juce::String category;
    juce::String description;
    juce::File file;
    bool isFactory = false;
    juce::Time dateModified;
    
    bool operator==(const PresetInfo& other) const {
        return file == other.file;
    }
};

/**
 * @brief Preset categories
 */
namespace PresetCategories {
    inline const juce::String Mixing = "Mixing";
    inline const juce::String Mastering = "Mastering";
    inline const juce::String Vocals = "Vocals";
    inline const juce::String Drums = "Drums";
    inline const juce::String Bass = "Bass";
    inline const juce::String Guitar = "Guitar";
    inline const juce::String Creative = "Creative";
    inline const juce::String Utility = "Utility";
    inline const juce::String User = "User";
    
    inline juce::StringArray getAll() {
        return { Mixing, Mastering, Vocals, Drums, Bass, Guitar, Creative, Utility, User };
    }
}

/**
 * @brief Manages presets for SeshEQ
 * 
 * Features:
 * - Save/load presets as XML files
 * - Factory presets (read-only)
 * - User presets
 * - Preset browsing with categories
 * - A/B comparison states
 */
class PresetManager {
public:
    PresetManager(juce::AudioProcessorValueTreeState& apvts);
    ~PresetManager() = default;
    
    //==========================================================================
    // Preset Operations
    
    /**
     * @brief Save current state as a preset
     * @param name Preset name
     * @param category Preset category
     * @param author Author name
     * @param description Optional description
     * @return True if save was successful
     */
    bool savePreset(const juce::String& name,
                    const juce::String& category = PresetCategories::User,
                    const juce::String& author = "",
                    const juce::String& description = "");
    
    /**
     * @brief Load a preset by file
     * @param presetFile The preset file to load
     * @return True if load was successful
     */
    bool loadPreset(const juce::File& presetFile);
    
    /**
     * @brief Load a preset by name
     * @param name Preset name
     * @return True if load was successful
     */
    bool loadPreset(const juce::String& name);
    
    /**
     * @brief Load preset from PresetInfo
     */
    bool loadPreset(const PresetInfo& preset);
    
    /**
     * @brief Delete a user preset
     * @return True if deletion was successful
     */
    bool deletePreset(const PresetInfo& preset);
    
    /**
     * @brief Rename a user preset
     */
    bool renamePreset(const PresetInfo& preset, const juce::String& newName);
    
    //==========================================================================
    // Preset Browsing
    
    /**
     * @brief Get all available presets
     */
    const std::vector<PresetInfo>& getAllPresets() const { return presets; }
    
    /**
     * @brief Get presets filtered by category
     */
    std::vector<PresetInfo> getPresetsByCategory(const juce::String& category) const;
    
    /**
     * @brief Get factory presets only
     */
    std::vector<PresetInfo> getFactoryPresets() const;
    
    /**
     * @brief Get user presets only
     */
    std::vector<PresetInfo> getUserPresets() const;
    
    /**
     * @brief Search presets by name
     */
    std::vector<PresetInfo> searchPresets(const juce::String& searchTerm) const;
    
    /**
     * @brief Refresh the preset list from disk
     */
    void refreshPresetList();
    
    /**
     * @brief Get the currently loaded preset
     */
    const PresetInfo& getCurrentPreset() const { return currentPreset; }
    
    /**
     * @brief Check if current state matches loaded preset
     */
    bool hasUnsavedChanges() const;
    
    //==========================================================================
    // A/B Comparison
    
    /**
     * @brief Store current state as A
     */
    void storeStateA();
    
    /**
     * @brief Store current state as B
     */
    void storeStateB();
    
    /**
     * @brief Recall state A
     */
    void recallStateA();
    
    /**
     * @brief Recall state B
     */
    void recallStateB();
    
    /**
     * @brief Copy A to B
     */
    void copyAToB();
    
    /**
     * @brief Copy B to A
     */
    void copyBToA();
    
    /**
     * @brief Check if currently showing A or B
     */
    bool isShowingA() const { return showingA; }
    
    /**
     * @brief Toggle between A and B
     */
    void toggleAB();
    
    //==========================================================================
    // Paths
    
    /**
     * @brief Get the factory presets directory
     */
    juce::File getFactoryPresetsDirectory() const;
    
    /**
     * @brief Get the user presets directory
     */
    juce::File getUserPresetsDirectory() const;
    
    //==========================================================================
    // Callbacks
    
    /**
     * @brief Set callback for when preset list changes
     */
    void setOnPresetListChanged(std::function<void()> callback) {
        onPresetListChanged = callback;
    }
    
    /**
     * @brief Set callback for when current preset changes
     */
    void setOnPresetChanged(std::function<void(const PresetInfo&)> callback) {
        onPresetChanged = callback;
    }
    
    //==========================================================================
    // Factory Preset Installation
    
    /**
     * @brief Install factory presets (call once on first run)
     */
    void installFactoryPresets();
    
    /**
     * @brief Check if factory presets are installed
     */
    bool areFactoryPresetsInstalled() const;

private:
    juce::ValueTree stateToValueTree() const;
    void valueTreeToState(const juce::ValueTree& tree);
    
    PresetInfo parsePresetFile(const juce::File& file) const;
    void scanDirectory(const juce::File& directory, bool isFactory);
    
    juce::AudioProcessorValueTreeState& apvts;
    
    std::vector<PresetInfo> presets;
    PresetInfo currentPreset;
    
    // A/B comparison states
    juce::ValueTree stateA;
    juce::ValueTree stateB;
    bool showingA = true;
    
    // Callbacks
    std::function<void()> onPresetListChanged;
    std::function<void(const PresetInfo&)> onPresetChanged;
    
    // File extension
    static inline const juce::String presetExtension = ".sesheq";
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};

} // namespace SeshEQ
