#include "PresetManager.h"
#include "Parameters.h"

namespace SeshEQ {

PresetManager::PresetManager(juce::AudioProcessorValueTreeState& apvts)
    : valueTreeState(apvts)
{
    createFactoryPresets();
}

juce::File PresetManager::getUserPresetsDirectory() const {
    auto presetDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                         .getChildFile("SeshNx")
                         .getChildFile("Quanta")
                         .getChildFile("Presets");

    if (!presetDir.exists()) {
        presetDir.createDirectory();
    }

    return presetDir;
}

void PresetManager::createFactoryPresets() {
    // Create factory presets with typical EQ curves

    // 1. Init - Flat response
    {
        FactoryPreset preset;
        preset.name = "Init";
        preset.category = "Default";
        // Init preset uses default parameter values - no changes needed
        factoryPresets.push_back(std::move(preset));
    }

    // 2. Vocal Presence
    {
        FactoryPreset preset;
        preset.name = "Vocal Presence";
        preset.category = "Vocals";
        auto xml = std::make_unique<juce::XmlElement>("Preset");
        xml->setAttribute("name", preset.name);
        // High-pass at 80Hz, boost at 3kHz for presence, slight air at 12kHz
        xml->setAttribute("band0_freq", 80.0f);
        xml->setAttribute("band0_type", 1); // HighPass
        xml->setAttribute("band0_gain", 0.0f);
        xml->setAttribute("band0_q", 0.7f);
        xml->setAttribute("band0_enabled", true);

        xml->setAttribute("band3_freq", 3000.0f);
        xml->setAttribute("band3_type", 4); // Peak
        xml->setAttribute("band3_gain", 3.0f);
        xml->setAttribute("band3_q", 1.0f);
        xml->setAttribute("band3_enabled", true);

        xml->setAttribute("band6_freq", 12000.0f);
        xml->setAttribute("band6_type", 4); // Peak
        xml->setAttribute("band6_gain", 2.0f);
        xml->setAttribute("band6_q", 0.8f);
        xml->setAttribute("band6_enabled", true);

        preset.state = std::move(xml);
        factoryPresets.push_back(std::move(preset));
    }

    // 3. Bass Enhancement
    {
        FactoryPreset preset;
        preset.name = "Bass Enhancement";
        preset.category = "Bass";
        auto xml = std::make_unique<juce::XmlElement>("Preset");
        xml->setAttribute("name", preset.name);

        xml->setAttribute("band0_freq", 60.0f);
        xml->setAttribute("band0_type", 5); // LowShelf
        xml->setAttribute("band0_gain", 4.0f);
        xml->setAttribute("band0_q", 0.7f);
        xml->setAttribute("band0_enabled", true);

        xml->setAttribute("band1_freq", 200.0f);
        xml->setAttribute("band1_type", 4); // Peak
        xml->setAttribute("band1_gain", 2.0f);
        xml->setAttribute("band1_q", 1.5f);
        xml->setAttribute("band1_enabled", true);

        preset.state = std::move(xml);
        factoryPresets.push_back(std::move(preset));
    }

    // 4. Bright Master
    {
        FactoryPreset preset;
        preset.name = "Bright Master";
        preset.category = "Mastering";
        auto xml = std::make_unique<juce::XmlElement>("Preset");
        xml->setAttribute("name", preset.name);

        xml->setAttribute("band0_freq", 30.0f);
        xml->setAttribute("band0_type", 1); // HighPass
        xml->setAttribute("band0_gain", 0.0f);
        xml->setAttribute("band0_q", 0.7f);
        xml->setAttribute("band0_enabled", true);

        xml->setAttribute("band5_freq", 8000.0f);
        xml->setAttribute("band5_type", 6); // HighShelf
        xml->setAttribute("band5_gain", 2.5f);
        xml->setAttribute("band5_q", 0.7f);
        xml->setAttribute("band5_enabled", true);

        xml->setAttribute("band7_freq", 16000.0f);
        xml->setAttribute("band7_type", 4); // Peak
        xml->setAttribute("band7_gain", 1.5f);
        xml->setAttribute("band7_q", 0.5f);
        xml->setAttribute("band7_enabled", true);

        preset.state = std::move(xml);
        factoryPresets.push_back(std::move(preset));
    }

    // 5. Warm Analog
    {
        FactoryPreset preset;
        preset.name = "Warm Analog";
        preset.category = "Character";
        auto xml = std::make_unique<juce::XmlElement>("Preset");
        xml->setAttribute("name", preset.name);

        xml->setAttribute("band0_freq", 100.0f);
        xml->setAttribute("band0_type", 5); // LowShelf
        xml->setAttribute("band0_gain", 2.0f);
        xml->setAttribute("band0_q", 0.7f);
        xml->setAttribute("band0_enabled", true);

        xml->setAttribute("band4_freq", 4000.0f);
        xml->setAttribute("band4_type", 4); // Peak
        xml->setAttribute("band4_gain", -1.5f);
        xml->setAttribute("band4_q", 1.0f);
        xml->setAttribute("band4_enabled", true);

        xml->setAttribute("band7_freq", 14000.0f);
        xml->setAttribute("band7_type", 6); // HighShelf
        xml->setAttribute("band7_gain", -2.0f);
        xml->setAttribute("band7_q", 0.7f);
        xml->setAttribute("band7_enabled", true);

        preset.state = std::move(xml);
        factoryPresets.push_back(std::move(preset));
    }

    // 6. De-Mud (remove muddiness)
    {
        FactoryPreset preset;
        preset.name = "De-Mud";
        preset.category = "Corrective";
        auto xml = std::make_unique<juce::XmlElement>("Preset");
        xml->setAttribute("name", preset.name);

        xml->setAttribute("band1_freq", 250.0f);
        xml->setAttribute("band1_type", 4); // Peak
        xml->setAttribute("band1_gain", -3.0f);
        xml->setAttribute("band1_q", 1.5f);
        xml->setAttribute("band1_enabled", true);

        xml->setAttribute("band2_freq", 400.0f);
        xml->setAttribute("band2_type", 4); // Peak
        xml->setAttribute("band2_gain", -2.0f);
        xml->setAttribute("band2_q", 2.0f);
        xml->setAttribute("band2_enabled", true);

        preset.state = std::move(xml);
        factoryPresets.push_back(std::move(preset));
    }
}

void PresetManager::savePreset(const juce::String& presetName) {
    auto presetFile = getUserPresetsDirectory().getChildFile(presetName + ".xml");

    auto state = valueTreeState.copyState();
    auto xml = state.createXml();

    if (xml) {
        xml->setAttribute("presetName", presetName);
        xml->writeTo(presetFile);
    }

    currentPresetName = presetName;
    presetModified = false;
}

void PresetManager::loadPreset(const juce::String& presetName) {
    // First check factory presets
    for (size_t i = 0; i < factoryPresets.size(); ++i) {
        if (factoryPresets[i].name == presetName) {
            loadFactoryPreset(static_cast<int>(i));
            return;
        }
    }

    // Then check user presets
    auto presetFile = getUserPresetsDirectory().getChildFile(presetName + ".xml");

    if (presetFile.existsAsFile()) {
        auto xml = juce::XmlDocument::parse(presetFile);
        if (xml) {
            valueTreeState.replaceState(juce::ValueTree::fromXml(*xml));
            currentPresetName = presetName;
            currentPresetIndex = -1; // User preset
            presetModified = false;
        }
    }
}

void PresetManager::deletePreset(const juce::String& presetName) {
    auto presetFile = getUserPresetsDirectory().getChildFile(presetName + ".xml");

    if (presetFile.existsAsFile()) {
        presetFile.deleteFile();
    }
}

void PresetManager::loadFactoryPreset(int index) {
    if (index < 0 || index >= static_cast<int>(factoryPresets.size())) {
        return;
    }

    const auto& preset = factoryPresets[static_cast<size_t>(index)];

    if (index == 0) {
        // Init preset - reset to defaults
        initializeDefaultPreset();
    } else if (preset.state) {
        // Apply preset-specific settings
        // First reset to defaults, then apply preset overrides
        initializeDefaultPreset();

        // Apply the preset settings
        for (int band = 0; band < 8; ++band) {
            auto freqAttr = "band" + juce::String(band) + "_freq";
            auto typeAttr = "band" + juce::String(band) + "_type";
            auto gainAttr = "band" + juce::String(band) + "_gain";
            auto qAttr = "band" + juce::String(band) + "_q";
            auto enabledAttr = "band" + juce::String(band) + "_enabled";

            if (preset.state->hasAttribute(freqAttr)) {
                if (auto* param = valueTreeState.getParameter(ParamIDs::getBandParamID(band, ParamIDs::bandFreq))) {
                    param->setValueNotifyingHost(param->convertTo0to1(preset.state->getDoubleAttribute(freqAttr)));
                }
            }
            if (preset.state->hasAttribute(typeAttr)) {
                if (auto* param = valueTreeState.getParameter(ParamIDs::getBandParamID(band, ParamIDs::bandType))) {
                    param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(preset.state->getIntAttribute(typeAttr))));
                }
            }
            if (preset.state->hasAttribute(gainAttr)) {
                if (auto* param = valueTreeState.getParameter(ParamIDs::getBandParamID(band, ParamIDs::bandGain))) {
                    param->setValueNotifyingHost(param->convertTo0to1(preset.state->getDoubleAttribute(gainAttr)));
                }
            }
            if (preset.state->hasAttribute(qAttr)) {
                if (auto* param = valueTreeState.getParameter(ParamIDs::getBandParamID(band, ParamIDs::bandQ))) {
                    param->setValueNotifyingHost(param->convertTo0to1(preset.state->getDoubleAttribute(qAttr)));
                }
            }
            if (preset.state->hasAttribute(enabledAttr)) {
                if (auto* param = valueTreeState.getParameter(ParamIDs::getBandParamID(band, ParamIDs::bandEnable))) {
                    param->setValueNotifyingHost(preset.state->getBoolAttribute(enabledAttr) ? 1.0f : 0.0f);
                }
            }
        }
    }

    currentPresetName = preset.name;
    currentPresetIndex = index;
    presetModified = false;
}

