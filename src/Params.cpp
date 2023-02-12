#include "Params.h"

#include "Voice.h"

namespace {
juce::NormalisableRange<float> rangeWithSkewForCentre(float rangeStart, float rangeEnd, float centrePointValue) {
    auto range = juce::NormalisableRange(rangeStart, rangeEnd);
    range.setSkewForCentre(centrePointValue);
    return range;
}
}  // namespace
//==============================================================================
GlobalParams::GlobalParams() {
    std::string idPrefix = "GLOBAL_";
    std::string namePrefix = "Global ";
    Pitch = new juce::AudioParameterFloat(idPrefix + "PITCH", namePrefix + "Pitch", -1.0f, 1.0f, 0.0f);
    Pan = new juce::AudioParameterFloat(idPrefix + "PAN", namePrefix + "Pan", -1.0f, 1.0f, 0.0f);
    Expression = new juce::AudioParameterFloat(idPrefix + "EXPRESSION", namePrefix + "Expression", 0.0f, 1.0f, 1.0f);
    MidiVolume = new juce::AudioParameterFloat(idPrefix + "MIDI_VOLUME", namePrefix + "Midi Volume", 0.0f, 1.0f, 1.0f);
    freeze();
}
void GlobalParams::addAllParameters(juce::AudioProcessor& processor) {
    processor.addParameter(Pitch);
    processor.addParameter(Pan);
    processor.addParameter(Expression);
    processor.addParameter(MidiVolume);
}
void GlobalParams::saveParameters(juce::XmlElement& xml) {}
void GlobalParams::loadParameters(juce::XmlElement& xml) {}

//==============================================================================
MasterParams::MasterParams() {
    juce::String idPrefix = "MASTER_";
    juce::String namePrefix = "Master ";
    Pan = new juce::AudioParameterFloat(idPrefix + "PAN", namePrefix + "Pan", -1.0f, 1.0f, 0.0f);
    MasterVolume = new juce::AudioParameterFloat(idPrefix + "VOLUME", namePrefix + "Volume", 0.0f, 1.0f, 1.0f);
    freeze();
}
void MasterParams::addAllParameters(juce::AudioProcessor& processor) {
    processor.addParameter(Pan);
    processor.addParameter(MasterVolume);
}
void MasterParams::saveParameters(juce::XmlElement& xml) {
    xml.setAttribute(Pan->paramID, (double)Pan->get());
    xml.setAttribute(MasterVolume->paramID, (double)MasterVolume->get());
}
void MasterParams::loadParameters(juce::XmlElement& xml) {
    *Pan = (float)xml.getDoubleAttribute(Pan->paramID, 0);
    *MasterVolume = (float)xml.getDoubleAttribute(MasterVolume->paramID, 1.0);
}

//==============================================================================
VoiceParams::VoiceParams() {
    std::string idPrefix = "VOICE_";
    std::string namePrefix = "Voice ";
    PitchBendRange =
        new juce::AudioParameterInt(idPrefix + "PITCH_BEND_RANGE", namePrefix + "Pitch-Bend Range", 1, 12, 2);
    freeze();
}
void VoiceParams::addAllParameters(juce::AudioProcessor& processor) { processor.addParameter(PitchBendRange); }
void VoiceParams::saveParameters(juce::XmlElement& xml) {
    xml.setAttribute(PitchBendRange->paramID, PitchBendRange->get());
}
void VoiceParams::loadParameters(juce::XmlElement& xml) {
    *PitchBendRange = xml.getIntAttribute(PitchBendRange->paramID, 2);
}

//==============================================================================
OscParams::OscParams(int timbreIndex, int index) : index(index) {
    auto idPrefix = "T" + std::to_string(timbreIndex) + "_OSC" + std::to_string(index) + "_";
    auto namePrefix = "T" + std::to_string(timbreIndex) + " OSC" + std::to_string(index) + " ";
    Gain = new juce::AudioParameterFloat(
        idPrefix + "GAIN", namePrefix + "Gain", rangeWithSkewForCentre(0.0f, 4.0f, 1.0f), 1.0f);
    NewEnvelope = new juce::AudioParameterBool(idPrefix + "SYNC_ENVELOPE", namePrefix + "Envelope", index == 0);
    freeze();
}
void OscParams::addAllParameters(juce::AudioProcessor& processor) {
    processor.addParameter(Gain);
    processor.addParameter(NewEnvelope);
}
void OscParams::saveParameters(juce::XmlElement& xml) {
    xml.setAttribute(Gain->paramID, (double)Gain->get());
    xml.setAttribute(NewEnvelope->paramID, NewEnvelope->get());
}
void OscParams::loadParameters(juce::XmlElement& xml) {
    *Gain = (float)xml.getDoubleAttribute(Gain->paramID, 0);
    *NewEnvelope = xml.getIntAttribute(NewEnvelope->paramID, index == 0);
}

