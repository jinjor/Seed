#pragma once

#include <JuceHeader.h>

#include "MathConstants.h"

using namespace math_constants;

namespace {
enum class WAVEFORM { Sine, Saw, Pink, White };

const juce::StringArray TIMBER_NAMES = juce::StringArray("1", "2");

const juce::StringArray OSC_ENV_NAMES = juce::StringArray("1", "2", "3");
const juce::StringArray NOISE_WAVEFORM_NAMES = juce::StringArray("White", "Pink");
const WAVEFORM NOISE_WAVEFORM_VALUES[2] = {WAVEFORM::White, WAVEFORM::Pink};

enum class FILTER_TYPE { Lowpass, Highpass, Bandpass1, Bandpass2, Notch, Peaking, LowShelf, HighShelf };
const juce::StringArray FILTER_TYPE_NAMES =
    juce::StringArray("Lowpass", "Highpass", "Bandpass1", "Bandpass2", "Notch", "Peaking", "LowShelf", "HighShelf");

enum class FILTER_FREQ_TYPE { Absolute, Relative };
const juce::StringArray FILTER_FREQ_TYPE_NAMES = juce::StringArray("Abs", "Rel");

enum class MODENV_TARGET_TYPE { Filter };
const juce::StringArray MODENV_TARGET_TYPE_NAMES = juce::StringArray("Filter", "Dummy");

const juce::StringArray MODENV_TARGET_OSC_NAMES = juce::StringArray("1", "2", "3", "All");
const juce::StringArray MODENV_TARGET_FILTER_NAMES = juce::StringArray("1", "2", "All");

enum class MODENV_TARGET_FILTER_PARAM { Freq, Q };
const juce::StringArray MODENV_TARGET_FILTER_PARAM_NAMES = juce::StringArray("Freq", "Q");

enum class MODENV_FADE { In, Out };
const juce::StringArray MODENV_FADE_NAMES = juce::StringArray("In", "Out");

enum class DELAY_TYPE { Parallel, PingPong };
const juce::StringArray DELAY_TYPE_NAMES = juce::StringArray("Parallel", "Ping-Pong");

const juce::StringArray DELAY_TIME_SYNC_NAMES = juce::StringArray("1/1",
                                                                  "1/2",
                                                                  "1/4",
                                                                  "1/8",
                                                                  "1/16",
                                                                  "1/32",
                                                                  "1/1 T",
                                                                  "1/2 T",
                                                                  "1/4 T",
                                                                  "1/8 T",
                                                                  "1/16 T",
                                                                  "1/32 T",
                                                                  "1/1 D",
                                                                  "1/2 D",
                                                                  "1/4 D",
                                                                  "1/8 D",
                                                                  "1/16 D",
                                                                  "1/32 D");
const double DELAY_TIME_SYNC_VALUES[18] = {1.0,
                                           0.5,
                                           0.25,
                                           0.125,
                                           0.0625,
                                           0.03125,
                                           1.0 * 2 / 3,
                                           0.5 * 2 / 3,
                                           0.25 * 2 / 3,
                                           0.125 * 2 / 3,
                                           0.0625 * 2 / 3,
                                           0.03125 * 2 / 3,
                                           1.0 * 3 / 2,
                                           0.5 * 3 / 2,
                                           0.25 * 3 / 2,
                                           0.125 * 3 / 2,
                                           0.0625 * 3 / 2,
                                           0.03125 * 3 / 2};

}  // namespace

//==============================================================================
class Wavetable {
public:
    const int *lookup = reinterpret_cast<const int *>(BinaryData::lookup);
    const float *sine = reinterpret_cast<const float *>(BinaryData::sine);
    const float *saw = reinterpret_cast<const float *>(BinaryData::saw);  // 128 variations
    Wavetable(){};
    ~Wavetable(){};
    Wavetable(const Wavetable &) = delete;
    double getSineValue(double normalizedAngle) {
        normalizedAngle = std::fmod(normalizedAngle, 1.0);
        return getValue(sine, normalizedAngle);
    }
    double getSawDownValue(double freq, double normalizedAngle) {
        normalizedAngle = std::fmod(normalizedAngle, 1.0);
        const float *partial = getPartial(saw, freq);
        return getValue(partial, normalizedAngle);
    }

private:
    double getValue(const float *partial, float normalizedAngle) {
        float indexFloat = normalizedAngle * 4095;
        int index = indexFloat;
        float fragment = indexFloat - index;
        return partial[index] * (1 - fragment) + partial[index + 1] * fragment;
    }
    const float *getPartial(const float *data, double freq) {
        int partialIndex = lookup[freq >= 22000 ? 21999 : (int)freq];
        return &data[partialIndex * 4096];
    }
};

