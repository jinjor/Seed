#include "Params.h"

//==============================================================================
EntryParams::EntryParams(int index) {
    auto idPrefix = "E" + juce::String(index) + "_";
    auto namePrefix = "E" + juce::String(index) + " ";
    BaseFreq = new juce::AudioParameterFloat(idPrefix + "BASE_FREQ", namePrefix + "Base Freq", 20.0f, 20000.0f, 440.0f);
    FilterLowFreq = new juce::AudioParameterFloat(
        idPrefix + "FILTER_LOW_FREQ", namePrefix + "Filter low Freq", 20.0f, 20000.0f, 20.0f);
    FilterHighFreq = new juce::AudioParameterFloat(
        idPrefix + "FILTER_HIGH_FREQ", namePrefix + "Filter High Freq", 20.0f, 20000.0f, 20000.0f);
}
void EntryParams::addAllParameters(juce::AudioProcessor& processor) {
    processor.addParameter(BaseFreq);
    processor.addParameter(FilterLowFreq);
    processor.addParameter(FilterHighFreq);
}
void EntryParams::saveParameters(juce::XmlElement& xml) {
    xml.setAttribute(BaseFreq->paramID, (double)BaseFreq->get());
    xml.setAttribute(FilterLowFreq->paramID, (double)FilterLowFreq->get());
    xml.setAttribute(FilterHighFreq->paramID, (double)FilterHighFreq->get());
}
void EntryParams::loadParameters(juce::XmlElement& xml) {
    *BaseFreq = (float)xml.getDoubleAttribute(BaseFreq->paramID, 440.0);
    *FilterLowFreq = (float)xml.getDoubleAttribute(FilterLowFreq->paramID, 20.0);
    *FilterHighFreq = (float)xml.getDoubleAttribute(FilterHighFreq->paramID, 20000.0);
}

//==============================================================================
AllParams::AllParams() : entryParams{EntryParams{0}, EntryParams{1}, EntryParams{2}, EntryParams{3}} {
    FilterN = new juce::AudioParameterInt("FILTER_N", "Filter N", 10, 400, 100);
}
void AllParams::addAllParameters(juce::AudioProcessor& processor) {
    processor.addParameter(FilterN);
    for (auto& params : entryParams) {
        params.addAllParameters(processor);
    }
}
void AllParams::saveParameters(juce::XmlElement& xml) {
    xml.setAttribute(FilterN->paramID, FilterN->get());
    for (auto& params : entryParams) {
        params.saveParameters(xml);
    }
}
void AllParams::loadParameters(juce::XmlElement& xml) {
    *FilterN = xml.getIntAttribute(FilterN->paramID, 100);
    for (auto& params : entryParams) {
        params.loadParameters(xml);
    }
}