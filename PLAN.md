# SeshEQ - VST3 EQ and Dynamics Plugin Plan

## 1. Project Overview

**Project Name:** SeshEQ  
**Type:** VST3 Audio Plugin  
**Purpose:** Professional-grade parametric equalizer with integrated dynamics processing for mixing and mastering applications.

### Goals
- Create a high-quality, low-latency EQ and dynamics processor
- Provide intuitive UI with real-time spectrum analysis
- Support stereo and mono processing
- Achieve professional audio quality (64-bit internal processing)
- Cross-platform compatibility (Windows, macOS, Linux)

---

## 2. Technical Stack

### Framework Options
| Option | Pros | Cons |
|--------|------|------|
| **JUCE** | Industry standard, excellent documentation, cross-platform | Licensing costs for commercial use |
| **iPlug2** | Free, lightweight, good VST3 support | Smaller community |
| **VST3 SDK Direct** | Full control, no dependencies | More boilerplate, steeper learning curve |

**Recommended:** JUCE Framework (most mature ecosystem for audio plugins)

### Development Languages
- **C++17/20** - Core DSP and plugin logic
- **SIMD Intrinsics** - Performance-critical DSP code
- **OpenGL/Metal** - GPU-accelerated UI rendering

### Build System
- CMake for cross-platform builds
- GitHub Actions for CI/CD

---

## 3. Feature Specifications

### 3.1 Equalizer Section

#### Band Types
| Band Type | Description | Use Case |
|-----------|-------------|----------|
| High Pass Filter (HPF) | 6/12/18/24/48 dB/oct slopes | Remove low-end rumble |
| Low Shelf | Boost/cut below frequency | Bass adjustment |
| Parametric Bell | Boost/cut with Q control | Surgical/broad corrections |
| High Shelf | Boost/cut above frequency | Air/brightness |
| Low Pass Filter (LPF) | 6/12/18/24/48 dB/oct slopes | Remove high-end harshness |
| Notch Filter | Narrow cut | Remove resonances |
| Band Pass | Pass only selected range | Special effects |

#### EQ Parameters (Per Band)
```
- Frequency: 20 Hz - 20 kHz (logarithmic)
- Gain: -24 dB to +24 dB
- Q (Bandwidth): 0.1 to 18.0
- Filter Type: Selectable per band
- Band Enable: On/Off toggle
```

#### EQ Features
- **8 fully parametric bands** (expandable)
- **Analog-modeled filter curves** (optional saturation)
- **Linear Phase mode** (for mastering, adds latency)
- **Minimum Phase mode** (default, low latency)
- **Mid/Side processing** option
- **Auto-gain compensation**

### 3.2 Dynamics Section

#### Compressor
```
Parameters:
- Threshold: -60 dB to 0 dB
- Ratio: 1:1 to ∞:1 (limiter mode)
- Attack: 0.01 ms to 100 ms
- Release: 10 ms to 3000 ms (+ Auto mode)
- Knee: 0 dB (hard) to 24 dB (soft)
- Makeup Gain: 0 dB to +24 dB
- Mix (Parallel): 0% to 100%
- Detection: Peak / RMS / True Peak
- Sidechain: Internal / External / EQ-filtered
```

#### Expander/Gate
```
Parameters:
- Threshold: -80 dB to 0 dB
- Ratio: 1:1 to 1:∞ (gate mode)
- Attack: 0.01 ms to 50 ms
- Hold: 0 ms to 500 ms
- Release: 10 ms to 2000 ms
- Range: 0 dB to -80 dB
```

#### Limiter (Output Stage)
```
Parameters:
- Ceiling: -12 dB to 0 dB
- Release: 10 ms to 1000 ms (+ Auto)
- True Peak limiting option
```

### 3.3 Global Features

- **Input/Output Gain** with metering
- **Dry/Wet Mix** for parallel processing
- **Oversampling**: 1x, 2x, 4x, 8x
- **A/B Comparison** with undo/redo
- **Preset System** with factory presets
- **Spectrum Analyzer** (Pre/Post/Overlay)
- **Gain Reduction Meter** for dynamics
- **Stereo/Mono/Mid-Side** mode selector
- **Channel Link** (0-100%)

---

## 4. DSP Architecture

