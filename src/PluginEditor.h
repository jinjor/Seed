#pragma once

#include <JuceHeader.h>

#include "Components.h"
#include "PluginProcessor.h"

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

    AnalyserToggle analyserToggle;
    AnalyserWindow analyserWindow;
    StatusComponent statusComponent;

    virtual void timerCallback() override;
    virtual void enabledChanged(SectionComponent *section) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SeedAudioProcessorEditor)
};
