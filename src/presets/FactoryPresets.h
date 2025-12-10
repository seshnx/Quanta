#pragma once

#include "PresetManager.h"
#include <juce_core/juce_core.h>

namespace SeshEQ {

/**
 * @brief Factory preset definitions
 * 
 * Contains all built-in presets organized by category.
 * These are embedded as XML strings and written to disk on first run.
 */
namespace FactoryPresets {

/**
 * @brief Create a preset XML string with given parameters
 */
juce::String createPresetXml(
    const juce::String& name,
    const juce::String& category,
    const juce::String& author,
    const juce::String& description,
    // Global parameters
    float inputGain = 0.0f,
    float outputGain = 0.0f,
    float dryWet = 100.0f,
    // EQ Band 1
    bool band1Enable = true, int band1Type = 6, float band1Freq = 80.0f, float band1Gain = 0.0f, float band1Q = 0.71f,
    // EQ Band 2
    bool band2Enable = true, int band2Type = 4, float band2Freq = 200.0f, float band2Gain = 0.0f, float band2Q = 1.0f,
    // EQ Band 3
    bool band3Enable = true, int band3Type = 4, float band3Freq = 500.0f, float band3Gain = 0.0f, float band3Q = 1.0f,
    // EQ Band 4
    bool band4Enable = true, int band4Type = 4, float band4Freq = 1000.0f, float band4Gain = 0.0f, float band4Q = 1.0f,
    // EQ Band 5
    bool band5Enable = true, int band5Type = 4, float band5Freq = 2000.0f, float band5Gain = 0.0f, float band5Q = 1.0f,
    // EQ Band 6
    bool band6Enable = true, int band6Type = 4, float band6Freq = 4000.0f, float band6Gain = 0.0f, float band6Q = 1.0f,
    // EQ Band 7
    bool band7Enable = true, int band7Type = 4, float band7Freq = 8000.0f, float band7Gain = 0.0f, float band7Q = 1.0f,
    // EQ Band 8
    bool band8Enable = true, int band8Type = 7, float band8Freq = 12000.0f, float band8Gain = 0.0f, float band8Q = 0.71f,
    // Compressor
    bool compEnable = false, float compThresh = 0.0f, float compRatio = 4.0f,
    float compAttack = 10.0f, float compRelease = 100.0f, float compKnee = 6.0f,
    float compMakeup = 0.0f, float compMix = 100.0f,
    // Gate
    bool gateEnable = false, float gateThresh = -60.0f, float gateRatio = 10.0f,
    float gateAttack = 0.5f, float gateHold = 50.0f, float gateRelease = 100.0f, float gateRange = -80.0f,
    // Limiter
    bool limEnable = false, float limCeiling = -0.3f, float limRelease = 100.0f,
    // Advanced
    int processingMode = 0, int oversampling = 0,
    int scFilterMode = 0, float scFilterFreq = 100.0f, float scFilterQ = 0.71f, bool scListen = false
);

/**
 * @brief Install all factory presets to the given directory
 */
void installAllPresets(const juce::File& factoryDirectory);

//==============================================================================
// Preset Categories

namespace Mixing {
    void install(const juce::File& dir);
}

namespace Mastering {
    void install(const juce::File& dir);
}

namespace Vocals {
    void install(const juce::File& dir);
}

namespace Drums {
    void install(const juce::File& dir);
}

namespace Bass {
    void install(const juce::File& dir);
}

namespace Guitar {
    void install(const juce::File& dir);
}

namespace Creative {
    void install(const juce::File& dir);
}

namespace Utility {
    void install(const juce::File& dir);
}

} // namespace FactoryPresets
} // namespace SeshEQ