### Signal Flow
```
┌─────────────────────────────────────────────────────────────────┐
│                         SIGNAL FLOW                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  INPUT ──► INPUT GAIN ──► OVERSAMPLING (UP) ──┐                 │
│                                                │                 │
│            ┌───────────────────────────────────┘                 │
│            ▼                                                     │
│     ┌──────────────┐                                            │
│     │  EQ SECTION  │ ◄── Pre/Post Dynamics routing option       │
│     │  (8 Bands)   │                                            │
│     └──────┬───────┘                                            │
│            │                                                     │
│            ▼                                                     │
│     ┌──────────────┐                                            │
│     │   DYNAMICS   │                                            │
│     │ ┌──────────┐ │                                            │
│     │ │Compressor│ │                                            │
│     │ └──────────┘ │                                            │
│     │ ┌──────────┐ │                                            │
│     │ │  Gate    │ │                                            │
│     │ └──────────┘ │                                            │
│     └──────┬───────┘                                            │
│            │                                                     │
│            ▼                                                     │
│     ┌──────────────┐                                            │
│     │   LIMITER    │                                            │
│     └──────┬───────┘                                            │
│            │                                                     │
│            ▼                                                     │
│     OVERSAMPLING (DOWN) ──► OUTPUT GAIN ──► DRY/WET ──► OUTPUT  │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

### Filter Algorithms

#### Biquad Filters (IIR)
- Robert Bristow-Johnson's Audio EQ Cookbook formulas
- Direct Form II Transposed implementation
- Double precision coefficients

#### Linear Phase FIR (Optional)
- FFT-based convolution for linear phase mode
- Symmetric FIR filter design
- Latency: ~2000-8000 samples depending on accuracy

### Dynamics Algorithms

#### Compressor Design
```cpp
// Envelope follower (logarithmic domain)
// Smooth ballistics with attack/release
// Feed-forward topology with optional feedback
// Soft knee implementation using polynomial curves
```

#### Level Detection
- **Peak**: Instantaneous sample magnitude
- **RMS**: Root Mean Square over window (10-300ms)
- **True Peak**: Oversampled peak detection (ITU-R BS.1770)

---

## 5. User Interface Design

### Layout Concept
```
┌─────────────────────────────────────────────────────────────────────┐
│  SESHEQ                                            [A/B] [Preset ▼] │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ┌────────────────────────────────────────────────────────────────┐ │
│  │                                                                 │ │
│  │                    SPECTRUM ANALYZER                            │ │
│  │                    (Interactive EQ Curve)                       │ │
│  │              ●───────●                                          │ │
│  │         ●              ───●───                                  │ │
│  │     ●                         ───●                              │ │
│  │  20Hz            1kHz            10kHz            20kHz         │ │
│  │                                                                 │ │
│  └────────────────────────────────────────────────────────────────┘ │
│                                                                      │
│  ┌─ EQ BANDS ───────────────────────────────────────────────────┐   │
│  │ [1]HPF  [2]LSH  [3]BELL  [4]BELL  [5]BELL  [6]BELL  [7]HSH [8]LPF│
│  │  80Hz   120Hz   400Hz    1kHz     2.5kHz   5kHz    10kHz  16kHz │
│  │  24dB   +2dB    -3dB     +1dB     -2dB     +3dB    +1dB   12dB  │
│  └──────────────────────────────────────────────────────────────────┘│
│                                                                      │
│  ┌─ DYNAMICS ──────────────────────────────────────────────────────┐│
│  │                                                                  ││
│  │  COMPRESSOR              GATE                 OUTPUT             ││
│  │  ┌──┐ ┌──┐ ┌──┐         ┌──┐ ┌──┐           ┌──┐               ││
│  │  │TH│ │RT│ │AT│         │TH│ │RG│           │CL│    [GR METER] ││
│  │  └──┘ └──┘ └──┘         └──┘ └──┘           └──┘               ││
│  │  -18  4:1  10ms         -40  -60            -0.3               ││
│  │                                                                  ││
│  │  ┌──┐ ┌──┐ ┌──┐         ┌──┐ ┌──┐                              ││
│  │  │RL│ │KN│ │MK│         │AT│ │RL│           [LIMITER ON]       ││
│  │  └──┘ └──┘ └──┘         └──┘ └──┘                              ││
│  │  100   6   +4           0.5  50ms                              ││
│  │                                                                  ││
│  └──────────────────────────────────────────────────────────────────┘│
│                                                                      │
│  [IN ▓▓▓▓▓▓░░░░] -18dB        [OUT ▓▓▓▓▓▓▓░░░] -12dB   [MIX 100%]  │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### UI Features
- **Draggable EQ nodes** on spectrum display
- **Real-time spectrum analysis** (FFT-based)
- **Resizable window** (min 800x500, scalable to 200%)
- **Dark/Light themes**
- **Tooltips** for all parameters
- **Mouse wheel** support for fine adjustment
- **Double-click** to reset to default
- **Shift+drag** for fine control
- **Alt+click** for band solo

