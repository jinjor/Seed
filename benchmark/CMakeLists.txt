juce_add_console_app(SeedBenchmarks)

target_compile_features(SeedBenchmarks PUBLIC cxx_std_17)

juce_generate_juce_header(SeedBenchmarks)

target_link_libraries(SeedBenchmarks
  PUBLIC
    benchmark::benchmark
    gtest_main
    SeedPluginBinaries
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_cryptography
    juce::juce_data_structures
    juce::juce_dsp
)

target_sources(SeedBenchmarks
  PRIVATE
    Benchmark.cpp
    ../src/Params.cpp
    ../src/Voice.cpp
)