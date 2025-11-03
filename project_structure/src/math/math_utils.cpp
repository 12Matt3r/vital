#include "math_utils.h"
#include <algorithm>

namespace vital {

// Static member initialization
const float MathUtils::sin_table_[SIN_TABLE_SIZE] = {};
unsigned int MathUtils::random_seed_ = 12345;
bool MathUtils::sin_table_initialized_ = false;

float MathUtils::fastAbs(float x) {
    // Use bit trick for absolute value (fast on most hardware)
    return x < 0 ? -x : x;
}

float MathUtils::fastSqrt(float x) {
    // Fast square root approximation
    if (x <= 0.0f) return 0.0f;
    
    // Use bit manipulation for approximation
    int i = 0x5f3759df - (*(int*)&x >> 1);
    float y = *(float*)&i;
    
    // Newton-Raphson refinement
    y = y * (1.5f - (x * 0.5f * y * y));
    y = y * (1.5f - (x * 0.5f * y * y));
    
    return x * y;
}

float MathUtils::fastPow(float x, float y) {
    // Fast power approximation
    return std::exp(y * std::log(x));
}

float MathUtils::fastExp(float x) {
    // Fast exponential approximation
    if (x < -5.0f) return 0.0f;
    if (x > 5.0f) return std::exp(5.0f);
    
    return 1.0f + x + x * x * 0.5f + x * x * x * 0.1666667f;
}

float MathUtils::lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

void MathUtils::lerpVector(const std::vector<float>& a, const std::vector<float>& b,
                          std::vector<float>& result, float t) {
    size_t size = std::min(a.size(), b.size());
    result.resize(size);
    
    for (size_t i = 0; i < size; ++i) {
        result[i] = lerp(a[i], b[i], t);
    }
}

float MathUtils::expLerp(float a, float b, float t, float curve) {
    float exponential_t = std::pow(t, curve);
    return lerp(a, b, exponential_t);
}

float MathUtils::dbToLinear(float db) {
    return std::pow(10.0f, db / 20.0f);
}

float MathUtils::linearToDb(float linear) {
    if (linear <= 0.0f) return -100.0f;
    return 20.0f * std::log10(linear);
}

float MathUtils::noteToFrequency(int note) {
    return 440.0f * std::pow(2.0f, (note - 69) / 12.0f);
}

int MathUtils::frequencyToNote(float frequency) {
    if (frequency <= 0.0f) return 0;
    
    float note = 69 + 12 * std::log2(frequency / 440.0f);
    return static_cast<int>(std::round(note));
}

float MathUtils::noteToSemitoneOffset(int note) {
    return note % 12;
}

float MathUtils::normalize(float value, float min, float max) {
    if (max == min) return 0.0f;
    return (value - min) / (max - min);
}

float MathUtils::denormalize(float normalized, float min, float max) {
    return min + normalized * (max - min);
}

float MathUtils::clamp(float value, float min, float max) {
    return std::max(min, std::min(max, value));
}

float MathUtils::wrap(float value, float min, float max) {
    float range = max - min;
    if (range == 0.0f) return min;
    
    float wrapped = value - min;
    wrapped = std::fmod(wrapped, range);
    if (wrapped < 0) wrapped += range;
    return wrapped + min;
}

float MathUtils::fastSin(float x) {
    // Use lookup table with linear interpolation
    if (!sin_table_initialized_) {
        initializeSinTable();
    }
    
    // Normalize to [0, 2π]
    x = wrap(x, 0.0f, TWO_PI);
    
    // Convert to table index
    float table_index = x * SIN_TABLE_SIZE / TWO_PI;
    int index = static_cast<int>(table_index);
    float fraction = table_index - index;
    
    // Linear interpolation
    float sin1 = sin_table_[index];
    float sin2 = sin_table_[(index + 1) % SIN_TABLE_SIZE];
    
    return sin1 + (sin2 - sin1) * fraction;
}

float MathUtils::fastCos(float x) {
    return fastSin(x + HALF_PI);
}

void MathUtils::sincos(float x, float& sin_result, float& cos_result) {
    sin_result = fastSin(x);
    cos_result = fastSin(x + HALF_PI);
}

void MathUtils::hannWindow(std::vector<float>& window) {
    size_t n = window.size();
    if (n <= 1) return;
    
    for (size_t i = 0; i < n; ++i) {
        window[i] = 0.5f * (1.0f - std::cos(2.0f * PI * i / (n - 1)));
    }
}

void MathUtils::hammingWindow(std::vector<float>& window) {
    size_t n = window.size();
    if (n <= 1) return;
    
    for (size_t i = 0; i < n; ++i) {
        window[i] = 0.54f - 0.46f * std::cos(2.0f * PI * i / (n - 1));
    }
}

void MathUtils::blackmanWindow(std::vector<float>& window) {
    size_t n = window.size();
    if (n <= 1) return;
    
    for (size_t i = 0; i < n; ++i) {
        float term1 = 0.08f * std::cos(4.0f * PI * i / (n - 1));
        float term2 = 0.42f * std::cos(2.0f * PI * i / (n - 1));
        window[i] = 0.42f - term2 - term1;
    }
}

void MathUtils::kaiserWindow(std::vector<float>& window, float beta) {
    size_t n = window.size();
    if (n <= 1) return;
    
    // Modified Bessel function of first kind
    auto modifiedBessel = [](float x) -> float {
        float sum = 1.0f;
        float term = 1.0f;
        int k = 1;
        
        while (term > 1e-6f) {
            term *= (x * x) / (4.0f * k * k);
            sum += term;
            ++k;
        }
        
        return sum;
    };
    
    float i0_beta = modifiedBessel(beta);
    
    for (size_t i = 0; i < n; ++i) {
        float normalized = 2.0f * i / (n - 1) - 1.0f;
        window[i] = modifiedBessel(beta * std::sqrt(1.0f - normalized * normalized)) / i0_beta;
    }
}

void MathUtils::seedRandom(unsigned int seed) {
    random_seed_ = seed;
}

float MathUtils::randomFloat() {
    // Linear congruential generator
    random_seed_ = random_seed_ * 1664525u + 1013904223u;
    return (random_seed_ & 0x00FFFFFF) / 16777216.0f;
}

float MathUtils::randomFloat(float min, float max) {
    return lerp(min, max, randomFloat());
}

int MathUtils::randomInt(int min, int max) {
    return static_cast<int>(randomFloat(min, max + 1));
}

bool MathUtils::isPowerOfTwo(int value) {
    return value > 0 && (value & (value - 1)) == 0;
}

int MathUtils::nextPowerOfTwo(int value) {
    int power = 1;
    while (power < value) {
        power <<= 1;
    }
    return power;
}

float MathUtils::approximatelyEqual(float a, float b, float epsilon) {
    return std::abs(a - b) <= epsilon;
}

float MathUtils::smoothStep(float edge0, float edge1, float x) {
    if (x <= edge0) return 0.0f;
    if (x >= edge1) return 1.0f;
    
    float t = (x - edge0) / (edge1 - edge0);
    return t * t * (3.0f - 2.0f * t);
}

void MathUtils::initializeSinTable() {
    if (sin_table_initialized_) return;
    
    for (int i = 0; i < SIN_TABLE_SIZE; ++i) {
        float angle = 2.0f * PI * i / SIN_TABLE_SIZE;
        sin_table_[i] = std::sin(angle);
    }
    
    sin_table_initialized_ = true;
}

} // namespace vital