---

## 6. Project Structure

```
SeshEQ/
├── CMakeLists.txt
├── README.md
├── PLAN.md
├── LICENSE
│
├── src/
│   ├── PluginProcessor.cpp/.h      # Main audio processor
│   ├── PluginEditor.cpp/.h         # Main UI component
│   │
│   ├── dsp/
│   │   ├── EQProcessor.cpp/.h      # EQ processing
│   │   ├── BiquadFilter.cpp/.h     # Biquad filter implementation
│   │   ├── LinearPhaseEQ.cpp/.h    # FIR-based linear phase
│   │   ├── Compressor.cpp/.h       # Compressor processor
│   │   ├── Gate.cpp/.h             # Gate/expander processor
│   │   ├── Limiter.cpp/.h          # Output limiter
│   │   ├── LevelDetector.cpp/.h    # Peak/RMS detection
│   │   ├── Oversampler.cpp/.h      # Oversampling utilities
│   │   └── SIMDHelpers.h           # SIMD optimizations
│   │
│   ├── ui/
│   │   ├── SpectrumAnalyzer.cpp/.h # FFT spectrum display
│   │   ├── EQCurveDisplay.cpp/.h   # Interactive EQ curve
│   │   ├── EQBandControls.cpp/.h   # Band parameter controls
│   │   ├── DynamicsPanel.cpp/.h    # Compressor/gate UI
│   │   ├── MeterComponent.cpp/.h   # Level meters
│   │   ├── KnobComponent.cpp/.h    # Custom rotary slider
│   │   └── LookAndFeel.cpp/.h      # Custom styling
│   │
│   ├── presets/
│   │   ├── PresetManager.cpp/.h    # Save/load presets
│   │   └── factory/                # Factory preset files
│   │
│   └── utils/
│       ├── Parameters.cpp/.h       # Parameter definitions
│       ├── FFTProcessor.cpp/.h     # FFT utilities
│       └── SmoothValue.h           # Parameter smoothing
│
├── resources/
│   ├── fonts/
│   ├── images/
│   └── presets/
│
├── tests/
│   ├── DSPTests.cpp
│   ├── FilterTests.cpp
│   └── ParameterTests.cpp
│
└── builds/
    ├── Windows/
    ├── macOS/
    └── Linux/
```

---

## 7. Development Phases

### Phase 1: Foundation (Weeks 1-2)
- [ ] Set up JUCE project with CMake
- [ ] Implement basic plugin shell (processor + editor)
- [ ] Create parameter system with APVTS
- [ ] Implement basic biquad filter class
- [ ] Unit tests for filter accuracy

### Phase 2: EQ Core (Weeks 3-4)
- [ ] Implement all filter types (HPF, LPF, shelf, bell, notch)
- [ ] Create 8-band EQ chain
- [ ] Implement parameter smoothing
- [ ] Add oversampling support
- [ ] Basic EQ UI with controls

### Phase 3: Dynamics Core (Weeks 5-6)
- [ ] Implement level detector (peak/RMS)
- [ ] Create compressor algorithm
- [ ] Create gate/expander algorithm
- [ ] Implement limiter
- [ ] Dynamics UI controls

### Phase 4: Advanced DSP (Weeks 7-8)
- [ ] Linear phase mode (FFT-based)
- [ ] Mid/Side processing
- [ ] Sidechain EQ filter
- [ ] SIMD optimizations
- [ ] CPU usage profiling and optimization

### Phase 5: UI Polish (Weeks 9-10)
- [ ] Spectrum analyzer (FFT display)
- [ ] Interactive EQ curve with draggable nodes
- [ ] Gain reduction meter
- [ ] Custom look and feel
- [ ] Resizable UI
- [ ] Tooltips and accessibility

