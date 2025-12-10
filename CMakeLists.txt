cmake_minimum_required(VERSION 3.15)

project(SeshEQ VERSION 1.0.0 LANGUAGES CXX)

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
    src/utils/Parameters.cpp
    src/utils/FFTProcessor.cpp
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
    src/dsp/MidSideProcessor.h
    src/dsp/Oversampler.h
    src/dsp/SidechainFilter.h
    src/utils/Parameters.h
    src/utils/SmoothValue.h
    src/utils/FFTProcessor.h
    src/ui/SpectrumAnalyzer.h
    src/ui/EQCurveDisplay.h
    src/ui/MeterComponent.h
    src/ui/LookAndFeel.h
)

# Define the plugin
juce_add_plugin(SeshEQ
    COMPANY_NAME "SeshAudio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Sesh
    PLUGIN_CODE SEQ1
    FORMATS VST3 Standalone
    PRODUCT_NAME "SeshEQ"
)

# Add sources
target_sources(SeshEQ
    PRIVATE
        ${PLUGIN_SOURCES}
        ${PLUGIN_HEADERS}
)

# Preprocessor definitions
target_compile_definitions(SeshEQ
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

# Link JUCE modules
target_link_libraries(SeshEQ
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Include directories
target_include_directories(SeshEQ
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

    add_executable(SeshEQ_Tests
        tests/BiquadFilterTests.cpp
        tests/LevelDetectorTests.cpp
        src/dsp/BiquadFilter.cpp
        src/dsp/LevelDetector.cpp
    )

    target_include_directories(SeshEQ_Tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(SeshEQ_Tests
        PRIVATE
            GTest::gtest_main
            GTest::gmock
    )

    target_compile_definitions(SeshEQ_Tests
        PRIVATE
            SESHEQ_TESTING=1
    )

    include(GoogleTest)
    gtest_discover_tests(SeshEQ_Tests)
endif()

# Print build info
message(STATUS "SeshEQ Version: ${PROJECT_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
