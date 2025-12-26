#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "dsp/BiquadFilter.h"  // For FilterType
#include <array>
#include <string>

namespace SeshEQ {

//==============================================================================
// Parameter IDs
//==============================================================================
namespace ParamIDs {
    // Global
    inline const juce::String inputGain  = "inputGain";
    inline const juce::String outputGain = "outputGain";
    inline const juce::String dryWet     = "dryWet";
    inline const juce::String bypass     = "bypass";
    
    // Advanced features
    inline const juce::String midSideMode = "midSideMode";
    inline const juce::String linearPhaseMode = "linearPhaseMode";
    inline const juce::String dynamicEQMode = "dynamicEQMode";

    // EQ Band parameters (use getBandParamID to get full ID)
    inline const juce::String bandFreq   = "freq";
    inline const juce::String bandGain   = "gain";
    inline const juce::String bandQ      = "q";
    inline const juce::String bandType   = "type";
    inline const juce::String bandEnable = "enable";
    
    // Per-band Multiband Dynamics parameters (Compressor/Expander)
    inline const juce::String bandDynThreshold = "dynThreshold";
    inline const juce::String bandDynRatio = "dynRatio";
    inline const juce::String bandDynAttack = "dynAttack";
    inline const juce::String bandDynRelease = "dynRelease";
    inline const juce::String bandDynKnee = "dynKnee";
    inline const juce::String bandDynEnable = "dynEnable";

    // Compressor
    inline const juce::String compThreshold = "compThreshold";
    inline const juce::String compRatio     = "compRatio";
    inline const juce::String compAttack    = "compAttack";
    inline const juce::String compRelease   = "compRelease";
    inline const juce::String compKnee      = "compKnee";
    inline const juce::String compMakeup    = "compMakeup";
    inline const juce::String compMix       = "compMix";
    inline const juce::String compEnable    = "compEnable";

    // Gate
    inline const juce::String gateThreshold = "gateThreshold";
    inline const juce::String gateRatio     = "gateRatio";
    inline const juce::String gateAttack    = "gateAttack";
    inline const juce::String gateHold      = "gateHold";
    inline const juce::String gateRelease   = "gateRelease";
    inline const juce::String gateRange     = "gateRange";
    inline const juce::String gateEnable    = "gateEnable";

    // True Peak Limiter
    inline const juce::String limiterThreshold = "limiterThreshold";
    inline const juce::String limiterCeiling = "limiterCeiling";
    inline const juce::String limiterRelease = "limiterRelease";
    inline const juce::String limiterEnable  = "limiterEnable";

    // Global Oversampling
    inline const juce::String oversamplingFactor = "oversamplingFactor";

    // Helper function to get band-specific parameter ID
    inline juce::String getBandParamID(int bandIndex, const juce::String& param) {
        return "band" + juce::String(bandIndex) + "_" + param;
    }
}

inline juce::StringArray getFilterTypeNames() {
    return { "Low Pass", "High Pass", "Band Pass", "Notch",
             "Peak", "Low Shelf", "High Shelf", "All Pass" };
}

inline juce::StringArray getOversamplingNames() {
    return { "1x (Off)", "2x", "4x", "8x" };
}

//==============================================================================
// Constants
//==============================================================================
namespace Constants {
    constexpr int numEQBands = 8;
    
    // Frequency range
    constexpr float minFrequency = 20.0f;
    constexpr float maxFrequency = 20000.0f;
    
    // Gain range
    constexpr float minGain = -24.0f;
    constexpr float maxGain = 24.0f;
    
    // Q range
    constexpr float minQ = 0.1f;
    constexpr float maxQ = 18.0f;
    constexpr float defaultQ = 0.707f; // Butterworth
    
    // Default filter types for each band
    constexpr std::array<FilterType, numEQBands> defaultBandTypes = {
        FilterType::HighPass,   // Band 1 - HPF
        FilterType::LowShelf,   // Band 2 - Low Shelf
        FilterType::Peak,       // Band 3 - Bell
        FilterType::Peak,       // Band 4 - Bell
        FilterType::Peak,       // Band 5 - Bell
        FilterType::Peak,       // Band 6 - Bell
        FilterType::HighShelf,  // Band 7 - High Shelf
        FilterType::LowPass     // Band 8 - LPF
    };
    
    // Default frequencies for each band
    constexpr std::array<float, numEQBands> defaultBandFrequencies = {
        80.0f,     // Band 1
        200.0f,    // Band 2
        500.0f,    // Band 3
        1000.0f,   // Band 4
        2500.0f,   // Band 5
        5000.0f,   // Band 6
        10000.0f,  // Band 7
        16000.0f   // Band 8
    };
    
    // Compressor defaults
    constexpr float defaultCompThreshold = -18.0f;
    constexpr float defaultCompRatio = 4.0f;
    constexpr float defaultCompAttack = 10.0f;
    constexpr float defaultCompRelease = 100.0f;
    constexpr float defaultCompKnee = 6.0f;
    
    // Gate defaults
    constexpr float defaultGateThreshold = -40.0f;
    constexpr float defaultGateRatio = 10.0f;
    constexpr float defaultGateAttack = 0.5f;
    constexpr float defaultGateHold = 50.0f;
    constexpr float defaultGateRelease = 100.0f;
    constexpr float defaultGateRange = -80.0f;
    
    // Limiter defaults
    constexpr float defaultLimiterCeiling = -0.3f;
    constexpr float defaultLimiterRelease = 100.0f;
}

//==============================================================================
// Parameter Layout Builder
//==============================================================================
class ParameterLayout {
public:
    static juce::AudioProcessorValueTreeState::ParameterLayout create();
    
private:
    static void addGlobalParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);
    static void addEQParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);
    static void addCompressorParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);
    static void addGateParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);
    static void addLimiterParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);
};

} // namespace SeshEQ
