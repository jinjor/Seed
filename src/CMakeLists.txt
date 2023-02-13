juce_add_plugin(SeedPlugin
    # VERSION ...                               # Set this if the plugin version is different to the project version
    # ICON_BIG ""                               # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
    # ICON_SMALL "${CMAKE_SOURCE_DIR}/assets/icon.png " 
    COMPANY_NAME "Yosuke Torii"                 # Specify the name of the plugin's author
    # COMPANY_COPYRIGHT "Yosuke Torii"
    # COMPANY_WEBSITE "https://github.com/jinjor"
    COMPANY_EMAIL "jinjorweb@gmail.com"
    IS_SYNTH FALSE                              # Is this a synth or an effect?
    NEEDS_MIDI_INPUT FALSE                      # Does the plugin need midi input?
    NEEDS_MIDI_OUTPUT FALSE                     # Does the plugin need midi output?
    IS_MIDI_EFFECT FALSE                        # Is this plugin a MIDI effect?
    # EDITOR_WANTS_KEYBOARD_FOCUS TRUE/FALSE    # Does the editor need keyboard focus?
    COPY_PLUGIN_AFTER_BUILD TRUE                # Should the plugin be installed to a default location after building?
    PLUGIN_MANUFACTURER_CODE JNJR               # A four-character manufacturer id with at least one upper-case character
    PLUGIN_CODE Seed                            # A unique four-character plugin id with exactly one upper-case character
                                                # GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case
    FORMATS VST3                                # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
    VST3_CATEGORIES "Analyzer"                  # The name of the final executable, which can differ from the target name
    # AU_MAIN_TYPE "kAudioUnitType_Effect"
    # AU_SANDBOX_SAFE TRUE
    # AAX_CATEGORY ""
    # HARDENED_RUNTIME_ENABLED # macOS app settings
    # HARDENED_RUNTIME_OPTIONS
    # APP_SANDBOX_ENABLED
    # APP_SANDBOX_INHERIT
    # DESCRIPTION ""
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "This applicaion requires a permission to use an audio input device of your computer. By Default, Built-In microphone will be used."
    PRODUCT_NAME "Seed"
)

target_compile_features(SeedPlugin PUBLIC cxx_std_17)

juce_generate_juce_header(SeedPlugin)

file(GLOB sources *.cpp)
target_sources(SeedPlugin
    PRIVATE
        ${sources}
)

target_compile_definitions(SeedPlugin
    PUBLIC
        JUCE_WEB_BROWSER=0              # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0                 # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        JUCE_DISABLE_CAUTIOUS_PARAMETER_ID_CHECKING=1
)

target_link_libraries(SeedPlugin
    PUBLIC
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
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_opengl
        juce::juce_product_unlocking
)