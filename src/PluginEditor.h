#pragma once

#include <JuceHeader.h>

#include "Components.h"
#include "PluginProcessor.h"
#include "Voice.h"

//==============================================================================
class SeedAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer, SectionComponent::Listener {
public:
    SeedAudioProcessorEditor(SeedAudioProcessor &);
    ~SeedAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;
    void resized() override;

private:
    SeedAudioProcessor &audioProcessor;
    ANALYSER_MODE analyserMode = ANALYSER_MODE::Spectrum;

    SectionComponent voiceComponent;
    AnalyserToggle analyserToggle;
    AnalyserWindow analyserWindow;
    StatusComponent statusComponent;
    SectionComponent utilComponent;
    SectionComponent oscComponents[NUM_OSC];
    SectionComponent filterComponents[NUM_FILTER];
    SectionComponent modEnvComponents[NUM_MODENV];
    SectionComponent delayComponent;
    SectionComponent masterComponent;

    virtual void timerCallback() override;
    virtual void enabledChanged(SectionComponent *section) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SeedAudioProcessorEditor)
};