void PresetManager::initializeDefaultPreset() {
    // Reset all parameters to their default values
    for (int band = 0; band < 8; ++band) {
        if (auto* param = valueTreeState.getParameter(ParamIDs::getBandParamID(band, ParamIDs::bandGain))) {
            param->setValueNotifyingHost(param->convertTo0to1(0.0f)); // 0 dB gain
        }
        if (auto* param = valueTreeState.getParameter(ParamIDs::getBandParamID(band, ParamIDs::bandQ))) {
            param->setValueNotifyingHost(param->convertTo0to1(0.707f)); // Default Q
        }
        if (auto* param = valueTreeState.getParameter(ParamIDs::getBandParamID(band, ParamIDs::bandType))) {
            param->setValueNotifyingHost(param->convertTo0to1(4.0f)); // Peak filter
        }
        if (auto* param = valueTreeState.getParameter(ParamIDs::getBandParamID(band, ParamIDs::bandEnable))) {
            param->setValueNotifyingHost(1.0f); // Enabled
        }
        // Reset per-band dynamics
        if (auto* param = valueTreeState.getParameter(ParamIDs::getBandParamID(band, ParamIDs::bandDynThreshold))) {
            param->setValueNotifyingHost(param->convertTo0to1(-20.0f));
        }
        if (auto* param = valueTreeState.getParameter(ParamIDs::getBandParamID(band, ParamIDs::bandDynRatio))) {
            param->setValueNotifyingHost(param->convertTo0to1(1.0f));
        }
        if (auto* param = valueTreeState.getParameter(ParamIDs::getBandParamID(band, ParamIDs::bandDynEnable))) {
            param->setValueNotifyingHost(0.0f); // Disabled by default
        }
    }

    // Reset global controls
    if (auto* param = valueTreeState.getParameter(ParamIDs::inputGain)) {
        param->setValueNotifyingHost(param->convertTo0to1(0.0f));
    }
    if (auto* param = valueTreeState.getParameter(ParamIDs::outputGain)) {
        param->setValueNotifyingHost(param->convertTo0to1(0.0f));
    }
    if (auto* param = valueTreeState.getParameter(ParamIDs::dryWet)) {
        param->setValueNotifyingHost(param->convertTo0to1(100.0f));
    }
    if (auto* param = valueTreeState.getParameter(ParamIDs::bypass)) {
        param->setValueNotifyingHost(0.0f);
    }

    currentPresetName = "Init";
    currentPresetIndex = 0;
    presetModified = false;
}

juce::StringArray PresetManager::getFactoryPresetNames() const {
    juce::StringArray names;
    for (const auto& preset : factoryPresets) {
        names.add(preset.name);
    }
    return names;
}

int PresetManager::getNumFactoryPresets() const {
    return static_cast<int>(factoryPresets.size());
}

juce::StringArray PresetManager::getUserPresetNames() const {
    juce::StringArray names;
    auto presetDir = getUserPresetsDirectory();

    for (const auto& file : presetDir.findChildFiles(juce::File::findFiles, false, "*.xml")) {
        names.add(file.getFileNameWithoutExtension());
    }

    names.sort(true);
    return names;
}

int PresetManager::getNumUserPresets() const {
    return getUserPresetNames().size();
}

juce::StringArray PresetManager::getAllPresetNames() const {
    juce::StringArray names;

    // Add factory presets first
    names.addArray(getFactoryPresetNames());

    // Add separator if there are user presets
    auto userPresets = getUserPresetNames();
    if (userPresets.size() > 0) {
        names.add("---"); // Separator
        names.addArray(userPresets);
    }

    return names;
}

} // namespace SeshEQ
