cmake_minimum_required(VERSION 3.15)

project(SeshNxQuanta VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option to build tests
option(BUILD_TESTS "Build unit tests" ON)

# Find or fetch JUCE
include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.9
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(JUCE)

# Plugin sources
set(PLUGIN_SOURCES
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/dsp/BiquadFilter.cpp
    src/dsp/EQProcessor.cpp
    src/dsp/LevelDetector.cpp
    src/dsp/Compressor.cpp
    src/dsp/Gate.cpp
    src/dsp/Limiter.cpp
    src/dsp/LinearPhaseEQ.cpp
    src/dsp/DynamicEQ.cpp
    src/dsp/BandDynamics.cpp
    src/utils/Parameters.cpp
    src/utils/FFTProcessor.cpp
    src/utils/PresetManager.cpp
    src/ui/SpectrumAnalyzer.cpp
    src/ui/EQCurveDisplay.cpp
    src/ui/MeterComponent.cpp
    src/ui/LookAndFeel.cpp
)

set(PLUGIN_HEADERS
    src/PluginProcessor.h
    src/PluginEditor.h
    src/dsp/BiquadFilter.h
    src/dsp/EQProcessor.h
    src/dsp/LevelDetector.h
    src/dsp/Compressor.h
    src/dsp/Gate.h
    src/dsp/Limiter.h
    src/dsp/LinearPhaseEQ.h
    src/dsp/DynamicEQ.h
    src/dsp/BandDynamics.h
    src/utils/Parameters.h
    src/utils/SmoothValue.h
    src/utils/FFTProcessor.h
    src/utils/MidSideProcessor.h
    src/utils/PresetManager.h
    src/ui/SpectrumAnalyzer.h
    src/ui/EQCurveDisplay.h
    src/ui/MeterComponent.h
    src/ui/LookAndFeel.h
)

# Define the plugin
juce_add_plugin(SeshNxQuanta
    COMPANY_NAME "SeshAudio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Sesh
    PLUGIN_CODE SNQ1
    FORMATS VST3 Standalone
    PRODUCT_NAME "SeshNx Quanta"
)

# Add sources
target_sources(SeshNxQuanta
    PRIVATE
        ${PLUGIN_SOURCES}
        ${PLUGIN_HEADERS}
)

# Preprocessor definitions
target_compile_definitions(SeshNxQuanta
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_USE_DIRECTWRITE=0
        JUCE_DIRECT2D=0
        JUCE_USE_OPENGL=0
)

# Link JUCE modules
target_link_libraries(SeshNxQuanta
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Include directories
target_include_directories(SeshNxQuanta
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Unit tests
if(BUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(SeshNxQuanta_Tests
        tests/BiquadFilterTests.cpp
        tests/LevelDetectorTests.cpp
        src/dsp/BiquadFilter.cpp
        src/dsp/LevelDetector.cpp
    )

    target_include_directories(SeshNxQuanta_Tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(SeshNxQuanta_Tests
        PRIVATE
            GTest::gtest_main
            GTest::gmock
    )

    target_compile_definitions(SeshNxQuanta_Tests
        PRIVATE
            SESHNXQUANTA_TESTING=1
    )

    include(GoogleTest)
    gtest_discover_tests(SeshNxQuanta_Tests)
endif()

# Print build info
message(STATUS "SeshNx Quanta Version: ${PROJECT_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
