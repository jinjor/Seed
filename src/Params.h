#pragma once

#include <JuceHeader.h>

namespace {
const int NUM_ENTRIES = 4;
}  // namespace

//==============================================================================
class ParametersBase {
public:
    virtual ~ParametersBase() {}
    virtual void addAllParameters(juce::AudioProcessor& processor) = 0;
    virtual void saveParameters(juce::XmlElement& xml) = 0;
    virtual void loadParameters(juce::XmlElement& xml) = 0;
};

//==============================================================================
class EntryParams : public ParametersBase {
public:
    juce::AudioParameterFloat* BaseFreq;
    juce::AudioParameterFloat* FilterLowFreq;
    juce::AudioParameterFloat* FilterHighFreq;
    juce::AudioParameterFloat* PlayStartSec;
    juce::AudioParameterFloat* FocusFreq;
    juce::AudioParameterFloat* FocusSec;

    EntryParams(int index);
    EntryParams(const EntryParams&) = delete;
    EntryParams(EntryParams&&) noexcept = default;

    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;
};

//==============================================================================
class AllParams : public ParametersBase {
public:
    juce::AudioParameterInt* FilterN;
    std::array<EntryParams, NUM_ENTRIES> entryParams;

    AllParams();
    AllParams(const AllParams&) = delete;
    AllParams(AllParams&&) noexcept = default;

    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;
};