//==============================================================================
EnvelopeParams::EnvelopeParams(int timbreIndex, int index) {
    auto idPrefix = "T" + std::to_string(timbreIndex) + "_ENV" + std::to_string(index) + "_";
    auto namePrefix = "T" + std::to_string(timbreIndex) + " Env" + std::to_string(index) + " ";
    AttackCurve = new juce::AudioParameterFloat(idPrefix + "ATTACK_CURVE", "Attack Curve", 0.01, 0.99, 0.5f);
    Attack =
        new juce::AudioParameterFloat(idPrefix + "ATTACK", "Attack", rangeWithSkewForCentre(0.001f, 0.2f, 0.1f), 0.05f);
    Decay = new juce::AudioParameterFloat(idPrefix + "DECAY", "Decay", rangeWithSkewForCentre(0.01f, 1.0f, 0.4f), 0.1f);
    Release =
        new juce::AudioParameterFloat(idPrefix + "RELEASE", "Release", rangeWithSkewForCentre(0.01f, 1.0f, 0.4f), 0.1f);
    freeze();
}
void EnvelopeParams::addAllParameters(juce::AudioProcessor& processor) {
    processor.addParameter(AttackCurve);
    processor.addParameter(Attack);
    processor.addParameter(Decay);
    processor.addParameter(Release);
}
void EnvelopeParams::saveParameters(juce::XmlElement& xml) {
    xml.setAttribute(AttackCurve->paramID, (double)AttackCurve->get());
    xml.setAttribute(Attack->paramID, (double)Attack->get());
    xml.setAttribute(Decay->paramID, (double)Decay->get());
    xml.setAttribute(Release->paramID, (double)Release->get());
}
void EnvelopeParams::loadParameters(juce::XmlElement& xml) {
    *AttackCurve = (float)xml.getDoubleAttribute(AttackCurve->paramID, 0.5);
    *Attack = (float)xml.getDoubleAttribute(Attack->paramID, 0.01);
    *Decay = (float)xml.getDoubleAttribute(Decay->paramID, 0.01);
    *Release = (float)xml.getDoubleAttribute(Release->paramID, 0.01);
}

//==============================================================================
FilterParams::FilterParams(int index) {
    auto idPrefix = "FILTER" + std::to_string(index) + "_";
    auto namePrefix = "Filter" + std::to_string(index) + " ";
    Enabled = new juce::AudioParameterBool(idPrefix + "ENABLED", namePrefix + "Enabled", false);
    Type = new juce::AudioParameterChoice(
        idPrefix + "TYPE", namePrefix + "Type", FILTER_TYPE_NAMES, FILTER_TYPE_NAMES.indexOf("Lowpass"));
    FreqType = new juce::AudioParameterChoice(idPrefix + "FREQ_TYPE",
                                              namePrefix + "Freq Type",
                                              FILTER_FREQ_TYPE_NAMES,
                                              FILTER_FREQ_TYPE_NAMES.indexOf("Abs"));
    Hz = new juce::AudioParameterFloat(
        idPrefix + "HZ", namePrefix + "Hz", rangeWithSkewForCentre(30.0f, 20000.0f, 2000.0f), 4000.0f);
    Semitone = new juce::AudioParameterInt(idPrefix + "SEMITONE", namePrefix + "Semitone", -48, 48, 0);
    Q = new juce::AudioParameterFloat(
        idPrefix + "Q", namePrefix + "Q", rangeWithSkewForCentre(0.01f, 100.0f, 1.0f), 1.0f);
    Gain = new juce::AudioParameterFloat(idPrefix + "GAIN", namePrefix + "Gain", -20.0f, 20.0f, 0.0f);
    freeze();
}
void FilterParams::addAllParameters(juce::AudioProcessor& processor) {
    processor.addParameter(Enabled);
    processor.addParameter(Type);
    processor.addParameter(FreqType);
    processor.addParameter(Hz);
    processor.addParameter(Semitone);
    processor.addParameter(Q);
    processor.addParameter(Gain);
}
void FilterParams::saveParameters(juce::XmlElement& xml) {
    xml.setAttribute(Enabled->paramID, Enabled->get());
    xml.setAttribute(Type->paramID, Type->getIndex());
    xml.setAttribute(FreqType->paramID, FreqType->getIndex());
    xml.setAttribute(Hz->paramID, (double)Hz->get());
    xml.setAttribute(Semitone->paramID, Semitone->get());
    xml.setAttribute(Q->paramID, (double)Q->get());
    xml.setAttribute(Gain->paramID, (double)Gain->get());
}
void FilterParams::loadParameters(juce::XmlElement& xml) {
    *Enabled = xml.getIntAttribute(Enabled->paramID, 0);
    *Type = xml.getIntAttribute(Type->paramID, 0);
    *FreqType = xml.getIntAttribute(FreqType->paramID, 0);
    *Hz = (float)xml.getDoubleAttribute(Hz->paramID, 0);
    *Semitone = xml.getDoubleAttribute(Semitone->paramID, 0);
    *Q = (float)xml.getDoubleAttribute(Q->paramID, 1.0);
    *Gain = (float)xml.getDoubleAttribute(Gain->paramID, 0);
}