//==============================================================================
enum class TRANSITION_TYPE {
    NONE = 0,
    LINEAR,
    EXPONENTIAL,
    EXPONENTIAL2,
};
class TransitiveValue {
public:
    double value = 0;
    TransitiveValue(){};
    ~TransitiveValue(){};
    TransitiveValue(const TransitiveValue &) = delete;
    void init(double value) {
        this->value = value;
        type = TRANSITION_TYPE::NONE;
        targetValue = value;
        stepAmount1 = 0;
        stepAmount2 = 0;
        steps = 0;
    }
    void linear(double duration, double targetValue, double sampleRate) {
        this->targetValue = targetValue;
        if (duration == 0) {
            end();
            return;
        }
        type = TRANSITION_TYPE::LINEAR;
        stepAmount1 = (targetValue - value) / sampleRate / duration;
        stepAmount2 = 0;
        steps = sampleRate * duration;
    }
    void exponentialFinite(double duration, double targetValue, double curveFactor, double sampleRate) {
        if (curveFactor == 0.5) {
            return linear(duration, targetValue, sampleRate);
        }
        jassert(curveFactor > 0);
        jassert(curveFactor < 1);
        this->targetValue = targetValue;
        if (duration == 0) {
            end();
            return;
        }
        type = TRANSITION_TYPE::EXPONENTIAL2;
        auto N = duration * sampleRate;
        auto aN = std::pow((1 - curveFactor) / curveFactor, 2);
        auto a = std::pow(aN, 1.0 / N);
        auto b = (1 - a) / (1 - aN) * (targetValue - value * aN);
        stepAmount1 = a;
        stepAmount2 = b;
        steps = N;
    }
    void exponentialInfinite(double duration, double targetValue, double sampleRate) {
        this->targetValue = targetValue;
        if (duration == 0) {
            end();
            return;
        }
        type = TRANSITION_TYPE::EXPONENTIAL;
        stepAmount1 = std::pow(0.37, 1.0 / sampleRate / duration);  // 63% 減衰する時間とする
        stepAmount2 = 0;
        steps = std::log(0.005) / std::log(stepAmount1);
    }
    bool step() {
        switch (type) {
            case TRANSITION_TYPE::LINEAR: {
                value += stepAmount1;
                if (steps <= 0) {
                    end();
                    return true;
                }
                steps--;
                break;
            }
            case TRANSITION_TYPE::EXPONENTIAL: {
                value = targetValue - (targetValue - value) * stepAmount1;
                if (steps <= 0) {
                    end();
                    return true;
                }
                steps--;
                break;
            }
            case TRANSITION_TYPE::EXPONENTIAL2: {
                value = value * stepAmount1 + stepAmount2;
                if (steps <= 0) {
                    end();
                    return true;
                }
                steps--;
                break;
            }
            case TRANSITION_TYPE::NONE:
                return true;
        }
        return false;
    }
    void end() {
        type = TRANSITION_TYPE::NONE;
        value = targetValue;
        stepAmount1 = 0;
        stepAmount2 = 0;
        steps = 0;
    }

private:
    TRANSITION_TYPE type = TRANSITION_TYPE::NONE;
    double targetValue = 0;
    double stepAmount1 = 0;
    double stepAmount2 = 0;
    int steps = 0;
    double endThreshold = 0;
    double setTargetAtTime(double initialValue, double targetValue, double pos);
};
//==============================================================================
/*
  p +     x---x
    |    /     \
    |   /       \
  s +  /         x------x
    | /                  \
    |/                    \
  b +-----+---+--+------+---
    |a    |h  |d |      |r |
*/
enum class ADSR_PHASE {
    WAIT = 0,
    ATTACK,
    DECAY,
    HOLD,
    SUSTAIN,
    RELEASE,
};
static constexpr int ADSR_BASE = 0;
static constexpr int ADSR_PEAK = 1;
static constexpr double ADSR_STOP_THRESHOLD = 0.005;
class Adsr {
public:
    Adsr(){};
    ~Adsr(){};
    Adsr(const Adsr &) = delete;
    double getValue() { return tvalue.value; }
    bool isActive() { return phase != ADSR_PHASE::WAIT; }
    bool isReleasing() { return phase == ADSR_PHASE::RELEASE; }
    void setParams(double curve, double a, double h, double d, double s, double r) {
        attackCurve = curve;
        attack = a;
        hold = h;
        decay = d;
        sustain = s;
        release = r;
    }
    void doAttack(double sampleRate) {
        phase = ADSR_PHASE::ATTACK;
        tvalue.exponentialFinite(attack, ADSR_PEAK, attackCurve, sampleRate);
    }
    void doRelease(double sampleRate) {
        phase = ADSR_PHASE::RELEASE;
        tvalue.exponentialInfinite(release, ADSR_BASE, sampleRate);
    }
    void doRelease(double sampleRate, double duration) {
        phase = ADSR_PHASE::RELEASE;
        tvalue.exponentialInfinite(duration, ADSR_BASE, sampleRate);
    }
    void forceStop() {
        tvalue.init(ADSR_BASE);
        phase = ADSR_PHASE::WAIT;
    }
    void step(double sampleRate) {
        // TODO: 途中でパラメータを変えた場合にアサーションに引っかかる
        switch (phase) {
            case ADSR_PHASE::ATTACK: {
                if (tvalue.step()) {
                    phase = ADSR_PHASE::HOLD;
                    tvalue.linear(hold, peak, sampleRate);
                }
                //                jassert(tvalue.value <= peak);
                break;
            }
            case ADSR_PHASE::HOLD: {
                if (tvalue.step()) {
                    phase = ADSR_PHASE::DECAY;
                    tvalue.exponentialInfinite(decay, sustain, sampleRate);
                }
                //                jassert(tvalue.value == peak);
                if (tvalue.value < ADSR_STOP_THRESHOLD) {
                    forceStop();
                }
                break;
            }
            case ADSR_PHASE::DECAY: {
                if (tvalue.value < ADSR_STOP_THRESHOLD) {
                    forceStop();
                    break;
                }
                if (tvalue.step()) {
                    phase = ADSR_PHASE::SUSTAIN;
                }
                //                jassert(tvalue.value >= sustain);
                break;
            }
            case ADSR_PHASE::SUSTAIN: {
                if (tvalue.value < ADSR_STOP_THRESHOLD) {
                    forceStop();
                    break;
                }
                //                jassert(tvalue.value == sustain);
                break;
            }
            case ADSR_PHASE::RELEASE: {
                if (tvalue.value < ADSR_STOP_THRESHOLD) {
                    forceStop();
                    break;
                }
                if (tvalue.step()) {
                    forceStop();
                }
                //                jassert(tvalue.value >= base);
                break;
            }
            case ADSR_PHASE::WAIT: {
                //                jassert(tvalue.value == base);
                break;
            }
        }
    }

private:
    double attackCurve;  // 0-1
    double attack;       // ms
    double hold;         // ms
    double decay;        // ms
    double sustain;      // 0-1
    double release;      // ms
    double base = 0;     // 0-1
    double peak = 1;     // 0-1
    ADSR_PHASE phase = phase = ADSR_PHASE::WAIT;
    TransitiveValue tvalue;
};

