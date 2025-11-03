#pragma once

#include <vector>
#include <cmath>

namespace vital {

class MathUtils {
public:
    // Basic math utilities
    static constexpr float PI = 3.14159265358979323846f;
    static constexpr float TWO_PI = 2.0f * PI;
    static constexpr float HALF_PI = PI / 2.0f;
    static constexpr float INV_PI = 1.0f / PI;
    
    // Safe math functions
    static float fastAbs(float x);
    static float fastSqrt(float x);
    static float fastPow(float x, float y);
    static float fastExp(float x);
    
    // Linear interpolation
    static float lerp(float a, float b, float t);
    static void lerpVector(const std::vector<float>& a, const std::vector<float>& b, 
                          std::vector<float>& result, float t);
    
    // Exponential interpolation
    static float expLerp(float a, float b, float t, float curve = 1.0f);
    
    // Decibels conversion
    static float dbToLinear(float db);
    static float linearToDb(float linear);
    
    // Frequency conversions
    static float noteToFrequency(int note);
    static int frequencyToNote(float frequency);
    static float noteToSemitoneOffset(int note);
    
    // Normalization and scaling
    static float normalize(float value, float min, float max);
    static float denormalize(float normalized, float min, float max);
    static float clamp(float value, float min, float max);
    static float wrap(float value, float min, float max);
    
    // Trigonometry
    static float fastSin(float x);
    static float fastCos(float x);
    static void sincos(float x, float& sin_result, float& cos_result);
    
    // Window functions
    static void hannWindow(std::vector<float>& window);
    static void hammingWindow(std::vector<float>& window);
    static void blackmanWindow(std::vector<float>& window);
    static void kaiserWindow(std::vector<float>& window, float beta = 8.0f);
    
    // Random generation
    static void seedRandom(unsigned int seed);
    static float randomFloat(); // 0.0 to 1.0
    static float randomFloat(float min, float max);
    static int randomInt(int min, int max);
    
    // Special functions
    static float factorial(int n);
    static float binomial(int n, int k);
    static float bezier(std::vector<float>& points, float t);
    
    // Polynomial evaluation
    static float evaluatePolynomial(const std::vector<float>& coefficients, float x);
    static float evaluateBezier(const std::vector<float>& control_points, float t);
    
    // Utility
    static bool isPowerOfTwo(int value);
    static int nextPowerOfTwo(int value);
    static int previousPowerOfTwo(int value);
    
    // Clamping with edge case handling
    static float safeClamp(float value, float min, float max);
    static float safeWrap(float value, float min, float max);
    
    // Comparison with tolerance
    static bool approximatelyEqual(float a, float b, float epsilon = 1e-6f);
    static float smoothStep(float edge0, float edge1, float x);

private:
    // Fast approximations using lookup tables
    static constexpr int SIN_TABLE_SIZE = 1024;
    static const float sin_table_[SIN_TABLE_SIZE];
    static void initializeSinTable();
    
    // Random state
    static unsigned int random_seed_;
    static bool sin_table_initialized_;
};

} // namespace vital