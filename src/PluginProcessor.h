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
    };

    std::array<Entry, NUM_ENTRIES> entries{};
    int getCurrentEntryIndex() {
        std::lock_guard<std::mutex> lock(mtx);
        return currentEntryIndex;
    }
    void setCurrentEntryIndex(int index) {
        std::lock_guard<std::mutex> lock(mtx);
        currentEntryIndex = index;
    }
    bool canOperate() {
        std::lock_guard<std::mutex> lock(mtx);
        return mode == Mode::WAITING;
    }
    void changeIndex(int index) {
        std::lock_guard<std::mutex> lock(mtx);
        currentEntryIndex = index;
        filterEnabled = false;  // TODO: ロジックが微妙
    }
    void play(bool filterEnabled, int n, float lowFreq, float highFreq) {
        std::lock_guard<std::mutex> lock(mtx);
        if (mode != Mode::WAITING) {
            return;
        }
        bool turnedOn = !this->filterEnabled && filterEnabled;
        bool changed = filterN != n || filterLowFreq != lowFreq || filterHighFreq != highFreq;
        this->filterEnabled = filterEnabled;
        filterN = n;
        filterLowFreq = lowFreq;
        filterHighFreq = highFreq;
        if (turnedOn || changed) {
            DBG("begin calculateFilter");
            calculateFilter();
            DBG("end calculateFilter");
        }
        mode = Mode::PLAYING;
        cursor = 0;
    }
    void record() {
        std::lock_guard<std::mutex> lock(mtx);
        if (mode != Mode::WAITING) {
            return;
        }
        mode = Mode::RECORDING;
        cursor = 0;
    }

    Recorder(){};
    ~Recorder(){};
    void push(juce::AudioBuffer<float> &buffer, float sampleRate) {
        if (buffer.getNumChannels() <= 0) {
            return;
        }
        std::lock_guard<std::mutex> lock(mtx);
        auto *readL = buffer.getReadPointer(0);
        auto *readR = buffer.getReadPointer(1);
        auto *writeL = buffer.getWritePointer(0);
        auto *writeR = buffer.getWritePointer(1);
        if (entries.size() <= 0) {
            return;
        }
        auto &entry = entries[currentEntryIndex];
        if (mode == Mode::RECORDING) {
            entry.sampleRate = sampleRate;
            for (auto i = 0; i < buffer.getNumSamples(); ++i) {
                if (MAX_REC_SAMPLES <= cursor) {
                    mode = Mode::WAITING;
                    cursor = 0;
                }
                if (mode == Mode::WAITING) {
                    break;
                }
                if (cursor == 0 && readL[i] == 0 && readR[i] == 0) {
                    continue;
                }
                entry.dataL[cursor] = readL[i];
                entry.dataR[cursor] = readR[i];
                cursor++;
            }
        } else if (mode == Mode::PLAYING) {
            // if (entry.sampleRate != sampleRate) {
            //     continue;
            // }
            auto &targetL = filterEnabled ? filteredL : entry.dataL;
            auto &targetR = filterEnabled ? filteredR : entry.dataR;
            for (auto i = 0; i < buffer.getNumSamples(); ++i) {
                if (MAX_REC_SAMPLES <= cursor) {
                    mode = Mode::WAITING;
                    cursor = 0;
                }
                if (mode == Mode::WAITING) {
                    break;
                }
                writeL[i] += targetL[cursor];
                writeR[i] += targetR[cursor];
                cursor++;
            }
        }
    }

private:
    enum class Mode { WAITING, RECORDING, PLAYING };
    std::mutex mtx;
    int currentEntryIndex = 0;
    int cursor = 0;
    Mode mode = Mode::WAITING;

    bool filterEnabled = false;
    int filterN = 100;
    float filterLowFreq = 20;
    float filterHighFreq = 20000;
    float filteredL[MAX_REC_SAMPLES]{};
    float filteredR[MAX_REC_SAMPLES]{};

    void calculateFilter() {
        auto &entry = entries[currentEntryIndex];
        auto h = std::vector<double>(filterN + 1, 0.0);
        {
            double fc = filterHighFreq / entry.sampleRate;
            double wc = fc * juce::MathConstants<double>::twoPi;
            for (int n = 0; n <= filterN; n++) {
                double n1 = n - (double)filterN / 2;
                double sinc = n1 == 0 ? 1 : std::sin(wc * n1) / (wc * n1);
                double value = 2.0 * fc * sinc;
                // blackman
                // TODO: 一般化して切り出す
                double window = 0.42 + 0.5 * std::cos(2 * juce::MathConstants<double>::pi * n1 / filterN) +
                                0.08 * std::cos(4 * juce::MathConstants<double>::pi * n1 / filterN);
                h[n] += value * window;
            }
        }
        {
            double fc = filterLowFreq / entry.sampleRate;
            double wc = fc * juce::MathConstants<double>::twoPi;
            for (int n = 0; n <= filterN; n++) {
                double n1 = n - (double)filterN / 2;
                double sinc = n1 == 0 ? 1 : std::sin(wc * n1) / (wc * n1);
                double value = 2.0 * fc * sinc;
                double window = 0.42 + 0.5 * std::cos(2 * juce::MathConstants<double>::pi * n1 / filterN) +
                                0.08 * std::cos(4 * juce::MathConstants<double>::pi * n1 / filterN);
                h[n] -= value * window;
            }
        }
        for (int i = 0; i < MAX_REC_SAMPLES; i++) {
            double sumL = 0;
            double sumR = 0;
            for (int n = 0; n <= filterN; n++) {
                double xl = i - n < 0 ? 0 : entry.dataL[i - n];
                sumL += h[n] * xl;
                double xr = i - n < 0 ? 0 : entry.dataR[i - n];
                sumR += h[n] * xr;
            }
            filteredL[i] = sumL;
            filteredR[i] = sumR;
        }
    }
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