//==============================================================================
ModEnvParams::ModEnvParams(int index) {
    auto idPrefix = "MODENV" + std::to_string(index) + "_";
    auto namePrefix = "ModEnv" + std::to_string(index) + " ";
    Enabled = new juce::AudioParameterBool(idPrefix + "ENABLED", namePrefix + "Enabled", false);
    TargetType = new juce::AudioParameterChoice(idPrefix + "TARGET_TYPE",
                                                namePrefix + "Target Type",
                                                MODENV_TARGET_TYPE_NAMES,
                                                MODENV_TARGET_TYPE_NAMES.indexOf("LFO"));
    TargetFilter = new juce::AudioParameterChoice(idPrefix + "TARGET_FILTER",
                                                  namePrefix + "Target Filter",
                                                  MODENV_TARGET_FILTER_NAMES,
                                                  MODENV_TARGET_FILTER_NAMES.indexOf("All"));
    TargetFilterParam = new juce::AudioParameterChoice(idPrefix + "TARGET_FILTER_PARAM",
                                                       namePrefix + "Target Filter Param",
                                                       MODENV_TARGET_FILTER_PARAM_NAMES,
                                                       MODENV_TARGET_FILTER_PARAM_NAMES.indexOf("Freq"));
    Fade = new juce::AudioParameterChoice(
        idPrefix + "FADE", namePrefix + "Fade", MODENV_FADE_NAMES, MODENV_FADE_NAMES.indexOf("In"));
    PeakFreq = new juce::AudioParameterFloat(idPrefix + "PEAK_FREQ", namePrefix + "Peak Freq", -8.0f, 8.0, 2.0f);
    Wait = new juce::AudioParameterFloat(
        idPrefix + "WAIT", namePrefix + "Wait", rangeWithSkewForCentre(0.0f, 1.0f, 0.2f), 0.5f);
    Attack = new juce::AudioParameterFloat(
        idPrefix + "ATTACK", namePrefix + "Attack", rangeWithSkewForCentre(0.0f, 1.0f, 0.2f), 0.0f);
    Decay = new juce::AudioParameterFloat(
        idPrefix + "DECAY", namePrefix + "Decay", rangeWithSkewForCentre(0.0f, 1.0f, 0.4f), 0.2f);
    freeze();
}
void ModEnvParams::addAllParameters(juce::AudioProcessor& processor) {
    processor.addParameter(Enabled);
    processor.addParameter(TargetType);
    processor.addParameter(TargetFilter);
    processor.addParameter(TargetFilterParam);
    processor.addParameter(Fade);
    processor.addParameter(PeakFreq);
    processor.addParameter(Wait);
    processor.addParameter(Attack);
    processor.addParameter(Decay);
}
void ModEnvParams::saveParameters(juce::XmlElement& xml) {
    xml.setAttribute(Enabled->paramID, Enabled->get());
    xml.setAttribute(TargetType->paramID, TargetType->getIndex());
    xml.setAttribute(TargetFilter->paramID, TargetFilter->getIndex());
    xml.setAttribute(TargetFilterParam->paramID, TargetFilterParam->getIndex());
    xml.setAttribute(Fade->paramID, Fade->getIndex());
    xml.setAttribute(PeakFreq->paramID, (double)PeakFreq->get());
    xml.setAttribute(Wait->paramID, (double)Wait->get());
    xml.setAttribute(Attack->paramID, (double)Attack->get());
    xml.setAttribute(Decay->paramID, (double)Decay->get());
}
void ModEnvParams::loadParameters(juce::XmlElement& xml) {
    *Enabled = xml.getIntAttribute(Enabled->paramID, 0);
    *TargetType = xml.getIntAttribute(TargetType->paramID, 0);
    *TargetFilter = xml.getIntAttribute(TargetFilter->paramID, NUM_FILTER);
    *TargetFilterParam = xml.getIntAttribute(TargetFilterParam->paramID, 0);
    *Fade = xml.getIntAttribute(Fade->paramID, 0);
    *PeakFreq = (float)xml.getDoubleAttribute(PeakFreq->paramID, 0);
    *Wait = (float)xml.getDoubleAttribute(Wait->paramID, 0);
    *Attack = (float)xml.getDoubleAttribute(Attack->paramID, 0.01);
    *Decay = (float)xml.getDoubleAttribute(Decay->paramID, 0.1);
}

