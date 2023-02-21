#include "Params.h"

//==============================================================================
EntryParams::EntryParams(int index) {
    auto idPrefix = "E" + juce::String(index) + "_";
    auto namePrefix = "E" + juce::String(index) + " ";
    BaseFreq =
        new juce::AudioParameterFloat(idPrefix + "BASE_FREQ", namePrefix + "Base Freq", -20.0f, 20000.0f, 440.0f);
}
void EntryParams::addAllParameters(juce::AudioProcessor& processor) { processor.addParameter(BaseFreq); }
void EntryParams::saveParameters(juce::XmlElement& xml) {
    xml.setAttribute(BaseFreq->paramID, (double)BaseFreq->get());
}
void EntryParams::loadParameters(juce::XmlElement& xml) {
    *BaseFreq = (float)xml.getDoubleAttribute(BaseFreq->paramID, 440.0);
}

//==============================================================================
AllParams::AllParams() : entryParams{EntryParams{0}, EntryParams{1}, EntryParams{2}, EntryParams{3}} {}
void AllParams::addAllParameters(juce::AudioProcessor& processor) {
    for (auto& params : entryParams) {
        params.addAllParameters(processor);
    }
}
void AllParams::saveParameters(juce::XmlElement& xml) {
    for (auto& params : entryParams) {
        params.saveParameters(xml);
    }
}
void AllParams::loadParameters(juce::XmlElement& xml) {
    for (auto& params : entryParams) {
        params.loadParameters(xml);
    }
}