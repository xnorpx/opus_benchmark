#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <numeric>
#include <functional>
#include <vector>
#include <assert.h>

namespace tone {

constexpr float kTONEAMPLITUDE = 0.5;
constexpr float kTONEFREQUENCY = 500;
constexpr double kTWO_PI = 2 * M_PI;

template <typename T>
std::vector<T> SinWave(size_t samples, float sampling_rate, float tone_frequency, float tone_amplitude);

template <>
inline std::vector<float> SinWave<float>(size_t samples, float sampling_rate, float tone_frequency, float tone_amplitude)
{
    assert(-1.f <= tone_amplitude && tone_amplitude <= 1.f);

    std::vector<float> s;
    s.reserve(samples);

    double kTWO_PI = 2 * M_PI;
    double f = tone_frequency / sampling_rate;

    for (auto i = 0; i < samples; i++) {
        s.push_back(tone_amplitude * sinf(static_cast<float>((kTWO_PI * i * f))));
    }
    return s;
}

template <>
inline std::vector<int16_t> SinWave<int16_t>(size_t samples, float sampling_rate, float tone_frequency, float tone_amplitude)
{
    assert(INT_MIN <= tone_amplitude && tone_amplitude <= INT_MAX); // TODO replace with limits

    std::vector<float> tmp = SinWave<float>(samples, sampling_rate, tone_frequency, tone_amplitude);
    std::vector<int16_t> s(samples, 0);

    std::transform(tmp.begin(), tmp.end(), s.begin(),
                   [](float& x) -> int16_t { return (int16_t)(x * std::numeric_limits<int16_t>::max()); });
    return s;
}

inline std::vector<float> CosWave(size_t samples, float sampling_rate, float tone_frequency, float tone_amplitude)
{
    assert(-1.f <= tone_amplitude && tone_amplitude <= 1.f);

    std::vector<float> c;
    c.reserve(samples);

    double f = tone_frequency / sampling_rate;

    for (auto i = 0; i < samples; i++) {
        c.push_back(tone_amplitude * cosf((float)(kTWO_PI * i * f)));
    }
    return c;
}

class ChirpSignal
{

public:

    ChirpSignal()
        : idx_(0), period_(16000), scale_(0.75f), sampling_(16000.0f), amplitude_(10000.0)
    {
    }

    void GetSamples(std::vector<int16_t> &outBuffer, int numSamples)
    {
        float sampleValue = 0.0f;
        outBuffer.clear();
        for (int i = 0; i < numSamples; i++) {
            idx_++;
            idx_ %= period_;

            sampleValue = sin((scale_ * ((float)idx_) * ((float)idx_)) / sampling_) * amplitude_;
            outBuffer.push_back((int16_t)(std::max(std::min(sampleValue, 32767.0f), -32767.0f)));
        }
    }

private:
    int32_t idx_;
    int32_t period_;
    float scale_;
    float sampling_;
    float amplitude_;
};

template <typename T>
std::map<std::string, float> ProjectOnTone(std::vector<T> input, float sampling_rate, float tone_frequency);

template <>
inline std::map<std::string, float> ProjectOnTone<float>(std::vector<float> input, float sampling_rate, float tone_frequency)
{
    int numSamples = (int)input.size();
    std::map<std::string, float> result;

    // generate reference
    std::vector<float> s = SinWave<float>(input.size(), sampling_rate, tone_frequency, 1.f);
    std::vector<float> c = CosWave(input.size(), sampling_rate, tone_frequency, 1.f);

    float ss = std::inner_product(s.begin(), s.end(), s.begin(), 0.f);
    float cc = std::inner_product(c.begin(), c.end(), c.begin(), 0.f);

    float sx = std::inner_product(s.begin(), s.end(), input.begin(), 0.f);
    float cx = std::inner_product(c.begin(), c.end(), input.begin(), 0.f);

    std::vector<float> signal;
    signal.reserve(numSamples);

    for (auto i = 0; i < numSamples; i++) {
        signal.push_back(s[i] * sx / ss + c[i] * cx / cc);
    }

    std::vector<float> noise(numSamples, 0);
    std::transform(signal.begin(), signal.end(), input.begin(), noise.begin(), std::minus<float>());

    float signalPower = std::inner_product(signal.begin(), signal.end(), signal.begin(), 0.f);
    float noisePower = std::inner_product(noise.begin(), noise.end(), noise.begin(), 0.f);

    result["signal"] = signalPower;
    result["noise"] = noisePower;

    if (noisePower != 0.f) {
        result["snr"] = signalPower / noisePower;
    } else {
        result["snr"] = FLT_MAX;
    }

    return result;
}

template <>
inline std::map<std::string, float> ProjectOnTone<int16_t>(std::vector<int16_t> input, float sampling_rate, float tone_frequency)
{
    std::vector<float> tmp(input.size(), 0);

    std::transform(input.begin(), input.end(), tmp.begin(), [](int16_t& x) -> float {
        return static_cast<float>(x) / static_cast<float>(std::numeric_limits<int16_t>::max());
    });

    return ProjectOnTone<float>(tmp, sampling_rate, tone_frequency);
}

}  // namespace tone