### Phase 6: Presets & Testing (Weeks 11-12)
- [ ] Preset manager (save/load/browse)
- [ ] Factory preset creation
- [ ] A/B comparison feature
- [ ] Comprehensive testing
- [ ] DAW compatibility testing
- [ ] Beta release

### Phase 7: Release (Week 13+)
- [ ] Documentation
- [ ] Installer creation
- [ ] Website and distribution
- [ ] User feedback and iteration

---

## 8. Technical Specifications

### Audio Specifications
| Specification | Value |
|---------------|-------|
| Sample Rates | 44.1, 48, 88.2, 96, 176.4, 192 kHz |
| Bit Depth | 32-bit float (64-bit internal) |
| Channels | Mono, Stereo, Mid/Side |
| Latency (Min Phase) | 0 samples |
| Latency (Lin Phase) | ~2048-8192 samples |
| Max Oversampling | 8x |

### Performance Targets
| Metric | Target |
|--------|--------|
| CPU Usage (8 bands, stereo) | < 1% @ 48kHz on modern CPU |
| CPU Usage (with analyzer) | < 2% |
| Memory Usage | < 50 MB |
| UI Frame Rate | 60 FPS |

### Plugin Formats
- VST3 (primary)
- AU (macOS)
- AAX (Pro Tools) - optional
- Standalone application

---

## 9. Testing Strategy

### Unit Tests
- Filter frequency response accuracy
- Compressor gain reduction calculations
- Parameter range validation
- Preset save/load integrity

### Integration Tests
- Full signal chain processing
- Automation parameter changes
- State save/restore (DAW recall)

### DAW Compatibility Testing
- Ableton Live
- Logic Pro
- Pro Tools
- FL Studio
- Cubase/Nuendo
- Reaper
- Studio One

### Audio Quality Tests
- Null test against reference implementations
- THD+N measurements
- Frequency response sweeps
- Impulse response analysis

---

## 10. Factory Presets

### Mixing Presets
- **Vocal Polish** - Gentle compression + presence boost
- **Drum Bus** - Punchy compression + low-end control
- **Bass Guitar** - Low-end enhancement + limiting
- **Guitar Shaper** - Mid-focus + dynamic control
- **Mix Bus Glue** - Subtle compression + master EQ

### Mastering Presets
- **Mastering EQ** - Transparent broad strokes
- **Loudness Maximizer** - Limiting + EQ
- **Vinyl Warmth** - Analog-style coloration

### Creative Presets
- **Telephone** - Band-pass filter effect
- **Radio Ready** - Bright and compressed
- **Scooped** - Mid-cut for heavy music

### Utility Presets
- **Default** - Flat, all bands at 0
- **High Pass 80Hz** - Common vocal cleanup
- **De-Mud** - Low-mid cut around 300Hz

---

## 11. Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| JUCE licensing costs | Medium | Medium | Budget for license or consider iPlug2 |
| Cross-platform bugs | High | Medium | Regular testing on all platforms |
| CPU performance issues | Medium | High | Profile early, optimize critical paths |
| DAW compatibility | Medium | High | Test extensively, follow VST3 spec strictly |
| UI complexity | Medium | Medium | Start simple, iterate based on feedback |

---

## 12. Future Enhancements (Post-Release)

- Dynamic EQ (frequency-dependent compression)
- Multiband compressor mode
- Harmonic exciter/saturation
- Analyzer-based EQ matching
- Dolby Atmos / Spatial Audio support
- GPU-accelerated DSP
- Mobile companion app for remote control
- Machine learning-based preset suggestions

---

## 13. Resources & References

### Documentation
- [VST3 SDK Documentation](https://steinbergmedia.github.io/vst3_doc/)
- [JUCE Documentation](https://docs.juce.com/)
- [Audio EQ Cookbook](https://www.w3.org/2011/audio/audio-eq-cookbook.html)

### Books
- "Designing Audio Effect Plugins in C++" by Will Pirkle
- "DAFX: Digital Audio Effects" by Udo Zölzer

### Reference Plugins
- FabFilter Pro-Q 3 (EQ reference)
- FabFilter Pro-C 2 (Compressor reference)
- TDR Nova (Free dynamic EQ reference)

---

*Document Version: 1.0*  
*Last Updated: December 2024*
