#include "PluginProcessor.h"

#include "PluginEditor.h"

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
{
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

    auto numSamples = buffer.getNumSamples();

    keyboardState.processNextMidiBuffer(midiMessages, 0, numSamples, true);
    latestDataProvider.push(buffer);
    recorder.push(buffer, getSampleRate());

    midiMessages.clear();
}

//==============================================================================
bool SeedAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* SeedAudioProcessor::createEditor() { return new SeedAudioProcessorEditor(*this); }

//==============================================================================
void SeedAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    juce::XmlElement xml("SeedAnalyser");
    for (int i = 0; i < NUM_ENTRIES; i++) {
        auto& entry = recorder.entries[i];
        allParams.entryParams[i].saveParameters(xml);
        xml.setAttribute("E" + juce::String(i) + "_DATA_L", juce::Base64::toBase64(&entry.dataL, DATA_SIZE));
        xml.setAttribute("E" + juce::String(i) + "_DATA_R", juce::Base64::toBase64(&entry.dataR, DATA_SIZE));
    }
    copyXmlToBinary(xml, destData);
}
void SeedAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName("SeedAnalyser")) {
        for (int i = 0; i < NUM_ENTRIES; i++) {
            auto& entry = recorder.entries[i];
            allParams.entryParams[i].loadParameters(*xml);
            MemoryOutputStream outL{entry.dataL, DATA_SIZE};
            MemoryOutputStream outR{entry.dataR, DATA_SIZE};
            juce::Base64::convertFromBase64(outL, xml->getStringAttribute("E" + juce::String(i) + "_DATA_L", ""));
            juce::Base64::convertFromBase64(outR, xml->getStringAttribute("E" + juce::String(i) + "_DATA_R", ""));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SeedAudioProcessor(); }