//==============================================================================
namespace {
const int NUM_FEEDFORWARD = 3;
const int NUM_FEEDBACK = 2;
const int NUM_PAST = 2;
}  // namespace
class Filter {
public:
    Filter() {}
    ~Filter() { DBG("Filter's destructor called."); }
    Filter(const Filter &) = delete;
    void initializePastData() { past[0][0] = past[0][1] = past[1][0] = past[1][1] = 0; }
    void setSampleRate(double sampleRate) {
        if (this->sampleRate != sampleRate) {
            reciprocal_sampleRate = 1.0 / sampleRate;
        }
        this->sampleRate = sampleRate;
    }
    double step(FILTER_TYPE filterType, double freq, double q, double dbGain, int ch, double input) {
        jassert(ch < 2);
        jassert(sampleRate != 0.0);
        jassert(reciprocal_sampleRate > 0.0);
        setParams(filterType, freq, q, dbGain);

        double *p = past[ch];
        // apply b
        for (auto j = 0; j < NUM_FEEDBACK; j++) {
            input -= p[j] * feedback[j];
        }
        // apply a
        auto o = input * feedforward[0];
        for (auto j = 1; j < NUM_FEEDFORWARD; j++) {
            o += p[j - 1] * feedforward[j];
        }
        // unshift f.past
        for (auto j = NUM_PAST - 2; j >= 0; j--) {
            p[j + 1] = p[j];
        }
        p[0] = input;
        return o;
    }

private:
    double feedforward[NUM_FEEDFORWARD];
    double feedback[NUM_FEEDBACK];
    double past[2][NUM_PAST]{};
    double sampleRate = 0.0;
    double reciprocal_sampleRate = -1;
    FILTER_TYPE currentFilterType = FILTER_TYPE::Lowpass;
    double currentFreq = 0.0;
    double currentQ = 0.0;
    double currentDbGain = 0.0;
    void setParams(FILTER_TYPE filterType, double freq, double q, double dbGain) {
        freq = std::min(sampleRate * 0.5 - 10, freq);
        if (filterType == currentFilterType && freq == currentFreq && q == currentQ && dbGain == currentDbGain) {
            return;
        }
        switch (filterType) {
            case FILTER_TYPE::Lowpass: {
                setLowpassParams(freq, q);
                break;
            }
            case FILTER_TYPE::Highpass: {
                setHighpassParams(freq, q);
                break;
            }
            case FILTER_TYPE::Bandpass1: {
                setBandpass1Params(freq, q);
                break;
            }
            case FILTER_TYPE::Bandpass2: {
                setBandpass2Params(freq, q);
                break;
            }
            case FILTER_TYPE::Notch: {
                setNotchParams(freq, q);
                break;
            }
            case FILTER_TYPE::Peaking: {
                setPeakingParams(freq, q, dbGain);
                break;
            }
            case FILTER_TYPE::LowShelf: {
                setLowShelfParams(freq, q, dbGain);
                break;
            }
            case FILTER_TYPE::HighShelf: {
                setHighShelfParams(freq, q, dbGain);
                break;
            }
        }
        currentFilterType = filterType;
        currentFreq = freq;
        currentQ = q;
        currentDbGain = dbGain;
    }
    void setLowpassParams(double freq, double q) {
        // from RBJ's cookbook
        auto fc = freq * reciprocal_sampleRate;
        auto w0 = TWO_PI * fc;
        auto cosw0 = std::cos(w0);
        auto alpha = std::sin(w0) / (2 * q);
        auto b0 = (1 - cosw0) * 0.5;
        auto b1 = (1 - cosw0);
        auto b2 = (1 - cosw0) * 0.5;
        auto a0 = 1 + alpha;
        auto a1 = -2 * cosw0;
        auto a2 = 1 - alpha;
        auto _a0 = 1.0 / a0;
        feedforward[0] = b0 * _a0;
        feedforward[1] = b1 * _a0;
        feedforward[2] = b2 * _a0;
        feedback[0] = a1 * _a0;
        feedback[1] = a2 * _a0;
    }
    void setHighpassParams(double freq, double q) {
        // from RBJ's cookbook
        auto fc = freq * reciprocal_sampleRate;
        auto w0 = TWO_PI * fc;
        auto cosw0 = std::cos(w0);
        auto alpha = std::sin(w0) / (2 * q);
        auto b0 = (1 + cosw0) * 0.5;
        auto b1 = -(1 + cosw0);
        auto b2 = (1 + cosw0) * 0.5;
        auto a0 = 1 + alpha;
        auto a1 = -2 * cosw0;
        auto a2 = 1 - alpha;
        auto _a0 = 1.0 / a0;
        feedforward[0] = b0 * _a0;
        feedforward[1] = b1 * _a0;
        feedforward[2] = b2 * _a0;
        feedback[0] = a1 * _a0;
        feedback[1] = a2 * _a0;
    }
    void setBandpass1Params(double freq, double q) {
        // from RBJ's cookbook
        auto fc = freq * reciprocal_sampleRate;
        auto w0 = TWO_PI * fc;
        auto sinw0 = std::sin(w0);
        auto alpha = sinw0 / (2 * q);
        auto b0 = sinw0 * 0.5;
        auto b1 = 0.0;
        auto b2 = -sinw0 * 0.5;
        auto a0 = 1 + alpha;
        auto a1 = -2 * std::cos(w0);
        auto a2 = 1 - alpha;
        auto _a0 = 1.0 / a0;
        feedforward[0] = b0 * _a0;
        feedforward[1] = b1 * _a0;
        feedforward[2] = b2 * _a0;
        feedback[0] = a1 * _a0;
        feedback[1] = a2 * _a0;
    }
    void setBandpass2Params(double freq, double q) {
        // from RBJ's cookbook
        auto fc = freq * reciprocal_sampleRate;
        auto w0 = TWO_PI * fc;
        auto alpha = std::sin(w0) / (2 * q);
        auto b0 = alpha;
        auto b1 = 0.0;
        auto b2 = -alpha;
        auto a0 = 1 + alpha;
        auto a1 = -2 * std::cos(w0);
        auto a2 = 1 - alpha;
        auto _a0 = 1.0 / a0;
        feedforward[0] = b0 * _a0;
        feedforward[1] = b1 * _a0;
        feedforward[2] = b2 * _a0;
        feedback[0] = a1 * _a0;
        feedback[1] = a2 * _a0;
    }
    void setNotchParams(double freq, double q) {
        // from RBJ's cookbook
        auto fc = freq * reciprocal_sampleRate;
        auto w0 = TWO_PI * fc;
        auto cosw0 = std::cos(w0);
        auto alpha = std::sin(w0) / (2 * q);
        auto b0 = 1.0;
        auto b1 = -2 * cosw0;
        auto b2 = 1.0;
        auto a0 = 1 + alpha;
        auto a1 = -2 * cosw0;
        auto a2 = 1 - alpha;
        auto _a0 = 1.0 / a0;
        feedforward[0] = b0 * _a0;
        feedforward[1] = b1 * _a0;
        feedforward[2] = b2 * _a0;
        feedback[0] = a1 * _a0;
        feedback[1] = a2 * _a0;
    }
    void setAllPassParams(double freq, double q) {
        // from RBJ's cookbook
        auto fc = freq * reciprocal_sampleRate;
        auto w0 = TWO_PI * fc;
        auto cosw0 = std::cos(w0);
        auto alpha = std::sin(w0) / (2 * q);
        auto b0 = 1 - alpha;
        auto b1 = -2 * cosw0;
        auto b2 = 1 + alpha;
        auto a0 = 1 + alpha;
        auto a1 = -2 * cosw0;
        auto a2 = 1 - alpha;
        auto _a0 = 1.0 / a0;
        feedforward[0] = b0 * _a0;
        feedforward[1] = b1 * _a0;
        feedforward[2] = b2 * _a0;
        feedback[0] = a1 * _a0;
        feedback[1] = a2 * _a0;
    }
    void setPeakingParams(double freq, double q, double dbGain) {
        // from RBJ's cookbook
        auto fc = freq * reciprocal_sampleRate;
        auto w0 = TWO_PI * fc;
        auto cosw0 = std::cos(w0);
        auto alpha = std::sin(w0) / (2 * q);
        auto A = std::pow(10, dbGain / 40);
        auto b0 = 1 + alpha * A;
        auto b1 = -2 * cosw0;
        auto b2 = 1 - alpha * A;
        auto a0 = 1 + alpha / A;
        auto a1 = -2 * cosw0;
        auto a2 = 1 - alpha / A;
        auto _a0 = 1.0 / a0;
        feedforward[0] = b0 * _a0;
        feedforward[1] = b1 * _a0;
        feedforward[2] = b2 * _a0;
        feedback[0] = a1 * _a0;
        feedback[1] = a2 * _a0;
    }
    void setLowShelfParams(double freq, double q, double dbGain) {
        // from RBJ's cookbook
        auto fc = freq * reciprocal_sampleRate;
        auto w0 = TWO_PI * fc;
        auto cosw0 = std::cos(w0);
        auto alpha = std::sin(w0) / (2 * q);
        auto A = std::pow(10, dbGain / 40);
        auto sqrtA = std::sqrt(A);
        auto b0 = A * ((A + 1) - (A - 1) * cosw0 + 2 * sqrtA * alpha);
        auto b1 = 2 * A * ((A - 1) - (A + 1) * cosw0);
        auto b2 = A * ((A + 1) - (A - 1) * cosw0 - 2 * sqrtA * alpha);
        auto a0 = (A + 1) + (A - 1) * cosw0 + 2 * sqrtA * alpha;
        auto a1 = -2 * ((A - 1) + (A + 1) * cosw0);
        auto a2 = (A + 1) + (A - 1) * cosw0 - 2 * sqrtA * alpha;
        auto _a0 = 1.0 / a0;
        feedforward[0] = b0 * _a0;
        feedforward[1] = b1 * _a0;
        feedforward[2] = b2 * _a0;
        feedback[0] = a1 * _a0;
        feedback[1] = a2 * _a0;
    }
    void setHighShelfParams(double freq, double q, double dbGain) {
        // from RBJ's cookbook
        auto fc = freq * reciprocal_sampleRate;
        auto w0 = TWO_PI * fc;
        auto cosw0 = std::cos(w0);
        auto alpha = std::sin(w0) / (2 * q);
        auto A = std::pow(10, dbGain / 40);
        auto sqrtA = std::sqrt(A);
        auto b0 = A * ((A + 1) + (A - 1) * cosw0 + 2 * sqrtA * alpha);
        auto b1 = -2 * A * ((A - 1) + (A + 1) * cosw0);
        auto b2 = A * ((A + 1) + (A - 1) * cosw0 - 2 * sqrtA * alpha);
        auto a0 = (A + 1) - (A - 1) * cosw0 + 2 * sqrtA * alpha;
        auto a1 = 2 * ((A - 1) - (A + 1) * cosw0);
        auto a2 = (A + 1) - (A - 1) * cosw0 - 2 * sqrtA * alpha;
        auto _a0 = 1.0 / a0;
        feedforward[0] = b0 * _a0;
        feedforward[1] = b1 * _a0;
        feedforward[2] = b2 * _a0;
        feedback[0] = a1 * _a0;
        feedback[1] = a2 * _a0;
    }
};

