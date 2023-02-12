#include <JuceHeader.h>
#include <benchmark/benchmark.h>

#include "../src/Params.h"
#include "../src/Voice.h"

static void doStepLoop(benchmark::State& state, AllParams& p) {
    CurrentPositionInfo currentPositionInfo{};
    juce::AudioBuffer<float> buffer{2, 0};

    SeedVoice voice{&currentPositionInfo, buffer, p};
    auto numChannels = 2;
    auto sampleRate = 48000;
    double out[2]{0, 0};

    SeedSound sound = SeedSound();

    p.freeze();
    voice.startNote(60, 1.0, &sound, 8192);
    auto calculatedParams = CalculatedParams{};
    voice.calculateParamsBeforeLoop(calculatedParams);
    voice.applyParamsBeforeLoop(sampleRate, calculatedParams);
    for (auto _ : state) {
        voice.step(out, sampleRate, numChannels, calculatedParams);
    }
}

static void BM_VoiceStep_empty(benchmark::State& state) {
    AllParams p{};
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_empty);

static void BM_VoiceStep_single_sine(benchmark::State& state) {
    AllParams p{};
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_single_sine);

static void BM_VoiceStep_multiple_sine(benchmark::State& state) {
    AllParams p{};
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_multiple_sine);

static void BM_VoiceStep_single_abs_filter(benchmark::State& state) {
    AllParams p{};
    *p.filterParams[0].Enabled = true;
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_single_abs_filter);

static void BM_VoiceStep_single_rel_filter(benchmark::State& state) {
    AllParams p{};
    *p.filterParams[0].Enabled = true;
    *p.filterParams[0].FreqType = FILTER_FREQ_TYPE_NAMES.indexOf("Rel");
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_single_rel_filter);

static void BM_VoiceStep_multiple_abs_filter(benchmark::State& state) {
    AllParams p{};
    *p.filterParams[0].Enabled = true;
    *p.filterParams[1].Enabled = true;
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_multiple_abs_filter);

static void BM_VoiceStep_multiple_rel_filter(benchmark::State& state) {
    AllParams p{};
    *p.filterParams[0].Enabled = true;
    *p.filterParams[0].FreqType = FILTER_FREQ_TYPE_NAMES.indexOf("Rel");
    *p.filterParams[1].Enabled = true;
    *p.filterParams[1].FreqType = FILTER_FREQ_TYPE_NAMES.indexOf("Rel");
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_multiple_rel_filter);

static void BM_VoiceStep_full(benchmark::State& state) {
    AllParams p{};
    *p.filterParams[0].Enabled = true;
    *p.filterParams[1].Enabled = true;
    *p.filterParams[1].FreqType = FILTER_FREQ_TYPE_NAMES.indexOf("Rel");
    *p.modEnvParams[0].Enabled = true;
    *p.modEnvParams[0].TargetType = MODENV_TARGET_TYPE_NAMES.indexOf("Filter");
    *p.modEnvParams[0].TargetFilterParam = MODENV_TARGET_FILTER_PARAM_NAMES.indexOf("Freq");
    *p.modEnvParams[1].Enabled = true;
    *p.modEnvParams[1].TargetType = MODENV_TARGET_TYPE_NAMES.indexOf("Filter");
    *p.modEnvParams[1].TargetFilterParam = MODENV_TARGET_FILTER_PARAM_NAMES.indexOf("Freq");
    *p.delayParams.Enabled = true;
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_full);

static void BM_DelayStep(benchmark::State& state) {
    auto numChannels = 2;
    auto sampleRate = 48000;
    auto bpm = 120;
    StereoDelay stereoDelay{};

    stereoDelay.setParams(sampleRate,
                          bpm,
                          DELAY_TYPE::Parallel,
                          false,  // Sync
                          0.3,    // TimeL
                          0.3,    // TimeR
                          0.125,  // TimeSyncL
                          0.125,  // TimeSyncR
                          100,    // LowFreq
                          4000,   // HighFreq
                          0.3,    // Feedback
                          0.3);   // Mix
    juce::Random whiteNoise;
    for (auto _ : state) {
        double s = whiteNoise.nextDouble();
        double sample[2]{s, s};
        stereoDelay.step(sample);
    }
}
BENCHMARK(BM_DelayStep);

BENCHMARK_MAIN();