#include "FactoryPresets.h"

namespace SeshEQ {
namespace FactoryPresets {

juce::String createPresetXml(
    const juce::String& name,
    const juce::String& category,
    const juce::String& author,
    const juce::String& description,
    float inputGain,
    float outputGain,
    float dryWet,
    bool band1Enable, int band1Type, float band1Freq, float band1Gain, float band1Q,
    bool band2Enable, int band2Type, float band2Freq, float band2Gain, float band2Q,
    bool band3Enable, int band3Type, float band3Freq, float band3Gain, float band3Q,
    bool band4Enable, int band4Type, float band4Freq, float band4Gain, float band4Q,
    bool band5Enable, int band5Type, float band5Freq, float band5Gain, float band5Q,
    bool band6Enable, int band6Type, float band6Freq, float band6Gain, float band6Q,
    bool band7Enable, int band7Type, float band7Freq, float band7Gain, float band7Q,
    bool band8Enable, int band8Type, float band8Freq, float band8Gain, float band8Q,
    bool compEnable, float compThresh, float compRatio,
    float compAttack, float compRelease, float compKnee,
    float compMakeup, float compMix,
    bool gateEnable, float gateThresh, float gateRatio,
    float gateAttack, float gateHold, float gateRelease, float gateRange,
    bool limEnable, float limCeiling, float limRelease,
    int processingMode, int oversampling,
    int scFilterMode, float scFilterFreq, float scFilterQ, bool scListen)
{
    juce::XmlElement root("SeshEQState");
    
    // Metadata
    root.setAttribute("presetName", name);
    root.setAttribute("presetCategory", category);
    root.setAttribute("presetAuthor", author);
    root.setAttribute("presetDescription", description);
    root.setAttribute("presetVersion", "1.0");
    
    // Global parameters
    root.setAttribute("inputGain", inputGain);
    root.setAttribute("outputGain", outputGain);
    root.setAttribute("dryWet", dryWet);
    
    // EQ Band 1
    root.setAttribute("band1Enable", band1Enable ? 1 : 0);
    root.setAttribute("band1Type", band1Type);
    root.setAttribute("band1Freq", band1Freq);
    root.setAttribute("band1Gain", band1Gain);
    root.setAttribute("band1Q", band1Q);
    
    // EQ Band 2
    root.setAttribute("band2Enable", band2Enable ? 1 : 0);
    root.setAttribute("band2Type", band2Type);
    root.setAttribute("band2Freq", band2Freq);
    root.setAttribute("band2Gain", band2Gain);
    root.setAttribute("band2Q", band2Q);
    
    // EQ Band 3
    root.setAttribute("band3Enable", band3Enable ? 1 : 0);
    root.setAttribute("band3Type", band3Type);
    root.setAttribute("band3Freq", band3Freq);
    root.setAttribute("band3Gain", band3Gain);
    root.setAttribute("band3Q", band3Q);
    
    // EQ Band 4
    root.setAttribute("band4Enable", band4Enable ? 1 : 0);
    root.setAttribute("band4Type", band4Type);
    root.setAttribute("band4Freq", band4Freq);
    root.setAttribute("band4Gain", band4Gain);
    root.setAttribute("band4Q", band4Q);
    
    // EQ Band 5
    root.setAttribute("band5Enable", band5Enable ? 1 : 0);
    root.setAttribute("band5Type", band5Type);
    root.setAttribute("band5Freq", band5Freq);
    root.setAttribute("band5Gain", band5Gain);
    root.setAttribute("band5Q", band5Q);
    
    // EQ Band 6
    root.setAttribute("band6Enable", band6Enable ? 1 : 0);
    root.setAttribute("band6Type", band6Type);
    root.setAttribute("band6Freq", band6Freq);
    root.setAttribute("band6Gain", band6Gain);
    root.setAttribute("band6Q", band6Q);
    
    // EQ Band 7
    root.setAttribute("band7Enable", band7Enable ? 1 : 0);
    root.setAttribute("band7Type", band7Type);
    root.setAttribute("band7Freq", band7Freq);
    root.setAttribute("band7Gain", band7Gain);
    root.setAttribute("band7Q", band7Q);
    
    // EQ Band 8
    root.setAttribute("band8Enable", band8Enable ? 1 : 0);
    root.setAttribute("band8Type", band8Type);
    root.setAttribute("band8Freq", band8Freq);
    root.setAttribute("band8Gain", band8Gain);
    root.setAttribute("band8Q", band8Q);
    
    // Compressor
    root.setAttribute("compEnable", compEnable ? 1 : 0);
    root.setAttribute("compThresh", compThresh);
    root.setAttribute("compRatio", compRatio);
    root.setAttribute("compAttack", compAttack);
    root.setAttribute("compRelease", compRelease);
    root.setAttribute("compKnee", compKnee);
    root.setAttribute("compMakeup", compMakeup);
    root.setAttribute("compMix", compMix);
    
    // Gate
    root.setAttribute("gateEnable", gateEnable ? 1 : 0);
    root.setAttribute("gateThresh", gateThresh);
    root.setAttribute("gateRatio", gateRatio);
    root.setAttribute("gateAttack", gateAttack);
    root.setAttribute("gateHold", gateHold);
    root.setAttribute("gateRelease", gateRelease);
    root.setAttribute("gateRange", gateRange);
    
    // Limiter
    root.setAttribute("limEnable", limEnable ? 1 : 0);
    root.setAttribute("limCeiling", limCeiling);
    root.setAttribute("limRelease", limRelease);
    
    // Advanced
    root.setAttribute("processingMode", processingMode);
    root.setAttribute("oversampling", oversampling);
    root.setAttribute("scFilterMode", scFilterMode);
    root.setAttribute("scFilterFreq", scFilterFreq);
    root.setAttribute("scFilterQ", scFilterQ);
    root.setAttribute("scListen", scListen ? 1 : 0);
    
    return root.toString();
}

static void writePreset(const juce::File& dir, const juce::String& name, const juce::String& xml)
{
    auto file = dir.getChildFile(name + ".sesheq");
    file.replaceWithText(xml);
}

//==============================================================================
// Mixing Presets

namespace Mixing {

void install(const juce::File& dir)
{
    // Clean Mix - Subtle clarity enhancement
    writePreset(dir, "Clean Mix", createPresetXml(
        "Clean Mix",
        "Mixing",
        "SeshEQ",
        "Subtle clarity enhancement for a clean mix",
        0.0f, 0.0f, 100.0f,
        // Band 1: Low shelf at 60Hz, slight cut
        true, 6, 60.0f, -1.5f, 0.71f,
        // Band 2: Reduce mud at 250Hz
        true, 4, 250.0f, -2.0f, 1.5f,
        // Band 3: Add warmth at 400Hz
        true, 4, 400.0f, 0.5f, 1.2f,
        // Band 4: Presence at 2.5kHz
        true, 4, 2500.0f, 1.5f, 1.0f,
        // Band 5: Air at 8kHz
        true, 4, 8000.0f, 1.0f, 0.8f,
        // Band 6-8: Neutral
        true, 4, 4000.0f, 0.0f, 1.0f,
        true, 4, 10000.0f, 0.0f, 1.0f,
        true, 7, 14000.0f, 0.5f, 0.71f
    ));
    
    // Glue Compression - Bus compression
    writePreset(dir, "Glue Compression", createPresetXml(
        "Glue Compression",
        "Mixing",
        "SeshEQ",
        "Gentle bus compression to glue tracks together",
        0.0f, 0.0f, 100.0f,
        // All bands neutral
        true, 6, 80.0f, 0.0f, 0.71f,
        true, 4, 200.0f, 0.0f, 1.0f,
        true, 4, 500.0f, 0.0f, 1.0f,
        true, 4, 1000.0f, 0.0f, 1.0f,
        true, 4, 2000.0f, 0.0f, 1.0f,
        true, 4, 4000.0f, 0.0f, 1.0f,
        true, 4, 8000.0f, 0.0f, 1.0f,
        true, 7, 12000.0f, 0.0f, 0.71f,
        // Compressor: Gentle glue
        true, -18.0f, 2.0f,
        30.0f, 200.0f, 10.0f,
        2.0f, 100.0f
    ));
    
    // Parallel Punch - Parallel compression for drums/bass
    writePreset(dir, "Parallel Punch", createPresetXml(
        "Parallel Punch",
        "Mixing",
        "SeshEQ",
        "Heavy parallel compression for punch and body",
        0.0f, 0.0f, 50.0f, // 50% wet for parallel
        // EQ for compression character
        true, 1, 80.0f, 0.0f, 0.71f, // HP to avoid pumping
        true, 4, 200.0f, 0.0f, 1.0f,
        true, 4, 500.0f, 0.0f, 1.0f,
        true, 4, 1000.0f, 0.0f, 1.0f,
        true, 4, 2500.0f, 2.0f, 1.0f, // Presence boost
        true, 4, 4000.0f, 0.0f, 1.0f,
        true, 4, 8000.0f, 0.0f, 1.0f,
        true, 7, 12000.0f, 0.0f, 0.71f,
        // Heavy compression
        true, -30.0f, 8.0f,
        5.0f, 100.0f, 3.0f,
        6.0f, 50.0f, // 50% mix
        // No gate
        false, -60.0f, 10.0f, 0.5f, 50.0f, 100.0f, -80.0f,
        // No limiter
        false, -0.3f, 100.0f,
        // Sidechain HP filter to prevent pumping
        0, 0, 1, 100.0f, 0.71f, false
    ));
}

} // namespace Mixing

//==============================================================================
// Mastering Presets

namespace Mastering {

void install(const juce::File& dir)
{
    // Mastering EQ - Subtle mastering curve
    writePreset(dir, "Mastering EQ", createPresetXml(
        "Mastering EQ",
        "Mastering",
        "SeshEQ",
        "Subtle mastering curve with gentle limiting",
        0.0f, 0.0f, 100.0f,
        // Low end control
        true, 1, 30.0f, 0.0f, 0.71f, // Subsonic filter
        true, 6, 60.0f, 0.5f, 0.71f, // Slight low shelf boost
        true, 4, 200.0f, -0.5f, 2.0f, // Tame low-mids
        true, 4, 800.0f, 0.0f, 1.0f,
        true, 4, 3000.0f, 0.5f, 1.5f, // Presence
        true, 4, 6000.0f, 0.0f, 1.0f,
        true, 7, 12000.0f, 1.0f, 0.71f, // Air
        true, 0, 18000.0f, 0.0f, 0.71f, // Gentle LP
        // Light compression
        true, -12.0f, 1.5f,
        50.0f, 300.0f, 12.0f,
        0.5f, 100.0f,
        // No gate
        false, -60.0f, 10.0f, 0.5f, 50.0f, 100.0f, -80.0f,
        // Limiter
        true, -0.3f, 150.0f,
        // Mid-Side, 2x oversampling
        1, 1, 0, 100.0f, 0.71f, false
    ));
    
    // Loudness Maximizer
    writePreset(dir, "Loudness Maximizer", createPresetXml(
        "Loudness Maximizer",
        "Mastering",
        "SeshEQ",
        "Aggressive loudness processing for competitive levels",
        2.0f, 0.0f, 100.0f,
        // High-end enhancement
        true, 1, 30.0f, 0.0f, 0.71f,
        true, 6, 80.0f, 1.0f, 0.71f,
        true, 4, 250.0f, -1.0f, 1.5f,
        true, 4, 800.0f, 0.0f, 1.0f,
        true, 4, 2500.0f, 1.5f, 1.0f,
        true, 4, 5000.0f, 1.0f, 1.0f,
        true, 7, 10000.0f, 2.0f, 0.71f,
        true, 0, 16000.0f, 0.0f, 0.71f,
        // Heavier compression
        true, -6.0f, 3.0f,
        20.0f, 150.0f, 6.0f,
        3.0f, 100.0f,
        // No gate
        false, -60.0f, 10.0f, 0.5f, 50.0f, 100.0f, -80.0f,
        // Brick wall limiting
        true, -0.1f, 50.0f,
        // 4x oversampling for limiter quality
        0, 2, 0, 100.0f, 0.71f, false
    ));
    
    // Transparent Master
    writePreset(dir, "Transparent Master", createPresetXml(
        "Transparent Master",
        "Mastering",
        "SeshEQ",
        "Ultra-transparent mastering with minimal coloration",
        0.0f, 0.0f, 100.0f,
        true, 1, 25.0f, 0.0f, 0.71f,
        true, 4, 100.0f, 0.0f, 1.0f,
        true, 4, 300.0f, 0.0f, 1.0f,
        true, 4, 1000.0f, 0.0f, 1.0f,
        true, 4, 3000.0f, 0.0f, 1.0f,
        true, 4, 6000.0f, 0.0f, 1.0f,
        true, 4, 10000.0f, 0.0f, 1.0f,
        true, 0, 20000.0f, 0.0f, 0.71f,
        // Very gentle compression
        true, -6.0f, 1.2f,
        100.0f, 500.0f, 12.0f,
        0.0f, 100.0f,
        false, -60.0f, 10.0f, 0.5f, 50.0f, 100.0f, -80.0f,
        // Light limiting
        true, -0.5f, 200.0f,
        1, 2, 0, 100.0f, 0.71f, false
    ));
}

} // namespace Mastering

//==============================================================================
// Vocal Presets

namespace Vocals {

void install(const juce::File& dir)
{
    // Vocal Clarity
    writePreset(dir, "Vocal Clarity", createPresetXml(
        "Vocal Clarity",
        "Vocals",
        "SeshEQ",
        "Clear and present vocal sound",
        0.0f, 0.0f, 100.0f,
        // HP to remove rumble
        true, 1, 80.0f, 0.0f, 0.71f,
        // Reduce mud
        true, 4, 250.0f, -2.5f, 1.5f,
        // Body
        true, 4, 400.0f, 1.0f, 1.2f,
        // Nasal cut
        true, 4, 800.0f, -1.0f, 2.0f,
        // Presence
        true, 4, 3000.0f, 2.5f, 1.0f,
        // Sibilance control
        true, 4, 6000.0f, -0.5f, 2.0f,
        // Air
        true, 7, 12000.0f, 2.0f, 0.71f,
        true, 0, 16000.0f, 0.0f, 0.71f,
        // Light compression
        true, -18.0f, 3.0f,
        10.0f, 80.0f, 6.0f,
        2.0f, 100.0f
    ));
    
    // Vocal Compression
    writePreset(dir, "Vocal Compression", createPresetXml(
        "Vocal Compression",
        "Vocals",
        "SeshEQ",
        "Smooth vocal compression for consistency",
        0.0f, 0.0f, 100.0f,
        true, 1, 100.0f, 0.0f, 0.71f,
        true, 4, 200.0f, 0.0f, 1.0f,
        true, 4, 500.0f, 0.0f, 1.0f,
        true, 4, 1000.0f, 0.0f, 1.0f,
        true, 4, 2000.0f, 0.0f, 1.0f,
        true, 4, 4000.0f, 0.0f, 1.0f,
        true, 4, 8000.0f, 0.0f, 1.0f,
        true, 7, 12000.0f, 0.0f, 0.71f,
        // Vocal compression
        true, -20.0f, 4.0f,
        5.0f, 60.0f, 6.0f,
        4.0f, 100.0f
    ));
    
    // De-Esser Setup
    writePreset(dir, "De-Esser Setup", createPresetXml(
        "De-Esser Setup",
        "Vocals",
        "SeshEQ",
        "Reduce harsh sibilance with sidechain filtering",
        0.0f, 0.0f, 100.0f,
        true, 1, 80.0f, 0.0f, 0.71f,
        true, 4, 200.0f, 0.0f, 1.0f,
        true, 4, 500.0f, 0.0f, 1.0f,
        true, 4, 1000.0f, 0.0f, 1.0f,
        true, 4, 2000.0f, 0.0f, 1.0f,
        // Cut at sibilance frequencies
        true, 4, 6000.0f, 0.0f, 1.0f,
        true, 4, 8000.0f, 0.0f, 1.0f,
        true, 7, 12000.0f, 0.0f, 0.71f,
        // Fast compression
        true, -20.0f, 6.0f,
        0.5f, 30.0f, 3.0f,
        0.0f, 100.0f,
        false, -60.0f, 10.0f, 0.5f, 50.0f, 100.0f, -80.0f,
        false, -0.3f, 100.0f,
        // Sidechain BP filter focused on sibilance
        0, 0, 3, 6000.0f, 2.0f, false
    ));
    
    // Radio Voice
    writePreset(dir, "Radio Voice", createPresetXml(
        "Radio Voice",
        "Vocals",
        "SeshEQ",
        "Broadcast-style voice processing",
        0.0f, 0.0f, 100.0f,
        // HP for proximity control
        true, 1, 120.0f, 0.0f, 0.71f,
        // Low-mid boost for warmth
        true, 6, 200.0f, 3.0f, 0.71f,
        true, 4, 400.0f, 0.0f, 1.0f,
        // Presence boost
        true, 4, 2000.0f, 2.0f, 1.0f,
        true, 4, 3500.0f, 3.0f, 1.2f,
        true, 4, 5000.0f, 1.0f, 1.0f,
        // Air
        true, 7, 10000.0f, 2.0f, 0.71f,
        true, 0, 14000.0f, 0.0f, 0.71f,
        // Heavy compression
        true, -24.0f, 6.0f,
        3.0f, 50.0f, 3.0f,
        6.0f, 100.0f,
        // Gate for clean pauses
        true, -45.0f, 10.0f, 1.0f, 100.0f, 80.0f, -40.0f,
        // Limiter for consistency
        true, -1.0f, 50.0f
    ));
}

} // namespace Vocals

//==============================================================================
// Drum Presets

namespace Drums {

void install(const juce::File& dir)
{
    // Punchy Drums
    writePreset(dir, "Punchy Drums", createPresetXml(
        "Punchy Drums",
        "Drums",
        "SeshEQ",
        "Add punch and clarity to drum bus",
        0.0f, 0.0f, 100.0f,
        // Sub boost
        true, 6, 60.0f, 2.0f, 0.71f,
        // Tight lows
        true, 4, 100.0f, 1.0f, 2.0f,
        // Reduce boxiness
        true, 4, 400.0f, -2.0f, 2.0f,
        // Attack
        true, 4, 2000.0f, 2.0f, 1.0f,
        true, 4, 4000.0f, 1.5f, 1.0f,
        // Snap
        true, 4, 6000.0f, 1.0f, 1.0f,
        // Air
        true, 7, 10000.0f, 2.0f, 0.71f,
        true, 0, 16000.0f, 0.0f, 0.71f,
        // Punchy compression
        true, -15.0f, 4.0f,
        10.0f, 100.0f, 6.0f,
        3.0f, 100.0f
    ));
    
    // Kick Enhancement
    writePreset(dir, "Kick Enhancement", createPresetXml(
        "Kick Enhancement",
        "Drums",
        "SeshEQ",
        "Full and punchy kick drum",
        0.0f, 0.0f, 100.0f,
        // Sub
        true, 6, 50.0f, 3.0f, 0.71f,
        // Fundamental
        true, 4, 80.0f, 2.0f, 2.0f,
        // Reduce mud
        true, 4, 300.0f, -3.0f, 2.0f,
        // Beater attack
        true, 4, 2500.0f, 2.5f, 1.5f,
        true, 4, 4000.0f, 1.0f, 1.0f,
        true, 4, 6000.0f, 0.0f, 1.0f,
        true, 7, 10000.0f, 0.0f, 0.71f,
        true, 0, 12000.0f, 0.0f, 0.71f,
        // Fast attack compression
        true, -12.0f, 4.0f,
        5.0f, 80.0f, 3.0f,
        2.0f, 100.0f,
        // Gate for tightness
        true, -40.0f, 10.0f, 0.3f, 30.0f, 50.0f, -60.0f
    ));
    
    // Snare Crack
    writePreset(dir, "Snare Crack", createPresetXml(
        "Snare Crack",
        "Drums",
        "SeshEQ",
        "Crisp and cutting snare sound",
        0.0f, 0.0f, 100.0f,
        // HP to remove kick bleed
        true, 1, 80.0f, 0.0f, 0.71f,
        // Body
        true, 4, 200.0f, 2.0f, 1.5f,
        // Reduce boxiness
        true, 4, 400.0f, -2.0f, 2.0f,
        // Crack
        true, 4, 900.0f, 1.5f, 1.5f,
        // Attack
        true, 4, 2500.0f, 2.0f, 1.0f,
        // Snap
        true, 4, 5000.0f, 3.0f, 1.0f,
        // Air
        true, 7, 8000.0f, 2.0f, 0.71f,
        true, 0, 14000.0f, 0.0f, 0.71f,
        // Fast compression
        true, -18.0f, 4.0f,
        3.0f, 60.0f, 6.0f,
        3.0f, 100.0f,
        // Gate
        true, -35.0f, 10.0f, 0.5f, 40.0f, 60.0f, -50.0f
    ));
    
    // Room Drums
    writePreset(dir, "Room Drums", createPresetXml(
        "Room Drums",
        "Drums",
        "SeshEQ",
        "Big room drum sound with heavy compression",
        0.0f, 0.0f, 100.0f,
        // LP to reduce harshness
        true, 0, 10000.0f, 0.0f, 0.71f,
        true, 6, 100.0f, 3.0f, 0.71f,
        true, 4, 400.0f, 2.0f, 1.0f,
        true, 4, 800.0f, 0.0f, 1.0f,
        true, 4, 2000.0f, 1.0f, 1.0f,
        true, 4, 4000.0f, 0.0f, 1.0f,
        true, 4, 6000.0f, -2.0f, 1.0f,
        true, 7, 8000.0f, 0.0f, 0.71f,
        // Heavy room compression
        true, -30.0f, 10.0f,
        1.0f, 200.0f, 0.0f,
        10.0f, 100.0f
    ));
}

} // namespace Drums

//==============================================================================
// Bass Presets

namespace Bass {

void install(const juce::File& dir)
{
    // Full Bass
    writePreset(dir, "Full Bass", createPresetXml(
        "Full Bass",
        "Bass",
        "SeshEQ",
        "Full and present bass sound",
        0.0f, 0.0f, 100.0f,
        // Sub control
        true, 1, 30.0f, 0.0f, 0.71f,
        // Fundamental boost
        true, 6, 80.0f, 2.0f, 0.71f,
        // Reduce mud
        true, 4, 250.0f, -2.0f, 1.5f,
        // Growl
        true, 4, 700.0f, 1.5f, 1.5f,
        // Definition
        true, 4, 1500.0f, 2.0f, 1.0f,
        // Clank
        true, 4, 3000.0f, 1.0f, 1.0f,
        // LP to prevent harshness
        true, 0, 8000.0f, 0.0f, 0.71f,
        true, 4, 5000.0f, 0.0f, 1.0f,
        // Compression
        true, -15.0f, 4.0f,
        10.0f, 100.0f, 6.0f,
        2.0f, 100.0f
    ));
    
    // Sub Bass
    writePreset(dir, "Sub Bass", createPresetXml(
        "Sub Bass",
        "Bass",
        "SeshEQ",
        "Deep sub-focused bass",
        0.0f, 0.0f, 100.0f,
        // HP below useful range
        true, 1, 25.0f, 0.0f, 0.71f,
        // Sub boost
        true, 6, 50.0f, 4.0f, 0.71f,
        // Fundamental
        true, 4, 80.0f, 1.0f, 1.5f,
        // Cut upper content
        true, 0, 500.0f, 0.0f, 0.71f,
        true, 4, 200.0f, -2.0f, 1.0f,
        true, 4, 300.0f, -3.0f, 1.0f,
        true, 4, 400.0f, -4.0f, 1.0f,
        true, 4, 600.0f, 0.0f, 1.0f,
        // Heavy compression for consistency
        true, -12.0f, 6.0f,
        5.0f, 80.0f, 3.0f,
        4.0f, 100.0f
    ));
    
    // DI Bass Warmth
    writePreset(dir, "DI Bass Warmth", createPresetXml(
        "DI Bass Warmth",
        "Bass",
        "SeshEQ",
        "Add warmth and character to DI bass",
        0.0f, 0.0f, 100.0f,
        true, 1, 40.0f, 0.0f, 0.71f,
        // Low shelf for warmth
        true, 6, 100.0f, 3.0f, 0.71f,
        // Mid character
        true, 4, 300.0f, 1.0f, 1.5f,
        true, 4, 600.0f, 2.0f, 1.2f,
        // Attack
        true, 4, 1000.0f, 1.5f, 1.0f,
        true, 4, 2000.0f, 1.0f, 1.0f,
        // Gentle high shelf rolloff
        true, 7, 4000.0f, -2.0f, 0.71f,
        true, 0, 10000.0f, 0.0f, 0.71f,
        // Moderate compression
        true, -16.0f, 3.5f,
        15.0f, 120.0f, 6.0f,
        2.0f, 100.0f
    ));
}

} // namespace Bass

//==============================================================================
// Guitar Presets

namespace Guitar {

void install(const juce::File& dir)
{
    // Acoustic Clarity
    writePreset(dir, "Acoustic Clarity", createPresetXml(
        "Acoustic Clarity",
        "Guitar",
        "SeshEQ",
        "Clear and present acoustic guitar",
        0.0f, 0.0f, 100.0f,
        // HP for mix space
        true, 1, 100.0f, 0.0f, 0.71f,
        // Body
        true, 4, 200.0f, 1.5f, 1.0f,
        // Reduce boom
        true, 4, 350.0f, -2.0f, 2.0f,
        // Warmth
        true, 4, 600.0f, 0.5f, 1.0f,
        // Presence
        true, 4, 2500.0f, 2.0f, 1.0f,
        // Sparkle
        true, 4, 5000.0f, 2.5f, 1.0f,
        // Air
        true, 7, 10000.0f, 3.0f, 0.71f,
        true, 0, 16000.0f, 0.0f, 0.71f,
        // Light compression
        true, -18.0f, 2.5f,
        15.0f, 150.0f, 6.0f,
        1.0f, 100.0f
    ));
    
    // Electric Crunch
    writePreset(dir, "Electric Crunch", createPresetXml(
        "Electric Crunch",
        "Guitar",
        "SeshEQ",
        "Tight and cutting electric guitar tone",
        0.0f, 0.0f, 100.0f,
        // HP to leave room for bass
        true, 1, 80.0f, 0.0f, 0.71f,
        // Low-mid presence
        true, 4, 150.0f, 1.0f, 1.5f,
        // Remove mud
        true, 4, 350.0f, -3.0f, 2.0f,
        // Crunch
        true, 4, 800.0f, 2.0f, 1.5f,
        // Bite
        true, 4, 2000.0f, 2.5f, 1.0f,
        // Definition
        true, 4, 4000.0f, 1.5f, 1.0f,
        // LP to reduce harshness
        true, 0, 10000.0f, 0.0f, 0.71f,
        true, 7, 8000.0f, 0.0f, 0.71f,
        // Mild compression
        true, -14.0f, 3.0f,
        8.0f, 100.0f, 6.0f,
        1.5f, 100.0f
    ));
    
    // Clean Electric
    writePreset(dir, "Clean Electric", createPresetXml(
        "Clean Electric",
        "Guitar",
        "SeshEQ",
        "Sparkly clean electric guitar",
        0.0f, 0.0f, 100.0f,
        true, 1, 80.0f, 0.0f, 0.71f,
        // Reduce low-mids
        true, 4, 200.0f, -1.0f, 1.5f,
        // Presence dip
        true, 4, 400.0f, -1.5f, 2.0f,
        // Chime
        true, 4, 800.0f, 1.0f, 1.0f,
        // Sparkle
        true, 4, 2500.0f, 2.0f, 1.0f,
        true, 4, 4000.0f, 2.5f, 1.0f,
        // Shimmer
        true, 7, 8000.0f, 3.0f, 0.71f,
        true, 0, 14000.0f, 0.0f, 0.71f,
        // Light compression
        true, -20.0f, 2.0f,
        20.0f, 200.0f, 6.0f,
        0.5f, 100.0f
    ));
}

} // namespace Guitar

//==============================================================================
// Creative Presets

namespace Creative {

void install(const juce::File& dir)
{
    // Telephone Effect
    writePreset(dir, "Telephone Effect", createPresetXml(
        "Telephone Effect",
        "Creative",
        "SeshEQ",
        "Classic telephone/radio effect",
        0.0f, 0.0f, 100.0f,
        // Bandpass
        true, 1, 300.0f, 0.0f, 0.71f, // HP
        true, 0, 3000.0f, 0.0f, 0.71f, // LP
        // Midrange boost for voice
        true, 4, 1000.0f, 6.0f, 1.5f,
        true, 4, 2000.0f, 4.0f, 1.5f,
        true, 4, 500.0f, 2.0f, 1.0f,
        true, 4, 1500.0f, 0.0f, 1.0f,
        true, 4, 2500.0f, 0.0f, 1.0f,
        true, 4, 4000.0f, 0.0f, 1.0f,
        // Heavy compression for that effect
        true, -30.0f, 10.0f,
        1.0f, 30.0f, 0.0f,
        10.0f, 100.0f
    ));
    
    // Lo-Fi
    writePreset(dir, "Lo-Fi", createPresetXml(
        "Lo-Fi",
        "Creative",
        "SeshEQ",
        "Vintage lo-fi character",
        0.0f, 0.0f, 100.0f,
        // Reduce bass
        true, 1, 60.0f, 0.0f, 0.71f,
        true, 6, 100.0f, -3.0f, 0.71f,
        // Boost mids
        true, 4, 400.0f, 3.0f, 0.8f,
        true, 4, 800.0f, 4.0f, 0.8f,
        true, 4, 1500.0f, 2.0f, 1.0f,
        // Roll off highs
        true, 0, 6000.0f, 0.0f, 0.71f,
        true, 7, 4000.0f, -4.0f, 0.71f,
        true, 4, 3000.0f, 0.0f, 1.0f,
        // Crushed compression
        true, -25.0f, 8.0f,
        5.0f, 50.0f, 0.0f,
        8.0f, 100.0f
    ));
    
    // Wide Stereo
    writePreset(dir, "Wide Stereo", createPresetXml(
        "Wide Stereo",
        "Creative",
        "SeshEQ",
        "Enhanced stereo width using Mid/Side",
        0.0f, 0.0f, 100.0f,
        // Neutral EQ
        true, 1, 30.0f, 0.0f, 0.71f,
        true, 4, 200.0f, 0.0f, 1.0f,
        true, 4, 500.0f, 0.0f, 1.0f,
        true, 4, 1000.0f, 0.0f, 1.0f,
        true, 4, 2000.0f, 0.0f, 1.0f,
        true, 4, 4000.0f, 0.0f, 1.0f,
        true, 4, 8000.0f, 0.0f, 1.0f,
        true, 7, 12000.0f, 0.0f, 0.71f,
        // No compression
        false, -12.0f, 4.0f, 10.0f, 100.0f, 6.0f, 0.0f, 100.0f,
        false, -60.0f, 10.0f, 0.5f, 50.0f, 100.0f, -80.0f,
        false, -0.3f, 100.0f,
        // Mid-Side processing
        1, 0, 0, 100.0f, 0.71f, false
    ));
    
    // Aggressive
    writePreset(dir, "Aggressive", createPresetXml(
        "Aggressive",
        "Creative",
        "SeshEQ",
        "Hard-hitting aggressive sound",
        2.0f, 0.0f, 100.0f,
        // Big lows
        true, 6, 60.0f, 4.0f, 0.71f,
        // Tight low-mids
        true, 4, 200.0f, -2.0f, 2.0f,
        // Aggression frequencies
        true, 4, 500.0f, 3.0f, 1.0f,
        true, 4, 1000.0f, 2.0f, 1.0f,
        true, 4, 2500.0f, 4.0f, 1.0f,
        true, 4, 5000.0f, 2.0f, 1.0f,
        // Bright top
        true, 7, 10000.0f, 3.0f, 0.71f,
        true, 0, 16000.0f, 0.0f, 0.71f,
        // Heavy compression
        true, -18.0f, 8.0f,
        2.0f, 50.0f, 0.0f,
        6.0f, 100.0f,
        false, -60.0f, 10.0f, 0.5f, 50.0f, 100.0f, -80.0f,
        // Hard limiting
        true, -0.1f, 30.0f
    ));
}

} // namespace Creative

//==============================================================================
// Utility Presets

namespace Utility {

void install(const juce::File& dir)
{
    // Default/Init
    writePreset(dir, "Default", createPresetXml(
        "Default",
        "Utility",
        "SeshEQ",
        "Clean slate with all parameters at default",
        0.0f, 0.0f, 100.0f
        // All defaults
    ));
    
    // Subsonic Filter
    writePreset(dir, "Subsonic Filter", createPresetXml(
        "Subsonic Filter",
        "Utility",
        "SeshEQ",
        "Remove unwanted subsonic frequencies",
        0.0f, 0.0f, 100.0f,
        // Steep HP
        true, 1, 30.0f, 0.0f, 0.71f,
        true, 1, 25.0f, 0.0f, 0.71f, // Stack for steeper slope
        true, 4, 100.0f, 0.0f, 1.0f,
        true, 4, 500.0f, 0.0f, 1.0f,
        true, 4, 1000.0f, 0.0f, 1.0f,
        true, 4, 2000.0f, 0.0f, 1.0f,
        true, 4, 4000.0f, 0.0f, 1.0f,
        true, 4, 8000.0f, 0.0f, 1.0f
    ));
    
    // Brick Wall Limiter
    writePreset(dir, "Brick Wall Limiter", createPresetXml(
        "Brick Wall Limiter",
        "Utility",
        "SeshEQ",
        "Transparent brick wall limiting only",
        0.0f, 0.0f, 100.0f,
        // All bands flat
        true, 6, 80.0f, 0.0f, 0.71f,
        true, 4, 200.0f, 0.0f, 1.0f,
        true, 4, 500.0f, 0.0f, 1.0f,
        true, 4, 1000.0f, 0.0f, 1.0f,
        true, 4, 2000.0f, 0.0f, 1.0f,
        true, 4, 4000.0f, 0.0f, 1.0f,
        true, 4, 8000.0f, 0.0f, 1.0f,
        true, 7, 12000.0f, 0.0f, 0.71f,
        // No compression
        false, -12.0f, 4.0f, 10.0f, 100.0f, 6.0f, 0.0f, 100.0f,
        // No gate
        false, -60.0f, 10.0f, 0.5f, 50.0f, 100.0f, -80.0f,
        // Limiter only
        true, -0.3f, 100.0f,
        // 4x oversampling for quality
        0, 2, 0, 100.0f, 0.71f, false
    ));
    
    // Mono Check
    writePreset(dir, "Mono Check", createPresetXml(
        "Mono Check",
        "Utility",
        "SeshEQ",
        "Sum to mono for compatibility checking",
        0.0f, 0.0f, 100.0f,
        true, 6, 80.0f, 0.0f, 0.71f,
        true, 4, 200.0f, 0.0f, 1.0f,
        true, 4, 500.0f, 0.0f, 1.0f,
        true, 4, 1000.0f, 0.0f, 1.0f,
        true, 4, 2000.0f, 0.0f, 1.0f,
        true, 4, 4000.0f, 0.0f, 1.0f,
        true, 4, 8000.0f, 0.0f, 1.0f,
        true, 7, 12000.0f, 0.0f, 0.71f,
        false, -12.0f, 4.0f, 10.0f, 100.0f, 6.0f, 0.0f, 100.0f,
        false, -60.0f, 10.0f, 0.5f, 50.0f, 100.0f, -80.0f,
        false, -0.3f, 100.0f,
        // Mono processing mode
        4, 0, 0, 100.0f, 0.71f, false
    ));
    
    // Gain Staging
    writePreset(dir, "Gain Staging", createPresetXml(
        "Gain Staging",
        "Utility",
        "SeshEQ",
        "Clean gain adjustment with metering",
        0.0f, 0.0f, 100.0f
        // All defaults - just use for input/output gain adjustment
    ));
}

} // namespace Utility

//==============================================================================
// Install All

void installAllPresets(const juce::File& factoryDirectory)
{
    factoryDirectory.createDirectory();
    
    Mixing::install(factoryDirectory);
    Mastering::install(factoryDirectory);
    Vocals::install(factoryDirectory);
    Drums::install(factoryDirectory);
    Bass::install(factoryDirectory);
    Guitar::install(factoryDirectory);
    Creative::install(factoryDirectory);
    Utility::install(factoryDirectory);
}

} // namespace FactoryPresets
} // namespace SeshEQ