//==============================================================================
class Osc {
public:
    Osc() {}
    ~Osc() { DBG("Osc's destructor called."); }
    Osc(const Osc &) = delete;
    void setWaveform(WAVEFORM waveform, bool useWavetable) {
        this->waveform = waveform;
        this->useWavetable = useWavetable;
    }
    void setSampleRate(double sampleRate) { reciprocal_sampleRate = 1.0 / sampleRate; }
    void setNormalizedAngle(double normalizedAngle) { currentNormalizedAngle = normalizedAngle; }
    double step(double freq, double normalizedAngleShift) {
        if (reciprocal_sampleRate <= 0.0) {
            return 0.0;
        }
        auto normalizedAngleDelta = freq * reciprocal_sampleRate;
        currentNormalizedAngle += normalizedAngleDelta;
        if (currentNormalizedAngle > 1.0) {
            currentNormalizedAngle -= 1.0;
        }
        auto normalizedAngle = currentNormalizedAngle + normalizedAngleShift;
        switch (waveform) {
            case WAVEFORM::Sine:
                return std::sin(normalizedAngle * TWO_PI);
                // return wavetable.getSineValue(normalizedAngle);
            case WAVEFORM::Saw:
                if (useWavetable) {
                    return wavetable.getSawDownValue(freq, normalizedAngle);
                } else {
                    normalizedAngle = std::fmod(normalizedAngle, 1.0);
                    return normalizedAngle * -2.0 + 1.0;
                }
            case WAVEFORM::Pink: {
                auto white = (whiteNoise.nextDouble() * 2.0 - 1.0) * 0.5;
                bool eco = true;
                if (eco) {
                    pink[0] = 0.99765 * pink[0] + white * 0.0990460;
                    pink[1] = 0.96300 * pink[1] + white * 0.2965164;
                    pink[2] = 0.57000 * pink[2] + white * 1.0526913;
                    auto value = pink[0] + pink[1] + pink[2] + white * 0.1848;
                    return value;
                } else {
                    pink[0] = 0.99886 * pink[0] + white * 0.0555179;
                    pink[1] = 0.99332 * pink[1] + white * 0.0750759;
                    pink[2] = 0.96900 * pink[2] + white * 0.1538520;
                    pink[3] = 0.86650 * pink[3] + white * 0.3104856;
                    pink[4] = 0.55000 * pink[4] + white * 0.5329522;
                    pink[5] = -0.7616 * pink[5] - white * 0.0168980;
                    auto value = pink[0] + pink[1] + pink[2] + pink[3] + pink[4] + pink[5] + pink[6] + white * 0.5362;
                    pink[6] = white * 0.115926;
                    return value;
                }
            }
            case WAVEFORM::White:
                return whiteNoise.nextDouble() * 2.0 - 1.0;
        }
    }

private:
    Wavetable wavetable;
    double currentNormalizedAngle = 0.0;
    double pink[7]{};
    juce::Random whiteNoise;
    WAVEFORM waveform = WAVEFORM::Sine;
    bool useWavetable = true;
    double reciprocal_sampleRate = -1;
};

