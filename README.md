# SeshEQ

A professional-grade VST3 parametric equalizer with integrated dynamics processing for mixing and mastering.

## Features

### Equalizer
- **8 fully parametric bands** with multiple filter types
- High/Low Pass Filters (6-48 dB/oct slopes)
- Low/High Shelf filters
- Parametric Bell filters with adjustable Q
- Notch and Band Pass filters
- **Linear Phase mode** for mastering (optional)
- **Mid/Side processing** support

### Dynamics
- **Compressor** with soft knee, parallel mix, and sidechain EQ
- **Gate/Expander** with hold time and range control
- **Output Limiter** with true peak detection
- Peak, RMS, and True Peak detection modes

### Interface
- Real-time spectrum analyzer
- Interactive EQ curve with draggable nodes
- Gain reduction metering
- A/B comparison
- Resizable window with dark/light themes

## Technical Specifications

| Spec | Value |
|------|-------|
| Sample Rates | 44.1 - 192 kHz |
| Processing | 64-bit internal |
| Latency (Min Phase) | 0 samples |
| Oversampling | Up to 8x |

## Plugin Formats

- VST3 (Windows, macOS, Linux)
- AU (macOS)
- Standalone application

## Building

### Requirements
- CMake 3.15+
- C++17 compiler
- JUCE Framework 7.x

### Build Steps

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

## Documentation

See [PLAN.md](PLAN.md) for detailed project planning and architecture documentation.

## License

TBD

---

*SeshEQ - Professional EQ & Dynamics*
