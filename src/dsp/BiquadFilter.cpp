#include "BiquadFilter.h"
#include <algorithm>
#include <complex>

namespace SeshEQ {

void BiquadFilter::prepare(double newSampleRate) {
    sampleRate = newSampleRate;
    reset();
    updateCoefficients();
}

void BiquadFilter::reset() {
    z1 = 0.0;
    z2 = 0.0;
}

void BiquadFilter::setParameters(FilterType type, float frequency, float q, float gainDb) {
    currentType = type;
    currentFreq = frequency;
    currentQ = q;
    currentGain = gainDb;
    updateCoefficients();
}

void BiquadFilter::setFrequency(float frequency) {
    currentFreq = frequency;
    updateCoefficients();
}

void BiquadFilter::setQ(float q) {
    currentQ = q;
    updateCoefficients();
}

void BiquadFilter::setGain(float gainDb) {
    currentGain = gainDb;
    updateCoefficients();
}

void BiquadFilter::setType(FilterType type) {
    currentType = type;
    updateCoefficients();
}

void BiquadFilter::updateCoefficients() {
    // Clamp frequency to valid range
    const double freq = std::clamp(static_cast<double>(currentFreq), 
                                    10.0, sampleRate * 0.499);
    const double Q = std::max(0.01, static_cast<double>(currentQ));
    
    // Angular frequency
    const double w0 = 2.0 * pi * freq / sampleRate;
    const double cosw0 = std::cos(w0);
    const double sinw0 = std::sin(w0);
    
    // Alpha (bandwidth parameter)
    const double alpha = sinw0 / (2.0 * Q);
    
    // Amplitude for shelf and peak filters
    const double A = std::pow(10.0, static_cast<double>(currentGain) / 40.0);
    
    // Temporary coefficients (will be normalized)
    double a0 = 1.0;
    
    switch (currentType) {
        case FilterType::LowPass:
            // H(s) = 1 / (s^2 + s/Q + 1)
            b0 = (1.0 - cosw0) / 2.0;
            b1 = 1.0 - cosw0;
            b2 = (1.0 - cosw0) / 2.0;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cosw0;
            a2 = 1.0 - alpha;
            break;
            
        case FilterType::HighPass:
            // H(s) = s^2 / (s^2 + s/Q + 1)
            b0 = (1.0 + cosw0) / 2.0;
            b1 = -(1.0 + cosw0);
            b2 = (1.0 + cosw0) / 2.0;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cosw0;
            a2 = 1.0 - alpha;
            break;
            
        case FilterType::BandPass:
            // H(s) = (s/Q) / (s^2 + s/Q + 1) (constant skirt gain, peak gain = Q)
            b0 = alpha;
            b1 = 0.0;
            b2 = -alpha;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cosw0;
            a2 = 1.0 - alpha;
            break;
            
        case FilterType::Notch:
            // H(s) = (s^2 + 1) / (s^2 + s/Q + 1)
            b0 = 1.0;
            b1 = -2.0 * cosw0;
            b2 = 1.0;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cosw0;
            a2 = 1.0 - alpha;
            break;
            
        case FilterType::Peak:
            // H(s) = (s^2 + s*(A/Q) + 1) / (s^2 + s/(A*Q) + 1)
            b0 = 1.0 + alpha * A;
            b1 = -2.0 * cosw0;
            b2 = 1.0 - alpha * A;
            a0 = 1.0 + alpha / A;
            a1 = -2.0 * cosw0;
            a2 = 1.0 - alpha / A;
            break;
            
        case FilterType::LowShelf: {
            // H(s) = A * [ (s^2 + (sqrt(A)/Q)*s + A) / (A*s^2 + (sqrt(A)/Q)*s + 1) ]
            const double sqrtA = std::sqrt(A);
            const double sqrtA_alpha = 2.0 * sqrtA * alpha;
            
            b0 = A * ((A + 1.0) - (A - 1.0) * cosw0 + sqrtA_alpha);
            b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * cosw0);
            b2 = A * ((A + 1.0) - (A - 1.0) * cosw0 - sqrtA_alpha);
            a0 = (A + 1.0) + (A - 1.0) * cosw0 + sqrtA_alpha;
            a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cosw0);
            a2 = (A + 1.0) + (A - 1.0) * cosw0 - sqrtA_alpha;
            break;
        }
            
