#include "Parameters.h"

namespace SeshEQ {

juce::AudioProcessorValueTreeState::ParameterLayout ParameterLayout::create() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    addGlobalParameters(layout);
    addEQParameters(layout);
    addCompressorParameters(layout);
    addGateParameters(layout);
    addLimiterParameters(layout);
    addAdvancedDSPParameters(layout);
    
    return layout;
}

void ParameterLayout::addGlobalParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout) {
    using namespace ParamIDs;
    
    // Input Gain: -24 to +24 dB
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(inputGain, 1),
        "Input Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")
    ));
    
    // Output Gain: -24 to +24 dB
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(outputGain, 1),
        "Output Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")
    ));
    
    // Dry/Wet Mix: 0-100%
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(dryWet, 1),
        "Dry/Wet",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        100.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")
    ));
    
    // Global Bypass
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(bypass, 1),
        "Bypass",
        false
    ));
}

void ParameterLayout::addEQParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout) {
    using namespace ParamIDs;
    using namespace Constants;
    
    // Frequency range with skew for logarithmic feel
    juce::NormalisableRange<float> freqRange(minFrequency, maxFrequency);
    freqRange.setSkewForCentre(1000.0f); // 1kHz at center
    
    // Q range with skew
    juce::NormalisableRange<float> qRange(minQ, maxQ, 0.01f);
    qRange.setSkewForCentre(1.0f);
    
    for (int i = 0; i < numEQBands; ++i) {
        const auto bandStr = juce::String(i);
        
        // Frequency
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(getBandParamID(i, bandFreq), 1),
            "Band " + bandStr + " Freq",
            freqRange,
            defaultBandFrequencies[static_cast<size_t>(i)],
            juce::AudioParameterFloatAttributes().withLabel("Hz")
        ));
        
        // Gain
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(getBandParamID(i, bandGain), 1),
            "Band " + bandStr + " Gain",
            juce::NormalisableRange<float>(minGain, maxGain, 0.1f),
            0.0f,
            juce::AudioParameterFloatAttributes().withLabel("dB")
        ));
        
        // Q
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(getBandParamID(i, bandQ), 1),
            "Band " + bandStr + " Q",
            qRange,
            defaultQ,
            juce::AudioParameterFloatAttributes()
        ));
        
        // Filter Type
        layout.add(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID(getBandParamID(i, bandType), 1),
            "Band " + bandStr + " Type",
            getFilterTypeNames(),
            static_cast<int>(defaultBandTypes[static_cast<size_t>(i)])
        ));
        
        // Enable
        layout.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID(getBandParamID(i, bandEnable), 1),
            "Band " + bandStr + " Enable",
            true
        ));
    }
}

void ParameterLayout::addCompressorParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout) {
    using namespace ParamIDs;
    using namespace Constants;
    
    // Threshold: -60 to 0 dB
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(compThreshold, 1),
        "Comp Threshold",
        juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f),
        defaultCompThreshold,
        juce::AudioParameterFloatAttributes().withLabel("dB")
    ));
    
    // Ratio: 1:1 to 20:1 (infinity approximation)
    juce::NormalisableRange<float> ratioRange(1.0f, 20.0f, 0.1f);
    ratioRange.setSkewForCentre(4.0f);
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(compRatio, 1),
        "Comp Ratio",
        ratioRange,
        defaultCompRatio,
        juce::AudioParameterFloatAttributes().withLabel(":1")
    ));
    
    // Attack: 0.01 to 100 ms
    juce::NormalisableRange<float> attackRange(0.01f, 100.0f, 0.01f);
    attackRange.setSkewForCentre(10.0f);
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(compAttack, 1),
        "Comp Attack",
        attackRange,
        defaultCompAttack,
        juce::AudioParameterFloatAttributes().withLabel("ms")
    ));
    
    // Release: 10 to 3000 ms
    juce::NormalisableRange<float> releaseRange(10.0f, 3000.0f, 1.0f);
    releaseRange.setSkewForCentre(200.0f);
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(compRelease, 1),
        "Comp Release",
        releaseRange,
        defaultCompRelease,
        juce::AudioParameterFloatAttributes().withLabel("ms")
    ));
    
    // Knee: 0 to 24 dB
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(compKnee, 1),
        "Comp Knee",
        juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f),
        defaultCompKnee,
        juce::AudioParameterFloatAttributes().withLabel("dB")
    ));
    
    // Makeup Gain: 0 to 24 dB
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(compMakeup, 1),
        "Comp Makeup",
        juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")
    ));
    
    // Mix (Parallel): 0-100%
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(compMix, 1),
        "Comp Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        100.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")
    ));
    
    // Enable
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(compEnable, 1),
        "Comp Enable",
        false
    ));
}

