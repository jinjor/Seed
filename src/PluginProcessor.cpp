#include "PluginProcessor.h"

#include "Params.h"
#include "PluginEditor.h"
#include "Voice.h"

//==============================================================================
SeedAudioProcessor::SeedAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
                         )
#endif
      ,
      allParams{},
      buffer{2, 0},
      synth(&currentPositionInfo, buffer, allParams) {
    allParams.addAllParameters(*this);
}

SeedAudioProcessor::~SeedAudioProcessor() { DBG("SeedAudioProcessor's destructor called."); }

//==============================================================================
const juce::String SeedAudioProcessor::getName() const { return JucePlugin_Name; }

bool SeedAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool SeedAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool SeedAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double SeedAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int SeedAudioProcessor::getNumPrograms() { return 2; }

int SeedAudioProcessor::getCurrentProgram() { return currentProgram; }

void SeedAudioProcessor::setCurrentProgram(int index) { currentProgram = index; }

const juce::String SeedAudioProcessor::getProgramName(int index) {
    switch (index) {
        case 0:
            return "program 0";
        case 1:
            return "program 1";
        default:
            return "";
    }
}

void SeedAudioProcessor::changeProgramName(int index, const juce::String& newName) {}

//==============================================================================
void SeedAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    std::cout << "prepareToPlay" << std::endl;
    std::cout << "sampleRate: " << sampleRate << std::endl;
    std::cout << "totalNumInputChannels: " << getTotalNumInputChannels() << std::endl;
    std::cout << "totalNumOutputChannels: " << getTotalNumOutputChannels() << std::endl;
    synth.setCurrentPlaybackSampleRate(sampleRate);
}

void SeedAudioProcessor::releaseResources() { std::cout << "releaseResources" << std::endl; }

#ifndef JucePlugin_PreferredChannelConfigurations
bool SeedAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

        // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet()) return false;
#endif

    return true;
#endif
}
#endif

void SeedAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    auto busCount = getBusCount(false);
    buffer.clear();
    if (auto* playHead = getPlayHead()) {
        if (auto positionInfo = playHead->getPosition()) {
            currentPositionInfo.bpm = *positionInfo->getBpm();
        }
    }
    int numVoices = 64;
    if (synth.getNumVoices() != numVoices) {
        synth.clearVoices();
        for (auto i = 0; i < numVoices; ++i) {
            synth.addVoice(new SeedVoice(&currentPositionInfo, buffer, allParams));
        }
    }
    auto numSamples = buffer.getNumSamples();

    keyboardState.processNextMidiBuffer(midiMessages, 0, numSamples, true);
    double startMillis = juce::Time::getMillisecondCounterHiRes();
    synth.renderNextBlock(buffer, midiMessages, 0, numSamples);  // don't upcast
    double endMillis = juce::Time::getMillisecondCounterHiRes();
    timeConsumptionState.push(getSampleRate(), numSamples, (endMillis - startMillis) / 1000);

    polyphony = 0;
    for (auto i = 0; i < synth.getNumVoices(); ++i) {
        if (synth.getVoice(i)->isVoiceActive()) {
            polyphony++;
        }
    }
    latestDataProvider.push(buffer);

    midiMessages.clear();
#if JUCE_DEBUG
    // auto* leftIn = buffer.getReadPointer(0);
    // auto* rightIn = buffer.getReadPointer(1);
    // for (int i = 0; i < buffer.getNumSamples(); ++i) {
    //     jassert(leftIn[i] >= -1);
    //     jassert(leftIn[i] <= 1);
    //     jassert(rightIn[i] >= -1);
    //     jassert(rightIn[i] <= 1);
    // }
#endif
}

//==============================================================================
bool SeedAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* SeedAudioProcessor::createEditor() { return new SeedAudioProcessorEditor(*this); }

//==============================================================================
void SeedAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    // TODO: ValueTree でもできるらしいので調べる
    juce::XmlElement xml("SeedInstrument");
    allParams.saveParameters(xml);
    copyXmlToBinary(xml, destData);
}
void SeedAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName("SeedInstrument")) {
        allParams.loadParameters(*xml);
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SeedAudioProcessor(); }

//==============================================================================
void SeedAudioProcessor::copyToClipboard() {
    // TODO: ValueTree でもできるらしいので調べる
    juce::XmlElement xml("SeedInstrumentClipboard");
    allParams.saveParametersToClipboard(xml);
    juce::SystemClipboard::copyTextToClipboard(xml.toString());
}
void SeedAudioProcessor::pasteFromClipboard() {
    auto text = juce::SystemClipboard::getTextFromClipboard();
    DBG(text);
    auto xml = juce::parseXML(text);
    if (xml && xml->hasTagName("SeedInstrumentClipboard")) {
        allParams.loadParametersFromClipboard(*xml);
    }
}