        case FilterType::HighShelf: {
            // H(s) = A * [ (A*s^2 + (sqrt(A)/Q)*s + 1) / (s^2 + (sqrt(A)/Q)*s + A) ]
            const double sqrtA = std::sqrt(A);
            const double sqrtA_alpha = 2.0 * sqrtA * alpha;
            
            b0 = A * ((A + 1.0) + (A - 1.0) * cosw0 + sqrtA_alpha);
            b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cosw0);
            b2 = A * ((A + 1.0) + (A - 1.0) * cosw0 - sqrtA_alpha);
            a0 = (A + 1.0) - (A - 1.0) * cosw0 + sqrtA_alpha;
            a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cosw0);
            a2 = (A + 1.0) - (A - 1.0) * cosw0 - sqrtA_alpha;
            break;
        }
            
        case FilterType::AllPass:
            // H(s) = (s^2 - s/Q + 1) / (s^2 + s/Q + 1)
            b0 = 1.0 - alpha;
            b1 = -2.0 * cosw0;
            b2 = 1.0 + alpha;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cosw0;
            a2 = 1.0 - alpha;
            break;
    }
    
    // Normalize coefficients (divide by a0)
    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
    a1 /= a0;
    a2 /= a0;
}

float BiquadFilter::processSample(float input) {
    // Direct Form II Transposed
    // More numerically stable for floating point
    const double x = static_cast<double>(input);
    const double y = b0 * x + z1;
    z1 = b1 * x - a1 * y + z2;
    z2 = b2 * x - a2 * y;
    
    return static_cast<float>(y);
}

void BiquadFilter::processBlock(float* data, int numSamples) {
    for (int i = 0; i < numSamples; ++i) {
        data[i] = processSample(data[i]);
    }
}

float BiquadFilter::getMagnitudeAtFrequency(float frequency) const {
    // Calculate magnitude response at given frequency
    // Using z = e^(jw) where w = 2*pi*f/fs
    
    const double w = 2.0 * pi * static_cast<double>(frequency) / sampleRate;
    
    // Complex exponentials
    const std::complex<double> z1_c = std::exp(std::complex<double>(0.0, -w));
    const std::complex<double> z2_c = std::exp(std::complex<double>(0.0, -2.0 * w));
    
    // Numerator: b0 + b1*z^-1 + b2*z^-2
    const std::complex<double> num = b0 + b1 * z1_c + b2 * z2_c;
    
    // Denominator: 1 + a1*z^-1 + a2*z^-2
    const std::complex<double> den = 1.0 + a1 * z1_c + a2 * z2_c;
    
    // Magnitude of transfer function
    const std::complex<double> H = num / den;
    
    return static_cast<float>(std::abs(H));
}

float BiquadFilter::getPhaseAtFrequency(float frequency) const {
    const double w = 2.0 * pi * static_cast<double>(frequency) / sampleRate;

    const std::complex<double> z1_c = std::exp(std::complex<double>(0.0, -w));
    const std::complex<double> z2_c = std::exp(std::complex<double>(0.0, -2.0 * w));

    const std::complex<double> num = b0 + b1 * z1_c + b2 * z2_c;
    const std::complex<double> den = 1.0 + a1 * z1_c + a2 * z2_c;

    const std::complex<double> H = num / den;

    return static_cast<float>(std::arg(H));
}

