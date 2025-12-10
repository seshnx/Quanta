#include "LevelDetector.h"

namespace SeshEQ {

void LevelDetector::prepare(double newSampleRate) {
    sampleRate = newSampleRate;
    rmsWindowSamples = static_cast<int>((rmsWindowMs / 1000.0) * sampleRate);
    reset();
    updateCoefficients();
}

void LevelDetector::reset() {
    envelope = 0.0f;
    rmsSum = 0.0f;
    rmsSampleCount = 0;
}

void LevelDetector::setAttackTime(float ms) {
    attackMs = ms;
    updateCoefficients();
}

void LevelDetector::setReleaseTime(float ms) {
    releaseMs = ms;
    updateCoefficients();
}

void LevelDetector::setMode(DetectionMode newMode) {
    mode = newMode;
    reset();
}

void LevelDetector::updateCoefficients() {
    // Convert time constants to coefficients
    // Using standard exponential envelope formula
    // coef = exp(-1 / (time_in_seconds * sample_rate))
    
    if (attackMs > 0.0f) {
        const double attackSeconds = attackMs / 1000.0;
        attackCoef = static_cast<float>(std::exp(-1.0 / (attackSeconds * sampleRate)));
    } else {
        attackCoef = 0.0f;  // Instant attack
    }
    
    if (releaseMs > 0.0f) {
        const double releaseSeconds = releaseMs / 1000.0;
        releaseCoef = static_cast<float>(std::exp(-1.0 / (releaseSeconds * sampleRate)));
    } else {
        releaseCoef = 0.0f;  // Instant release
    }
}

float LevelDetector::processSample(float input) {
    float level = 0.0f;
    
    switch (mode) {
        case DetectionMode::Peak:
            // Simple peak detection (absolute value)
            level = std::abs(input);
            break;
            
        case DetectionMode::RMS: {
            // Running RMS approximation
            const float squared = input * input;
            
            // Simple exponential moving average of squared signal
            const float rmsCoef = static_cast<float>(std::exp(-1.0 / rmsWindowSamples));
            rmsSum = rmsCoef * rmsSum + (1.0f - rmsCoef) * squared;
            level = std::sqrt(rmsSum);
            break;
        }
            
        case DetectionMode::TruePeak:
            // Simplified true peak (in reality would use oversampling)
            // For now, use peak detection with some interpolation estimation
            level = std::abs(input);
            // Could add oversampled peak detection here
            break;
    }
    
    // Apply attack/release envelope
    if (level > envelope) {
        // Attack phase
        envelope = attackCoef * envelope + (1.0f - attackCoef) * level;
    } else {
        // Release phase
        envelope = releaseCoef * envelope + (1.0f - releaseCoef) * level;
    }
    
    return envelope;
}

float LevelDetector::processStereo(float left, float right) {
    // Take maximum of both channels
    const float maxInput = std::max(std::abs(left), std::abs(right));
    
    float level = maxInput;
    
    if (mode == DetectionMode::RMS) {
        const float squared = maxInput * maxInput;
        const float rmsCoef = static_cast<float>(std::exp(-1.0 / rmsWindowSamples));
        rmsSum = rmsCoef * rmsSum + (1.0f - rmsCoef) * squared;
        level = std::sqrt(rmsSum);
    }
    
    // Apply attack/release envelope
    if (level > envelope) {
        envelope = attackCoef * envelope + (1.0f - attackCoef) * level;
    } else {
        envelope = releaseCoef * envelope + (1.0f - releaseCoef) * level;
    }
    
    return envelope;
}

float LevelDetector::getCurrentLevelDb() const {
    return dBUtils::linearToDb(envelope);
}

} // namespace SeshEQ
