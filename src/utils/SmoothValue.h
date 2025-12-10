#pragma once

#include <cmath>
#include <algorithm>

namespace SeshEQ {

/**
 * @brief Simple exponential smoothing for parameter changes
 * 
 * Provides smooth transitions to avoid zipper noise when parameters change.
 * Uses a one-pole lowpass filter for smoothing.
 */
template <typename FloatType>
class SmoothValue {
public:
    SmoothValue() = default;
    
    explicit SmoothValue(FloatType initialValue)
        : currentValue(initialValue), targetValue(initialValue) {}
    
    /**
     * @brief Prepare the smoother with sample rate and ramp time
     * @param sampleRate The audio sample rate
     * @param rampTimeMs The time to reach ~63% of target (time constant)
     */
    void prepare(double sampleRate, double rampTimeMs = 20.0) {
        if (sampleRate > 0.0 && rampTimeMs > 0.0) {
            // Calculate coefficient for exponential smoothing
            // After rampTimeMs, we reach ~63.2% of the target
            const double rampTimeSamples = (rampTimeMs / 1000.0) * sampleRate;
            coefficient = static_cast<FloatType>(1.0 - std::exp(-1.0 / rampTimeSamples));
        } else {
            coefficient = static_cast<FloatType>(1.0); // Instant
        }
    }
    
    /**
     * @brief Set the target value
     */
    void setTargetValue(FloatType newTarget) {
        targetValue = newTarget;
    }
    
    /**
     * @brief Get the current smoothed value and advance
     */
    FloatType getNextValue() {
        currentValue += coefficient * (targetValue - currentValue);
        return currentValue;
    }
    
    /**
     * @brief Get current value without advancing
     */
    FloatType getCurrentValue() const {
        return currentValue;
    }
    
    /**
     * @brief Get target value
     */
    FloatType getTargetValue() const {
        return targetValue;
    }
    
    /**
     * @brief Check if we've essentially reached the target
     */
    bool isSmoothing() const {
        return std::abs(targetValue - currentValue) > static_cast<FloatType>(1e-6);
    }
    
    /**
     * @brief Skip smoothing and jump to target immediately
     */
    void reset(FloatType value) {
        currentValue = value;
        targetValue = value;
    }
    
    /**
     * @brief Skip to current target
     */
    void skipToTarget() {
        currentValue = targetValue;
    }
    
private:
    FloatType currentValue = static_cast<FloatType>(0);
    FloatType targetValue = static_cast<FloatType>(0);
    FloatType coefficient = static_cast<FloatType>(1);
};

/**
 * @brief Smooth value for decibel values (smooths in linear domain)
 */
template <typename FloatType>
class SmoothGain {
public:
    void prepare(double sampleRate, double rampTimeMs = 20.0) {
        linearSmoother.prepare(sampleRate, rampTimeMs);
    }
    
    void setTargetDb(FloatType dB) {
        // Convert dB to linear for smoothing
        FloatType linear = std::pow(static_cast<FloatType>(10), dB / static_cast<FloatType>(20));
        linearSmoother.setTargetValue(linear);
    }
    
    FloatType getNextGain() {
        return linearSmoother.getNextValue();
    }
    
    FloatType getCurrentGain() const {
        return linearSmoother.getCurrentValue();
    }
    
    bool isSmoothing() const {
        return linearSmoother.isSmoothing();
    }
    
    void reset(FloatType linearValue) {
        linearSmoother.reset(linearValue);
    }
    
private:
    SmoothValue<FloatType> linearSmoother { static_cast<FloatType>(1) };
};

} // namespace SeshEQ