float BiquadFilter::calcMagnitudeFromParams(FilterType type, float frequency, float q,
                                             float gainDb, double sampleRate, float evalFrequency) {
    // Calculate coefficients from parameters (same as updateCoefficients)
    const double freq = std::clamp(static_cast<double>(frequency), 10.0, sampleRate * 0.499);
    const double Q = std::max(0.01, static_cast<double>(q));
    const double w0 = 2.0 * pi * freq / sampleRate;
    const double cosw0 = std::cos(w0);
    const double sinw0 = std::sin(w0);
    const double alpha = sinw0 / (2.0 * Q);
    const double A = std::pow(10.0, static_cast<double>(gainDb) / 40.0);

    double b0_l = 1.0, b1_l = 0.0, b2_l = 0.0;
    double a0_l = 1.0, a1_l = 0.0, a2_l = 0.0;

    switch (type) {
        case FilterType::LowPass:
            b0_l = (1.0 - cosw0) / 2.0;
            b1_l = 1.0 - cosw0;
            b2_l = (1.0 - cosw0) / 2.0;
            a0_l = 1.0 + alpha;
            a1_l = -2.0 * cosw0;
            a2_l = 1.0 - alpha;
            break;
        case FilterType::HighPass:
            b0_l = (1.0 + cosw0) / 2.0;
            b1_l = -(1.0 + cosw0);
            b2_l = (1.0 + cosw0) / 2.0;
            a0_l = 1.0 + alpha;
            a1_l = -2.0 * cosw0;
            a2_l = 1.0 - alpha;
            break;
        case FilterType::BandPass:
            b0_l = alpha;
            b1_l = 0.0;
            b2_l = -alpha;
            a0_l = 1.0 + alpha;
            a1_l = -2.0 * cosw0;
            a2_l = 1.0 - alpha;
            break;
        case FilterType::Notch:
            b0_l = 1.0;
            b1_l = -2.0 * cosw0;
            b2_l = 1.0;
            a0_l = 1.0 + alpha;
            a1_l = -2.0 * cosw0;
            a2_l = 1.0 - alpha;
            break;
        case FilterType::Peak:
            b0_l = 1.0 + alpha * A;
            b1_l = -2.0 * cosw0;
            b2_l = 1.0 - alpha * A;
            a0_l = 1.0 + alpha / A;
            a1_l = -2.0 * cosw0;
            a2_l = 1.0 - alpha / A;
            break;
        case FilterType::LowShelf: {
            const double sqrtA = std::sqrt(A);
            const double sqrtA_alpha = 2.0 * sqrtA * alpha;
            b0_l = A * ((A + 1.0) - (A - 1.0) * cosw0 + sqrtA_alpha);
            b1_l = 2.0 * A * ((A - 1.0) - (A + 1.0) * cosw0);
            b2_l = A * ((A + 1.0) - (A - 1.0) * cosw0 - sqrtA_alpha);
            a0_l = (A + 1.0) + (A - 1.0) * cosw0 + sqrtA_alpha;
            a1_l = -2.0 * ((A - 1.0) + (A + 1.0) * cosw0);
            a2_l = (A + 1.0) + (A - 1.0) * cosw0 - sqrtA_alpha;
            break;
        }
        case FilterType::HighShelf: {
            const double sqrtA = std::sqrt(A);
            const double sqrtA_alpha = 2.0 * sqrtA * alpha;
            b0_l = A * ((A + 1.0) + (A - 1.0) * cosw0 + sqrtA_alpha);
            b1_l = -2.0 * A * ((A - 1.0) + (A + 1.0) * cosw0);
            b2_l = A * ((A + 1.0) + (A - 1.0) * cosw0 - sqrtA_alpha);
            a0_l = (A + 1.0) - (A - 1.0) * cosw0 + sqrtA_alpha;
            a1_l = 2.0 * ((A - 1.0) - (A + 1.0) * cosw0);
            a2_l = (A + 1.0) - (A - 1.0) * cosw0 - sqrtA_alpha;
            break;
        }
        case FilterType::AllPass:
            b0_l = 1.0 - alpha;
            b1_l = -2.0 * cosw0;
            b2_l = 1.0 + alpha;
            a0_l = 1.0 + alpha;
            a1_l = -2.0 * cosw0;
            a2_l = 1.0 - alpha;
            break;
    }

    // Normalize
    b0_l /= a0_l;
    b1_l /= a0_l;
    b2_l /= a0_l;
    a1_l /= a0_l;
    a2_l /= a0_l;

    // Calculate magnitude at evalFrequency
    const double w = 2.0 * pi * static_cast<double>(evalFrequency) / sampleRate;
    const std::complex<double> z1_c = std::exp(std::complex<double>(0.0, -w));
    const std::complex<double> z2_c = std::exp(std::complex<double>(0.0, -2.0 * w));
    const std::complex<double> num = b0_l + b1_l * z1_c + b2_l * z2_c;
    const std::complex<double> den = 1.0 + a1_l * z1_c + a2_l * z2_c;
    const std::complex<double> H = num / den;

    return static_cast<float>(std::abs(H));
}

} // namespace SeshEQ