//==============================================================================
class MultiOsc {
public:
    MultiOsc(bool saw) { osc.setWaveform(saw ? WAVEFORM::Saw : WAVEFORM::Sine, true); }
    ~MultiOsc() { DBG("MultiOsc's destructor called."); }
    MultiOsc(const MultiOsc &) = delete;
    void setSampleRate(double sampleRate) { osc.setSampleRate(sampleRate); }
    void step(double pan, double freq, double normalizedAngleShift, double gain, double *outout) {
        calcPan(1, pan, 0, 0);
        auto value = osc.step(freq, normalizedAngleShift) * gain;
        for (int ch = 0; ch < 2; ch++) {
            outout[ch] = value * pans[ch];
        }
    }

private:
    Osc osc;
    double pans[2]{std::cos(0.5 * HALF_PI), std::sin(0.5 * HALF_PI)};
    double currentPan = 0.0;
    void calcPan(int numOsc, double pan, double detune, double spread) {
        if (pan != currentPan) {
            jassert(pan >= -1);
            jassert(pan <= 1);
            double theta = (pan + 1) * 0.5 * HALF_PI;
            pans[0] = std::cos(theta);
            pans[1] = std::sin(theta);
        }
        currentPan = pan;
    }
};

//==============================================================================
class StereoDelay {
public:
    StereoDelay() {}
    ~StereoDelay() { DBG("DelayEffect's destructor called."); }
    StereoDelay(const StereoDelay &) = delete;
    void setParams(double sampleRate,
                   std::optional<double> bpm,  // TODO: UI 上 optional であることが分からない
                   DELAY_TYPE type,
                   bool sync,
                   double delayTimeL,
                   double delayTimeR,
                   double delayTimeSyncL,
                   double delayTimeSyncR,
                   double lowFreq,
                   double highFreq,
                   double feedback,
                   double mix) {
        lowpass.setSampleRate(sampleRate);
        highpass.setSampleRate(sampleRate);
        if (sync && bpm) {
            auto timePerBar = 60 * 4 / *bpm;
            delayTimeL = timePerBar * delayTimeSyncL;
            delayTimeR = timePerBar * delayTimeSyncR;
        }
        delayLength[0] = std::max(1.0, std::min(192000.0, sampleRate * delayTimeL));
        delayLength[1] = std::max(1.0, std::min(192000.0, sampleRate * delayTimeR));
        this->type = type;
        this->lowFreq = lowFreq;
        this->highFreq = highFreq;
        this->feedback = feedback;
        this->mix = mix;
    }
    void step(double *input) {
        double tmp[2]{past[0][cursor[0]], past[1][cursor[1]]};  // loop の中で past を上書きするのでここに保持しておく
        for (int ch = 0; ch < 2; ch++) {
            auto dry = input[ch];
            auto wet = past[ch][cursor[ch]];
            input[ch] = dry * (1 - mix) + wet * mix;

            auto newWet = dry + tmp[type == DELAY_TYPE::PingPong ? 1 - ch : ch] * feedback;
            newWet = lowpass.step(FILTER_TYPE::Lowpass, highFreq, 1.0, 0.0, ch, newWet);
            newWet = highpass.step(FILTER_TYPE::Highpass, lowFreq, 1.0, 0.0, ch, newWet);
            past[ch][cursor[ch]] = newWet;
        }
        for (int ch = 0; ch < 2; ch++) {
            cursor[ch]++;
            if (cursor[ch] >= delayLength[ch]) {
                cursor[ch] = 0;
            }
        }
    }

private:
    float past[2][192000]{};
    int delayLength[2]{1, 1};
    int cursor[2]{};
    Filter lowpass;
    Filter highpass;
    DELAY_TYPE type = DELAY_TYPE::Parallel;
    double lowFreq = 10;
    double highFreq = 20000;
    double feedback = 0;
    double mix = 0;
};