//==============================================================================
DelayParams::DelayParams() {
    juce::String idPrefix = "DELAY_";
    juce::String namePrefix = "Delay ";
    Enabled = new juce::AudioParameterBool(idPrefix + "ENABLED", namePrefix + "Enabled", false);
    Type = new juce::AudioParameterChoice(
        idPrefix + "TYPE", namePrefix + "Type", DELAY_TYPE_NAMES, DELAY_TYPE_NAMES.indexOf("Parallel"));
    Sync = new juce::AudioParameterBool(idPrefix + "SYNC", namePrefix + "Sync", false);
    TimeL = new juce::AudioParameterFloat(
        idPrefix + "TIME_L", namePrefix + "TimeL", rangeWithSkewForCentre(0.01f, 1.0f, 0.4f), 0.3f);
    TimeR = new juce::AudioParameterFloat(
        idPrefix + "TIME_R", namePrefix + "TimeR", rangeWithSkewForCentre(0.01f, 1.0f, 0.4f), 0.4f);
    TimeSyncL = new juce::AudioParameterChoice(idPrefix + "TIME_SYNC_L",
                                               namePrefix + "TimeSyncL",
                                               DELAY_TIME_SYNC_NAMES,
                                               DELAY_TIME_SYNC_NAMES.indexOf("1/8"));
    TimeSyncR = new juce::AudioParameterChoice(idPrefix + "TIME_SYNC_R",
                                               namePrefix + "TimeSyncR",
                                               DELAY_TIME_SYNC_NAMES,
                                               DELAY_TIME_SYNC_NAMES.indexOf("1/8"));
    LowFreq = new juce::AudioParameterFloat(
        idPrefix + "LOW_FREQ", namePrefix + "LowFfreq", rangeWithSkewForCentre(10.0f, 20000.0f, 2000.0f), 10.0f);
    HighFreq = new juce::AudioParameterFloat(
        idPrefix + "HIGH_FREQ", namePrefix + "HighFreq", rangeWithSkewForCentre(10.0f, 20000.0f, 2000.0f), 20000.0f);
    Feedback = new juce::AudioParameterFloat(idPrefix + "FEEDBACK", namePrefix + "Feedback", 0.0f, 1.0f, 0.3f);
    Mix = new juce::AudioParameterFloat(idPrefix + "MIX", namePrefix + "Mix", 0.0f, 1.0f, 0.3f);
    freeze();
}
void DelayParams::addAllParameters(juce::AudioProcessor& processor) {
    processor.addParameter(Enabled);
    processor.addParameter(Type);
    processor.addParameter(Sync);
    processor.addParameter(TimeL);
    processor.addParameter(TimeR);
    processor.addParameter(TimeSyncL);
    processor.addParameter(TimeSyncR);
    processor.addParameter(LowFreq);
    processor.addParameter(HighFreq);
    processor.addParameter(Feedback);
    processor.addParameter(Mix);
}
void DelayParams::saveParameters(juce::XmlElement& xml) {
    xml.setAttribute(Enabled->paramID, Enabled->get());
    xml.setAttribute(Type->paramID, Type->getIndex());
    xml.setAttribute(Sync->paramID, Sync->get());
    xml.setAttribute(TimeL->paramID, (double)TimeL->get());
    xml.setAttribute(TimeR->paramID, (double)TimeR->get());
    xml.setAttribute(TimeSyncL->paramID, TimeSyncL->getIndex());
    xml.setAttribute(TimeSyncR->paramID, TimeSyncR->getIndex());
    xml.setAttribute(LowFreq->paramID, (double)LowFreq->get());
    xml.setAttribute(HighFreq->paramID, (double)HighFreq->get());
    xml.setAttribute(Feedback->paramID, (double)Feedback->get());
    xml.setAttribute(Mix->paramID, (double)Mix->get());
}
void DelayParams::loadParameters(juce::XmlElement& xml) {
    *Enabled = xml.getBoolAttribute(Enabled->paramID, false);
    *Type = xml.getIntAttribute(Type->paramID, 0);
    *Sync = xml.getBoolAttribute(Sync->paramID, false);
    *TimeL = (float)xml.getDoubleAttribute(TimeL->paramID, 0.01);
    *TimeR = (float)xml.getDoubleAttribute(TimeR->paramID, 0.01);
    *TimeSyncL = xml.getIntAttribute(TimeSyncL->paramID, 0);
    *TimeSyncR = xml.getIntAttribute(TimeSyncR->paramID, 0);
    *LowFreq = (float)xml.getDoubleAttribute(LowFreq->paramID, 10);
    *HighFreq = (float)xml.getDoubleAttribute(HighFreq->paramID, 20000);
    *Feedback = (float)xml.getDoubleAttribute(Feedback->paramID, 0);
    *Mix = (float)xml.getDoubleAttribute(Mix->paramID, 0);
}

