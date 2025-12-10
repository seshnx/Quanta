#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

namespace SeshEQ {

/**
 * @brief Processing mode for stereo signals
 */
enum class ProcessingMode {
    Stereo = 0,     // Normal L/R stereo processing
    MidSide,        // Mid/Side encoding, process, decode
    LeftOnly,       // Process left channel only
    RightOnly,      // Process right channel only
    Mono            // Sum to mono, process, copy to both
};

/**
 * @brief Mid/Side encoder and decoder
 * 
 * Converts between Left/Right and Mid/Side representations:
 * - Mid = (L + R) / sqrt(2)
 * - Side = (L - R) / sqrt(2)
 * 
 * And back:
 * - L = (Mid + Side) / sqrt(2)
 * - R = (Mid - Side) / sqrt(2)
 */
class MidSideProcessor {
public:
    MidSideProcessor() = default;
    
    /**
     * @brief Set the processing mode
     */
    void setMode(ProcessingMode mode) { currentMode = mode; }
    
    /**
     * @brief Get current processing mode
     */
    ProcessingMode getMode() const { return currentMode; }
    
    /**
     * @brief Encode stereo buffer to Mid/Side (in-place)
     * After encoding: channel 0 = Mid, channel 1 = Side
     */
    void encode(juce::AudioBuffer<float>& buffer) {
        if (buffer.getNumChannels() < 2) return;
        
        const int numSamples = buffer.getNumSamples();
        float* left = buffer.getWritePointer(0);
        float* right = buffer.getWritePointer(1);
        
        for (int i = 0; i < numSamples; ++i) {
            const float l = left[i];
            const float r = right[i];
            
            // Encode to M/S
            left[i] = (l + r) * sqrtHalf;   // Mid
            right[i] = (l - r) * sqrtHalf;  // Side
        }
    }
    
    /**
     * @brief Decode Mid/Side buffer back to stereo (in-place)
     * Before decoding: channel 0 = Mid, channel 1 = Side
     */
    void decode(juce::AudioBuffer<float>& buffer) {
        if (buffer.getNumChannels() < 2) return;
        
        const int numSamples = buffer.getNumSamples();
        float* mid = buffer.getWritePointer(0);
        float* side = buffer.getWritePointer(1);
        
        for (int i = 0; i < numSamples; ++i) {
            const float m = mid[i];
            const float s = side[i];
            
            // Decode to L/R
            mid[i] = (m + s) * sqrtHalf;    // Left
            side[i] = (m - s) * sqrtHalf;   // Right
        }
    }
    
    /**
     * @brief Encode single sample pair to Mid/Side
     */
    static void encodeSample(float& left, float& right) {
        const float l = left;
        const float r = right;
        left = (l + r) * sqrtHalf;
        right = (l - r) * sqrtHalf;
    }
    
    /**
     * @brief Decode single sample pair from Mid/Side
     */
    static void decodeSample(float& mid, float& side) {
        const float m = mid;
        const float s = side;
        mid = (m + s) * sqrtHalf;
        side = (m - s) * sqrtHalf;
    }
    
    /**
     * @brief Prepare buffer based on processing mode (before processing)
     * Returns true if the buffer was modified
     */
    bool prepareBuffer(juce::AudioBuffer<float>& buffer) {
        if (buffer.getNumChannels() < 2) return false;
        
        switch (currentMode) {
            case ProcessingMode::Stereo:
                // No preparation needed
                return false;
                
            case ProcessingMode::MidSide:
                encode(buffer);
                return true;
                
            case ProcessingMode::Mono: {
                // Sum to mono
                const int numSamples = buffer.getNumSamples();
                float* left = buffer.getWritePointer(0);
                float* right = buffer.getWritePointer(1);
                
                for (int i = 0; i < numSamples; ++i) {
                    const float mono = (left[i] + right[i]) * 0.5f;
                    left[i] = mono;
                    right[i] = mono;
                }
                return true;
            }
                
            case ProcessingMode::LeftOnly:
            case ProcessingMode::RightOnly:
                // No preparation needed, handled in finalize
                return false;
        }
        
        return false;
    }
    
    /**
     * @brief Finalize buffer based on processing mode (after processing)
     */
    void finalizeBuffer(juce::AudioBuffer<float>& buffer) {
        if (buffer.getNumChannels() < 2) return;
        
        switch (currentMode) {
            case ProcessingMode::Stereo:
            case ProcessingMode::Mono:
                // No finalization needed
                break;
                
            case ProcessingMode::MidSide:
                decode(buffer);
                break;
                
            case ProcessingMode::LeftOnly: {
                // Copy left to right
                buffer.copyFrom(1, 0, buffer, 0, 0, buffer.getNumSamples());
                break;
            }
                
            case ProcessingMode::RightOnly: {
                // Copy right to left
                buffer.copyFrom(0, 0, buffer, 1, 0, buffer.getNumSamples());
                break;
            }
        }
    }
    
private:
    ProcessingMode currentMode = ProcessingMode::Stereo;
    static constexpr float sqrtHalf = 0.7071067811865476f;  // 1/sqrt(2)
};

/**
 * @brief Get names for processing modes (for UI)
 */
inline juce::StringArray getProcessingModeNames() {
    return { "Stereo", "Mid/Side", "Left Only", "Right Only", "Mono" };
}

} // namespace SeshEQ