void ParameterLayout::addGateParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout) {
    using namespace ParamIDs;
    using namespace Constants;
    
    // Threshold: -80 to 0 dB
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(gateThreshold, 1),
        "Gate Threshold",
        juce::NormalisableRange<float>(-80.0f, 0.0f, 0.1f),
        defaultGateThreshold,
        juce::AudioParameterFloatAttributes().withLabel("dB")
    ));
    
    // Ratio: 1:1 to 20:1
    juce::NormalisableRange<float> ratioRange(1.0f, 20.0f, 0.1f);
    ratioRange.setSkewForCentre(4.0f);
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(gateRatio, 1),
        "Gate Ratio",
        ratioRange,
        defaultGateRatio,
        juce::AudioParameterFloatAttributes().withLabel(":1")
    ));
    
    // Attack: 0.01 to 50 ms
    juce::NormalisableRange<float> attackRange(0.01f, 50.0f, 0.01f);
    attackRange.setSkewForCentre(5.0f);
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(gateAttack, 1),
        "Gate Attack",
        attackRange,
        defaultGateAttack,
        juce::AudioParameterFloatAttributes().withLabel("ms")
    ));
    
    // Hold: 0 to 500 ms
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(gateHold, 1),
        "Gate Hold",
        juce::NormalisableRange<float>(0.0f, 500.0f, 1.0f),
        defaultGateHold,
        juce::AudioParameterFloatAttributes().withLabel("ms")
    ));
    
    // Release: 10 to 2000 ms
    juce::NormalisableRange<float> releaseRange(10.0f, 2000.0f, 1.0f);
    releaseRange.setSkewForCentre(150.0f);
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(gateRelease, 1),
        "Gate Release",
        releaseRange,
        defaultGateRelease,
        juce::AudioParameterFloatAttributes().withLabel("ms")
    ));
    
    // Range: -80 to 0 dB
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(gateRange, 1),
        "Gate Range",
        juce::NormalisableRange<float>(-80.0f, 0.0f, 0.1f),
        defaultGateRange,
        juce::AudioParameterFloatAttributes().withLabel("dB")
    ));
    
    // Enable
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(gateEnable, 1),
        "Gate Enable",
        false
    ));
}

void ParameterLayout::addLimiterParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout) {
    using namespace ParamIDs;
    using namespace Constants;
    
    // Ceiling: -12 to 0 dB
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(limiterCeiling, 1),
        "Limiter Ceiling",
        juce::NormalisableRange<float>(-12.0f, 0.0f, 0.1f),
        defaultLimiterCeiling,
        juce::AudioParameterFloatAttributes().withLabel("dB")
    ));
    
    // Release: 10 to 1000 ms
    juce::NormalisableRange<float> releaseRange(10.0f, 1000.0f, 1.0f);
    releaseRange.setSkewForCentre(100.0f);
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(limiterRelease, 1),
        "Limiter Release",
        releaseRange,
        defaultLimiterRelease,
        juce::AudioParameterFloatAttributes().withLabel("ms")
    ));
    
    // Enable
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(limiterEnable, 1),
        "Limiter Enable",
        false
    ));
}

void ParameterLayout::addAdvancedDSPParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout) {
    using namespace ParamIDs;
    
    // Processing Mode: Stereo, Mid/Side, Left, Right, Mono
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(processingMode, 1),
        "Processing Mode",
        juce::StringArray { "Stereo", "Mid/Side", "Left Only", "Right Only", "Mono" },
        0  // Default: Stereo
    ));
    
    // Oversampling: Off, 2x, 4x, 8x
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(oversampling, 1),
        "Oversampling",
        juce::StringArray { "Off", "2x", "4x", "8x" },
        0  // Default: Off
    ));
    
    // Sidechain Filter Mode
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(scFilterMode, 1),
        "SC Filter Mode",
        juce::StringArray { "Off", "High Pass", "Low Pass", "Band Pass", "Tilt" },
        0  // Default: Off
    ));
    
    // Sidechain Filter Frequency
    juce::NormalisableRange<float> freqRange(20.0f, 20000.0f);
    freqRange.setSkewForCentre(500.0f);
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(scFilterFreq, 1),
        "SC Filter Freq",
        freqRange,
        100.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")
    ));
    
    // Sidechain Filter Q
    juce::NormalisableRange<float> qRange(0.1f, 10.0f, 0.01f);
    qRange.setSkewForCentre(1.0f);
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(scFilterQ, 1),
        "SC Filter Q",
        qRange,
        1.0f,
        juce::AudioParameterFloatAttributes()
    ));
    
    // Sidechain Listen (monitor the filtered sidechain)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(scFilterListen, 1),
        "SC Listen",
        false
    ));
}

} // namespace SeshEQ
