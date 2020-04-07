#include <benchmark/benchmark.h>
#include <opus.h>
#include <opus_defines.h>
#include <opus_types.h>

#include <iostream>
#include <vector>

#include "test_data_1_channel_16_khz_pcm.h"

constexpr auto BM_UNITS = benchmark::kMillisecond;

constexpr opus_int32 kFs = 16000;
constexpr int kChannels = 1;
constexpr size_t kFrameSizeMS = 20;
constexpr opus_int32 kFrameSizeSamples = kFs / 1000 * kFrameSizeMS;

using namespace std;

auto BM_ENCODE = [](benchmark::State& state, const vector<int16_t>& pcm_data,
                    int complexity) {

    opus_int32 ret = OPUS_OK;
    std::vector<uint8_t> payload(1500);

    OpusEncoder* encoder =
        opus_encoder_create(kFs, kChannels, OPUS_APPLICATION_VOIP, &ret);

    opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(complexity));

    const auto data_length = pcm_data.size();

    for (auto _ : state) {
        auto data_ptr = pcm_data.data();
        size_t offset = 0;
        opus_int32 num_bytes;
        for (; offset < data_length - kFrameSizeSamples + 1;
             offset += kFrameSizeSamples) {
            num_bytes = opus_encode(encoder, data_ptr, kFrameSizeSamples,
                                    payload.data(),
                                    static_cast<opus_int32>(payload.size()));
            if (num_bytes < 0) {
                cout << "Opus encoder failed with: " << num_bytes << '\n';
            }
            benchmark::DoNotOptimize(payload.data());
            std::fill(payload.begin(), payload.end(), static_cast<uint8_t>(0u));
            benchmark::DoNotOptimize(payload.data());
            data_ptr += kFrameSizeSamples;
        }
        if (data_length % offset) {
            num_bytes = opus_encode(
                encoder, data_ptr, static_cast<int>(data_length % offset),
                payload.data(), static_cast<opus_int32>(payload.size()));
            if (num_bytes < 0) {
                cout << "Opus encoder failed with: " << num_bytes << '\n';
            }
        }
    }

    opus_encoder_destroy(encoder);
};

int main(int, char**) {
    vector<int16_t> pcm_data;
    const size_t test_time_seconds = 10;

    pcm_data.resize(test_data::k1_channel_16_kHz_pcm::length);
    pcm_data.assign(test_data::k1_channel_16_kHz_pcm::data,
                    test_data::k1_channel_16_kHz_pcm::data +
                        test_data::k1_channel_16_kHz_pcm::length /
                            (test_data::k1_channel_16_kHz_pcm::seconds /
                             test_time_seconds));

    cout << "Encoding " << test_time_seconds << " [s] of speech.\n";
    benchmark::RegisterBenchmark("Encode_1_channel_16kHz_Complexity_0",
                                 BM_ENCODE, pcm_data, 0)
        ->Unit(BM_UNITS)->Repetitions(64);
#if 0
    benchmark::RegisterBenchmark("Encode_1_channel_16kHz_Complexity_1",
                                 BM_ENCODE, pcm_data, 1)
        ->Unit(BM_UNITS)->Repetitions(16);
    benchmark::RegisterBenchmark("Encode_1_channel_16kHz_Complexity_2",
                                 BM_ENCODE, pcm_data, 2)
        ->Unit(BM_UNITS)->Repetitions(16);
    benchmark::RegisterBenchmark("Encode_1_channel_16kHz_Complexity_3",
                                 BM_ENCODE, pcm_data, 3)
        ->Unit(BM_UNITS)->Repetitions(16);
    benchmark::RegisterBenchmark("Encode_1_channel_16kHz_Complexity_4",
                                 BM_ENCODE, pcm_data, 4)
        ->Unit(BM_UNITS)->Repetitions(16);
    benchmark::RegisterBenchmark("Encode_1_channel_16kHz_Complexity_5",
                                 BM_ENCODE, pcm_data, 5)
        ->Unit(BM_UNITS)->Repetitions(16);
    benchmark::RegisterBenchmark("Encode_1_channel_16kHz_Complexity_6",
                                 BM_ENCODE, pcm_data, 6)
        ->Unit(BM_UNITS)->Repetitions(16);
    benchmark::RegisterBenchmark("Encode_1_channel_16kHz_Complexity_7",
                                 BM_ENCODE, pcm_data, 7)
        ->Unit(BM_UNITS)->Repetitions(16);
    benchmark::RegisterBenchmark("Encode_1_channel_16kHz_Complexity_8",
                                 BM_ENCODE, pcm_data, 8)
        ->Unit(BM_UNITS)->Repetitions(16);
    benchmark::RegisterBenchmark("Encode_1_channel_16kHz_Complexity_9",
                                 BM_ENCODE, pcm_data, 9)
        ->Unit(BM_UNITS)->Repetitions(64);
#endif
    benchmark::RunSpecifiedBenchmarks();
}
