#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace SeshEQ {

/**
 * @brief Mid/Side processing utility
 * 
 * Converts stereo signal to Mid/Side representation:
 * - Mid = (L + R) / 2 (mono component)
 * - Side = (L - R) / 2 (stereo difference)
 * 
 * And converts back:
 * - L = Mid + Side
 * - R = Mid - Side
 */
class MidSideProcessor {
public:
    /**
     * @brief Convert stereo to Mid/Side
     * @param left Left channel data
     * @param right Right channel data
     * @param numSamples Number of samples
     * @param mid Output: Mid channel (mono)
     * @param side Output: Side channel (stereo difference)
     */
    static void encode(float* left, float* right, int numSamples, 
                      float* mid, float* side) {
        for (int i = 0; i < numSamples; ++i) {
            mid[i] = (left[i] + right[i]) * 0.5f;
            side[i] = (left[i] - right[i]) * 0.5f;
        }
    }
    
    /**
     * @brief Convert Mid/Side back to stereo
     * @param mid Mid channel (mono)
     * @param side Side channel (stereo difference)
     * @param numSamples Number of samples
     * @param left Output: Left channel
     * @param right Output: Right channel
     */
    static void decode(float* mid, float* side, int numSamples,
                      float* left, float* right) {
        for (int i = 0; i < numSamples; ++i) {
            left[i] = mid[i] + side[i];
            right[i] = mid[i] - side[i];
        }
    }
    
    /**
     * @brief Process stereo buffer in-place with Mid/Side encoding/decoding
     * @param buffer Stereo audio buffer
     * @param processMid Function to process mid channel: void(float* mid, int numSamples)
     * @param processSide Function to process side channel: void(float* side, int numSamples)
     */
    template<typename ProcessMidFunc, typename ProcessSideFunc>
    static void process(juce::AudioBuffer<float>& buffer,
                       ProcessMidFunc processMid,
                       ProcessSideFunc processSide) {
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();
        
        if (numChannels < 2) return;  // Need stereo for M/S
        
        float* left = buffer.getWritePointer(0);
        float* right = buffer.getWritePointer(1);
        
        // Allocate temporary buffers
        juce::AudioBuffer<float> midBuffer(1, numSamples);
        juce::AudioBuffer<float> sideBuffer(1, numSamples);
        
        float* mid = midBuffer.getWritePointer(0);
        float* side = sideBuffer.getWritePointer(0);
        
        // Encode to M/S
        encode(left, right, numSamples, mid, side);
        
        // Process Mid and Side separately
        processMid(mid, numSamples);
        processSide(side, numSamples);
        
        // Decode back to L/R
        decode(mid, side, numSamples, left, right);
    }
};

} // namespace SeshEQ

