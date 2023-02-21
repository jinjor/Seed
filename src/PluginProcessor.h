#pragma once

#include <JuceHeader.h>

#include "Params.h"

//==============================================================================
class TimeConsumptionState {
public:
    float currentTimeConsumptionRate = 0.0f;

    TimeConsumptionState(){};
    ~TimeConsumptionState(){};
    void push(double sampleRate, int numSamples, double seconds) {
        if (currentSampleRate != sampleRate) {
            totalNumSamples = 0;
            totalSeconds = 0.0f;
        }
        currentSampleRate = sampleRate;
        totalNumSamples += numSamples;
        totalSeconds += seconds;
        if (totalNumSamples > currentSampleRate) {
            auto timeLimitPerSample = 1.0 / currentSampleRate;
            currentTimeConsumptionRate = totalSeconds / totalNumSamples / timeLimitPerSample;
            totalNumSamples = 0;
            totalSeconds = 0.0f;
        }
    }

private:
    double currentSampleRate = 0.0;
    int totalNumSamples = 0;
    double totalSeconds = 0.0;
};

//==============================================================================
class LatestDataProvider {
public:
    class Consumer {
    public:
        float *destinationL;
        float *destinationR;
        int numSamples = 0;
        bool ready = false;
    };
    enum { numSamples = 2048 };
    std::vector<Consumer *> consumers{};

    LatestDataProvider(){};
    ~LatestDataProvider(){};
    void addConsumer(Consumer *c) {
        std::lock_guard<std::mutex> lock(mtx);
        jassert(c->numSamples <= numSamples);
        consumers.push_back(c);
    }
    void removeConsumer(Consumer *c) {
        std::lock_guard<std::mutex> lock(mtx);
        consumers.erase(remove(consumers.begin(), consumers.end(), c), consumers.end());
    }
    void push(juce::AudioBuffer<float> &buffer) {
        if (buffer.getNumChannels() <= 0) {
            return;
        }
        std::lock_guard<std::mutex> lock(mtx);
        auto *dataL = buffer.getReadPointer(0);
        auto *dataR = buffer.getReadPointer(1);
        for (auto i = 0; i < buffer.getNumSamples(); ++i) {
            fifoL[fifoIndex] = dataL[i];
            fifoR[fifoIndex] = dataR[i];
            fifoIndex++;
            for (auto *consumer : consumers) {
                if (!consumer->ready && fifoIndex >= consumer->numSamples) {
                    memcpy(consumer->destinationL,
                           fifoL + fifoIndex - consumer->numSamples,
                           sizeof(float) * consumer->numSamples);
                    memcpy(consumer->destinationR,
                           fifoR + fifoIndex - consumer->numSamples,
                           sizeof(float) * consumer->numSamples);
                    consumer->ready = true;
                }
            }
            if (fifoIndex >= numSamples) {
                fifoIndex = 0;
            }
        }
    }

private:
    float fifoL[numSamples]{};
    float fifoR[numSamples]{};
    int fifoIndex = 0;
    std::mutex mtx;
};

//==============================================================================

namespace {
constexpr int MAX_REC_SAMPLES = 48000 * 4;
constexpr int DATA_SIZE = sizeof(float) * MAX_REC_SAMPLES;
}  // namespace
class Recorder {
public:
    class Entry {
    public:
        Entry(const Entry &) = delete;
        float sampleRate = 48000;
        float dataL[MAX_REC_SAMPLES]{};
        float dataR[MAX_REC_SAMPLES]{};
        int cursor = 0;
        bool recording = false;
    };
    std::array<Entry, NUM_ENTRIES> entries{};

    Recorder(){};
    ~Recorder(){};
    void push(juce::AudioBuffer<float> &buffer, float sampleRate) {
        if (buffer.getNumChannels() <= 0) {
            return;
        }
        std::lock_guard<std::mutex> lock(mtx);
        auto *dataL = buffer.getReadPointer(0);
        auto *dataR = buffer.getReadPointer(1);
        for (auto &entry : entries) {
            entry.sampleRate = sampleRate;
            for (auto i = 0; i < buffer.getNumSamples(); ++i) {
                if (MAX_REC_SAMPLES <= entry.cursor) {
                    entry.recording = false;
                    entry.cursor = 0;
                }
                if (!entry.recording) {
                    break;
                }
                if (entry.cursor == 0 && dataL[i] == 0 && dataR[i] == 0) {
                    continue;
                }
                entry.dataL[entry.cursor] = dataL[i];
                entry.dataR[entry.cursor] = dataR[i];
                entry.cursor++;
            }
        }
    }

private:
    std::mutex mtx;
};

//==============================================================================
class SeedAudioProcessor : public juce::AudioProcessor {
public:
    //==============================================================================
    SeedAudioProcessor();
    ~SeedAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    //==============================================================================
    juce::AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String &newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;

    //==============================================================================
    int currentProgram = 0;
    juce::MidiKeyboardState keyboardState;
    LatestDataProvider latestDataProvider;
    Recorder recorder;
    AllParams allParams{};

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SeedAudioProcessor)
};