//==============================================================================
MainParams::MainParams(int index)
    : oscParams{OscParams{index, 0},
                OscParams{index, 1},
                OscParams{index, 2},
                OscParams{index, 3},
                OscParams{index, 4},
                OscParams{index, 5},
                OscParams{index, 6},
                OscParams{index, 7}},
      envelopeParams{EnvelopeParams{index, 0},
                     EnvelopeParams{index, 1},
                     EnvelopeParams{index, 2},
                     EnvelopeParams{index, 3},
                     EnvelopeParams{index, 4},
                     EnvelopeParams{index, 5},
                     EnvelopeParams{index, 6},
                     EnvelopeParams{index, 7}} {}
void MainParams::addAllParameters(juce::AudioProcessor& processor) {
    for (auto& params : envelopeParams) {
        params.addAllParameters(processor);
    }
    for (auto& params : oscParams) {
        params.addAllParameters(processor);
    }
}
void MainParams::saveParameters(juce::XmlElement& xml) {
    for (auto& param : envelopeParams) {
        param.saveParameters(xml);
    }
    for (auto& param : oscParams) {
        param.saveParameters(xml);
    }
}
void MainParams::loadParameters(juce::XmlElement& xml) {
    for (auto& param : envelopeParams) {
        param.loadParameters(xml);
    }
    for (auto& param : oscParams) {
        param.loadParameters(xml);
    }
}

//==============================================================================
AllParams::AllParams()
    : globalParams{},
      voiceParams{},
      mainParams{MainParams{0}, MainParams{1}},
      filterParams{FilterParams{0}, FilterParams{1}},
      modEnvParams{ModEnvParams{0}, ModEnvParams{1}, ModEnvParams{2}},
      delayParams{},
      masterParams{} {}
void AllParams::addAllParameters(juce::AudioProcessor& processor) {
    globalParams.addAllParameters(processor);
    voiceParams.addAllParameters(processor);
    for (auto& params : mainParams) {
        params.addAllParameters(processor);
    }
    for (auto& params : filterParams) {
        params.addAllParameters(processor);
    }
    for (auto& params : modEnvParams) {
        params.addAllParameters(processor);
    }
    delayParams.addAllParameters(processor);
    masterParams.addAllParameters(processor);
}
void AllParams::saveParameters(juce::XmlElement& xml) {
    globalParams.saveParameters(xml);
    voiceParams.saveParameters(xml);
    for (auto& param : mainParams) {
        param.saveParameters(xml);
    }
    for (auto& param : filterParams) {
        param.saveParameters(xml);
    }
    for (auto& param : modEnvParams) {
        param.saveParameters(xml);
    }
    delayParams.saveParameters(xml);
    masterParams.saveParameters(xml);
}
void AllParams::loadParameters(juce::XmlElement& xml) {
    globalParams.loadParameters(xml);
    voiceParams.loadParameters(xml);
    for (auto& param : mainParams) {
        param.loadParameters(xml);
    }
    for (auto& param : filterParams) {
        param.loadParameters(xml);
    }
    for (auto& param : modEnvParams) {
        param.loadParameters(xml);
    }
    delayParams.loadParameters(xml);
    masterParams.loadParameters(xml);
}
void AllParams::saveParametersToClipboard(juce::XmlElement& xml) {
    // TODO
    // mainParams.saveParameters(xml);
}
void AllParams::loadParametersFromClipboard(juce::XmlElement& xml) {
    // TODO
    // mainParams.loadParameters(xml);
}