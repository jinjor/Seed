#include "PluginEditor.h"

#include "PluginProcessor.h"
#include "StyleConstants.h"

using namespace styles;

//==============================================================================
SeedAudioProcessorEditor::SeedAudioProcessorEditor(SeedAudioProcessor &p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      analyserToggle(&analyserMode),
      analyserWindow(&analyserMode, &p.latestDataProvider),
      statusComponent(&p.latestDataProvider),
      analyserWindow2(p.recorder) {
    getLookAndFeel().setColour(juce::Label::textColourId, colour::TEXT);

    addAndMakeVisible(analyserToggle);
    addAndMakeVisible(analyserWindow);
    addAndMakeVisible(statusComponent);
    addAndMakeVisible(analyserWindow2);

    setSize(1024, 768);
#if JUCE_DEBUG
    setResizable(true, true);  // for debug
#endif
    startTimerHz(30.0f);
}

SeedAudioProcessorEditor::~SeedAudioProcessorEditor() {}

//==============================================================================
void SeedAudioProcessorEditor::paint(juce::Graphics &g) {
    auto bounds = getLocalBounds();
    auto height = bounds.getHeight();
    auto upperArea = bounds.removeFromTop(height * 0.12);

    g.fillAll(colour::BACKGROUND);

    auto areas = std::array{upperArea};
    for (auto &area : areas) {
        {
            juce::Path p;
            p.addLineSegment(juce::Line<float>(0, area.getBottom() - 0.5, area.getWidth(), area.getBottom() - 0.5), 0);
            g.setColour(juce::Colour(10, 10, 10));
            g.strokePath(p, juce::PathStrokeType(1));
        }
        {
            juce::Path p;
            p.addLineSegment(juce::Line<float>(0, area.getBottom() + 0.5, area.getWidth(), area.getBottom() + 0.5), 0);
            g.setColour(juce::Colour(60, 60, 60));
            g.strokePath(p, juce::PathStrokeType(1));
        }
    }
}

void SeedAudioProcessorEditor::resized() {
    auto bounds = getLocalBounds();

    auto width = bounds.getWidth();
    auto height = bounds.getHeight();

    auto upperHeight = height * 0.12;
    auto middleHeight = height - upperHeight;
    {
        auto upperArea = bounds.removeFromTop(upperHeight).reduced(AREA_PADDING_X, AREA_PADDING_Y);
        auto sideWidth = width * 0.36;
        auto centreWidth = width - sideWidth * 2;

        auto analyserToggleWidth = sideWidth * 1 / 3;
        auto statusWidth = sideWidth * 0.5;

        analyserToggle.setBounds(upperArea.removeFromLeft(analyserToggleWidth).reduced(3));
        analyserWindow.setBounds(upperArea.removeFromLeft(centreWidth).reduced(3));
        statusComponent.setBounds(upperArea.removeFromLeft(statusWidth));
    }
    {
        auto middleArea = bounds.removeFromTop(middleHeight).reduced(AREA_PADDING_X, AREA_PADDING_Y);
        analyserWindow2.setBounds(middleArea.reduced(30));
    }
}
void SeedAudioProcessorEditor::timerCallback() {}
void SeedAudioProcessorEditor::enabledChanged(SectionComponent *section) {}