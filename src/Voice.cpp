#include "Voice.h"

#include "Params.h"

//==============================================================================
SeedVoice::SeedVoice(CurrentPositionInfo *currentPositionInfo, juce::AudioBuffer<float> &buffer, AllParams &allParams)
    : perf(juce::PerformanceCounter("voice cycle", 100000)),
      currentPositionInfo(currentPositionInfo),
      buffer(buffer),
      allParams(allParams),
      oscs{MultiOsc(false),
           MultiOsc(false),
           MultiOsc(false),
           MultiOsc(false),
           MultiOsc(false),
           MultiOsc(false),
           MultiOsc(false),
           MultiOsc(true)},
      adsr{Adsr(), Adsr()},
      filters{Filter(), Filter()},
      modEnvs{Adsr(), Adsr(), Adsr()} {}
SeedVoice::~SeedVoice() { DBG("SeedVoice's destructor called."); }
bool SeedVoice::canPlaySound(juce::SynthesiserSound *sound) {
    if (dynamic_cast<SeedSound *>(sound) != nullptr) {
        return true;
    }
    return false;
}
void SeedVoice::startNote(int midiNoteNumber,
                           float velocity,
                           juce::SynthesiserSound *sound,
                           int currentPitchWheelPosition) {
    DBG("startNote() midiNoteNumber:" << midiNoteNumber);
    noteNumberAtStart = midiNoteNumber;
    if (SeedSound *playingSound = dynamic_cast<SeedSound *>(sound)) {
        auto sampleRate = getSampleRate();
        smoothNote.init(midiNoteNumber);
        if (stolen) {
            smoothVelocity.exponentialInfinite(0.01, velocity, sampleRate);
        } else {
            smoothVelocity.init(velocity);
        }
        stolen = false;

        auto fixedSampleRate = sampleRate * CONTROL_RATE;  // for control
        auto calculatedParams = CalculatedParams{};
        calculateParamsBeforeLoop(calculatedParams);

        for (int i = 0; i < NUM_OSC; ++i) {
            if (!stolen) {
                //                oscs[i].setAngle(0.0);
            }
            adsr[i].setParams(calculatedParams.attackCurve[i],
                              calculatedParams.attack[i],
                              0.0,
                              calculatedParams.decay[i],
                              0.0,
                              calculatedParams.release[i]);
            adsr[i].doAttack(fixedSampleRate);
        }
        for (int i = 0; i < NUM_FILTER; ++i) {
            filters[i].initializePastData();
        }
        for (int i = 0; i < NUM_MODENV; ++i) {
            auto &params = allParams.modEnvParams[i];
            if (params.shouldUseHold()) {
                modEnvs[i].setParams(0.5, 0.0, params.wait, params.decay, 0.0, 0.0);
            } else {
                modEnvs[i].setParams(0.5, params.attack, 0.0, params.decay, 0.0, 0.0);
            }
            modEnvs[i].doAttack(fixedSampleRate);
        }
        stepCounter = 0;
    }
}
void SeedVoice::stopNote(float velocity, bool allowTailOff) {
    DBG("stopNote() allowTailOff:" << std::to_string(allowTailOff));
    if (SeedSound *playingSound = dynamic_cast<SeedSound *>(getCurrentlyPlayingSound().get())) {
        if (allowTailOff) {
            auto sampleRate = getSampleRate();
            auto fixedSampleRate = sampleRate * CONTROL_RATE;  // for control
            for (int i = 0; i < NUM_OSC; ++i) {
                if (adsr[i].isReleasing()) {
                    continue;
                }
                adsr[i].doRelease(fixedSampleRate);
            }
        } else {
            stolen = true;
            for (int i = 0; i < NUM_OSC; ++i) {
                oscs[i].setSampleRate(0.0);  // stop
            }
            for (int i = 0; i < NUM_OSC; ++i) {
                adsr[i].forceStop();
            }
            clearCurrentNote();
        }
    }
}
void SeedVoice::mute(double duration) {
    DBG("mute()");
    auto sampleRate = getSampleRate();
    auto fixedSampleRate = sampleRate * CONTROL_RATE;  // for control
    for (int i = 0; i < NUM_OSC; ++i) {
        adsr[i].doRelease(fixedSampleRate, duration);
    }
}
void SeedVoice::renderNextBlock(juce::AudioBuffer<float> &outputBuffer, int startSample, int numSamples) {
    if (SeedSound *playingSound = dynamic_cast<SeedSound *>(getCurrentlyPlayingSound().get())) {
        // DBG("startSample: " + std::to_string(startSample));
        // DBG("numSamples: " + std::to_string(numSamples));
        if (getCurrentlyPlayingNote() == 0) {
            return;
        }
        auto sampleRate = getSampleRate();

        auto calculatedParams = CalculatedParams{};
        calculateParamsBeforeLoop(calculatedParams);
        applyParamsBeforeLoop(sampleRate, calculatedParams);

        int numChannels = outputBuffer.getNumChannels();
        jassert(numChannels <= 2);
        while (--numSamples >= 0) {
            double out[2]{0, 0};
            auto active = step(out, sampleRate, numChannels, calculatedParams);
            for (auto ch = 0; ch < numChannels; ++ch) {
                buffer.addSample(ch, startSample, out[ch]);
            }
            ++startSample;
            if (!active) {
                clearCurrentNote();
                break;
            }
        }
    }
}
void SeedVoice::applyParamsBeforeLoop(double sampleRate, CalculatedParams &params) {
    for (int i = 0; i < NUM_OSC; ++i) {
        oscs[i].setSampleRate(sampleRate);
        adsr[i].setParams(params.attackCurve[i], params.attack[i], 0.0, params.decay[i], 0.0, params.release[i]);
    }
    for (int i = 0; i < NUM_FILTER; ++i) {
        filters[i].setSampleRate(sampleRate);
    }
    for (int i = 0; i < NUM_MODENV; ++i) {
        auto &params = allParams.modEnvParams[i];
        if (params.shouldUseHold()) {
            modEnvs[i].setParams(0.5, 0.0, params.wait, params.decay, 0.0, 0.0);
        } else {
            modEnvs[i].setParams(0.5, params.attack, 0.0, params.decay, 0.0, 0.0);
        }
    }
}
void SeedVoice::calculateParamsBeforeLoop(CalculatedParams &params) {
    int controlPoints[NUM_TIMBRES] = {40, 80};
    auto leftIndex = 0;
    auto rightIndex = NUM_TIMBRES - 1;
    auto leftNote = 0;
    auto rightNote = 127;
    for (int i = 0; i < NUM_TIMBRES; i++) {
        if (noteNumberAtStart <= controlPoints[i]) {
            rightIndex = i;
            rightNote = controlPoints[i];
            break;
        }
    }
    for (int i = NUM_TIMBRES - 1; i >= 0; i--) {
        if (controlPoints[i] < noteNumberAtStart) {
            leftIndex = i;
            leftNote = controlPoints[i];
            break;
        }
    }
    jassert(rightNote - leftNote > 0);
    auto leftRatio = (double)(rightNote - noteNumberAtStart) / (double)(rightNote - leftNote);
    auto rightRatio = 1.0 - leftRatio;
    for (int oscIndex = 0; oscIndex < NUM_OSC; ++oscIndex) {
        auto &leftParams = allParams.mainParams[leftIndex];
        auto &rightParams = allParams.mainParams[rightIndex];
        auto &leftOsc = leftParams.oscParams[oscIndex];
        auto &rightOsc = rightParams.oscParams[oscIndex];
        int leftEnvelopeIndex = 0;
        for (int i = oscIndex; i >= 1; i--) {
            auto &p = leftParams.oscParams[i];
            if (p.newEnvelope) {
                leftEnvelopeIndex = i;
                break;
            }
        }
        int rightEnvelopeIndex = 0;
        for (int i = oscIndex; i >= 1; i--) {
            auto &p = rightParams.oscParams[i];
            if (p.newEnvelope) {
                rightEnvelopeIndex = i;
                break;
            }
        }
        auto &leftEnv = leftParams.envelopeParams[leftEnvelopeIndex];
        auto &rightEnv = rightParams.envelopeParams[rightEnvelopeIndex];
        params.gain[oscIndex] = leftOsc.gain * leftRatio + rightOsc.gain * rightRatio;
        params.attackCurve[oscIndex] = leftEnv.attackCurve * leftRatio + rightEnv.attackCurve * rightRatio;
        params.attack[oscIndex] = leftEnv.attack * leftRatio + rightEnv.attack * rightRatio;
        params.decay[oscIndex] = leftEnv.decay * leftRatio + rightEnv.decay * rightRatio;
        params.release[oscIndex] = leftEnv.release * leftRatio + rightEnv.release * rightRatio;
    }
}
bool SeedVoice::step(double *out, double sampleRate, int numChannels, CalculatedParams &params) {
    smoothNote.step();
    smoothVelocity.step();

    double midiNoteNumber = smoothNote.value + allParams.globalParams.pitch * allParams.voiceParams.pitchBendRange;
    auto baseFreq = getMidiNoteInHertzDouble(midiNoteNumber);

    if (stepCounter == 0) {
        auto fixedSampleRate = sampleRate * CONTROL_RATE;
        for (int i = 0; i < NUM_OSC; ++i) {
            adsr[i].step(fixedSampleRate);
        }
        controlModifiers = Modifiers{};
        updateModifiersByModEnv(controlModifiers, fixedSampleRate);
    }
    stepCounter++;
    if (stepCounter >= CONTROL_INTERVAL) {
        stepCounter = 0;
    }

    auto modifiers = controlModifiers;  // copy;

    bool active = false;
    auto panBase = allParams.masterParams.pan;
    if (allParams.globalParams.pan >= 0) {
        panBase = (1 - panBase) * allParams.globalParams.pan + panBase;
    } else {
        panBase = (1 + panBase) * allParams.globalParams.pan + panBase;
    }

    auto panModAmp = std::min(1.0 - panBase, 1.0 + panBase);
    // ---------------- OSC with Envelope and Filter ----------------
    for (int oscIndex = 0; oscIndex < NUM_OSC; ++oscIndex) {
        if (!adsr[oscIndex].isActive()) {
            continue;
        }
        active = true;
        auto freq = oscIndex == NUM_OSC - 1 ? baseFreq : baseFreq * (oscIndex + 1);
        auto pan = panBase + panModAmp * modifiers.panMod[oscIndex];
        jassert(pan >= -1);
        jassert(pan <= 1);

        double o[2]{0, 0};
        auto sineGain = adsr[oscIndex].getValue() * params.gain[oscIndex];

        oscs[oscIndex].step(pan, freq, 0.0, sineGain, o);

        out[0] += o[0];
        out[1] += o[1];
    }
    for (int filterIndex = 0; filterIndex < NUM_FILTER; ++filterIndex) {
        auto &fp = allParams.filterParams[filterIndex];
        if (!fp.enabled) {
            continue;
        }
        if (true) {
            auto filterType = fp.type;
            double freq;
            if (fp.isFreqAbsoluteFreezed) {
                double noteShift = modifiers.filterOctShift[filterIndex] * 12;
                freq = shiftHertsByNotes(fp.hz, noteShift);
            } else {
                double shiftedNoteNumber = midiNoteNumber + fp.semitone + modifiers.filterOctShift[filterIndex] * 12;
                freq = getMidiNoteInHertzDouble(shiftedNoteNumber);
            }
            auto q = fp.q;
            if (modifiers.filterQExp[filterIndex] != 1.0) {
                q = std::pow(q, modifiers.filterQExp[filterIndex]);
            }
            auto gain = fp.gain;
            for (auto ch = 0; ch < numChannels; ++ch) {
                out[ch] = filters[filterIndex].step(filterType, freq, q, gain, ch, out[ch]);
            }
        }
    }
    auto finalGain = 0.3 * smoothVelocity.value;
    for (auto ch = 0; ch < numChannels; ++ch) {
        out[ch] *= finalGain;
    }
    return active;
    // for (auto ch = 0; ch < numChannels; ++ch) {
    //     outputBuffer.addSample (ch, startSample, out[ch]);
    // }
    // ++startSample;
    // if(!active) {
    //     clearCurrentNote();
    //     break;
    // }
}

void SeedVoice::updateModifiersByModEnv(Modifiers &modifiers, double sampleRate) {
    auto &mainParams = allParams.mainParams[0];  // TODO
    for (int i = 0; i < NUM_MODENV; ++i) {
        auto &params = allParams.modEnvParams[i];
        if (!params.enabled) {
            continue;
        }
        modEnvs[i].step(sampleRate);
        auto modEnvValue = modEnvs[i].getValue();
        switch (params.targetType) {
            case MODENV_TARGET_TYPE::Filter: {
                int targetIndex = params.targetFilter;
                switch (params.targetFilterParam) {
                    case MODENV_TARGET_FILTER_PARAM::Freq: {
                        auto v = params.peakFreq * modEnvValue;
                        if (targetIndex == NUM_FILTER) {
                            for (int filterIndex = 0; filterIndex < NUM_FILTER; ++filterIndex) {
                                modifiers.filterOctShift[filterIndex] += v;
                            }
                        } else {
                            modifiers.filterOctShift[targetIndex] += v;
                        }
                        break;
                    }
                    case MODENV_TARGET_FILTER_PARAM::Q: {
                        auto v = params.fadeIn ? 1 - modEnvValue : modEnvValue;
                        if (targetIndex == NUM_FILTER) {
                            for (int filterIndex = 0; filterIndex < NUM_FILTER; ++filterIndex) {
                                modifiers.filterQExp[filterIndex] *= v;
                            }
                        } else {
                            modifiers.filterQExp[targetIndex] *= v;
                        }
                        break;
                    }
                }
                break;
            }
        }
    }
}
