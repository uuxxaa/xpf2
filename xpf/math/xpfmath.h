#pragma once
#include <cmath>
#include <math.h>
#include <tuple>
#include <vector>

// copy more from here: https://github.com/uuxxaa/UnityX/blob/main/UnityX/MathX.cs
namespace xpf {
using radians_t = float;
using degrees_t = float;

struct lerp_interval {
    float t, t2, t3;
};

static_assert(sizeof(lerp_interval) < 16, "do not exceed 16 bytes");

} // PCPP

namespace xpf::math {

#pragma region constants
static constexpr radians_t PI = radians_t(3.141592653589793f);
static constexpr radians_t TAU = PI * 2.0f;
static constexpr radians_t PI_HALF = PI * .5f; // 90 degrees
static constexpr radians_t PI_QUATER = PI_HALF * .5f; // 45 degrees
static constexpr radians_t MinAngleForRoundShapes = PI / 18.f; // 10 degrees
static constexpr float Epsilon = 0.00000000001f;
static constexpr float NegativeEpsilon = -math::Epsilon;
static constexpr float E = 2.71828182846f;
static constexpr float GOLDEN_RATIO = 1.61803398875f;
static constexpr float SQRT2 = 1.41421356237f;
static constexpr float F1Over3 = 1.0f / 3.0f;
static constexpr float F2Over3 = 2.0f / 3.0f;
static constexpr float F1Over6 = 1.0f / 6.0f;
#pragma endregion

#pragma region clamping & mapping
template <typename T> T clamp(T value, T minValue, T maxValue) { return value <= minValue ? minValue : (value <= maxValue ? value : maxValue); }
template <typename T> T map(T value, T start1, T stop1, T start2, T stop2) { return (value - start1) / (stop1 - start1) * (stop2 - start2) + start2; }
template <typename T> T map_clamped(T value, T start1, T stop1, T start2, T stop2) {
    value = map(value, start1, stop1, start2, stop2);
    if (start2 < stop2)
        return clamp(value, start2, stop2);
    return clamp(value, stop2, start2);
}
#pragma endregion

#pragma region lerping
template <typename T> T lerp(T a, T b, float t) { return a + (b - a) * t; }
template <typename T> T lerp(T a, T b, lerp_interval ts) { return lerp(a, b, ts.t); }
template <typename T> T lerp_clamped(T a, T b, float t) { return clamp(lerp(a, b, t), a, b); }
template <typename T> T lerp_clamped(T a, T b, lerp_interval ts) { return lerp_clamped(a, b, ts.t); }
template <typename T> T inverse_lerp(T a, T b, float value) { return (value - a)/(b - a); }

template <typename T> T lerp_quadratic(T a, T b, T c, float t)
{
    float t2 = t * t;
    return
        (a * (1 - 2*t + t2)) +
        (b * (      t - t2) * 2) +
        (c * t2);
}

template <typename T> T lerp_quadratic(T a, T b, T c, lerp_interval ts)
{
    return
        (a * (1 - 2*ts.t + ts.t2)) +
        (b * (      ts.t - ts.t2) * 2) +
        (c * ts.t2);
}

template <typename T> T lerp_cubic(T a, T b, T c, T d, lerp_interval ts)
{
    return
        (a * (1 - 3*ts.t + 3*ts.t2 - ts.t3)) +
        (b * (      ts.t - 2*ts.t2 + ts.t3) * 3) +
        (c * (               ts.t2 - ts.t3) * 3) +
        (d * ts.t3);
}

template <typename T> T lerp_quartic(T a, T b, T c, T d, T e, lerp_interval ts)
{
    return lerp(
        lerp_cubic(a, b, c, d, ts),
        lerp_cubic(b, c, d, e, ts), ts.t);
}

template <typename T> T midpoint(T a, T b) { return (a + b) * .5f; }

inline std::vector<lerp_interval> calculate_lerp_points(uint32_t divisions){
    std::vector<lerp_interval> result;
    result.reserve(divisions + 1);
    for (uint32_t i = 0; i <= 10000; i+= 10000 / divisions)
    {
        float t = i / 10000.0f;
        float t2 = t * t;
        float t3 = t2 * t;
        result.push_back({t, t2, t3});
    }
    return result;
}

static const std::vector<lerp_interval> s_lerpPoints2 = calculate_lerp_points(2);
static const std::vector<lerp_interval> s_lerpPoints10 = calculate_lerp_points(10);
static const std::vector<lerp_interval> s_lerpPoints20 = calculate_lerp_points(20);
static const std::vector<lerp_interval> s_lerpPoints40 = calculate_lerp_points(40);
static const std::vector<lerp_interval> s_lerpPoints200 = calculate_lerp_points(200);
#pragma endregion

template <typename T> T smoothstep(T edge0, T edge1, T x)
{
    // Scale, bias and saturate x to 0..1 range
    x = clamp<T>((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    // Evaluate polynomial
    return x * x * (3 - 2 * x);
}

inline bool is_negligible(int32_t value) { return value == 0; }
inline bool is_negligible(uint32_t value) { return value == 0; }

inline bool is_negligible(float value) { return -math::Epsilon < value && value < math::Epsilon; }
inline bool is_negligible(float value, float epsilon) { return -epsilon < value && value < epsilon; }
inline bool is_negligible_or_negative(float value) { return value < math::Epsilon; }
template<typename T> T sign(T value) { return value < 0 ? (T)-1 : (T)1; }
inline bool is_less_than(float f1, float f2) { return (f1 < f2) && !is_negligible(f1 - f2); }

inline bool is_equals_almost(float value, float valueCompareTo) { return is_negligible(value - valueCompareTo); }
inline bool is_equals_almost(float value, float valueCompareTo, float epsilon) { return is_negligible(value - valueCompareTo, epsilon); }

inline float sin(radians_t theta) { return std::sinf(theta); }
inline float cos(radians_t theta) { return std::cosf(theta); }
inline radians_t acos(float x) { return std::acosf(x); }
inline radians_t asin(float x) { return std::asinf(x); }
inline radians_t atan2(float y, float x) { return std::atan2(y, x); }

inline float BezierTangent(float a, float b, float c, float d, lerp_interval ts) {
  const float adjustedT = 1 - ts.t;
  return (
    3 * d * ts.t2 -
    3 * c * ts.t2 +
    6 * c * adjustedT * ts.t -
    6 * b * adjustedT * ts.t +
    3 * b * (adjustedT * adjustedT) -
    3 * a * (adjustedT * adjustedT)
  );
};

inline float BezierPoint(float a, float b, float c, float d, lerp_interval ts) {
  const float adjustedT = 1 - ts.t;
  return (
    (adjustedT * adjustedT * adjustedT) * a +
    3 * (adjustedT * adjustedT) * ts.t * b +
    3 * adjustedT * ts.t2 * c +
    ts.t3 * d
  );
};

// https://www.moshplant.com/direct-or/bezier/math.html
inline std::tuple<float, float, float> BezierCoefficients(float y0, float y1, float y2, float y3) {
    float c = 3 * (y1 - y0);
    float b = 3 * (y2 - y1) - c;
    float a = y3 - y0 - c - b;
    return {a, b, c};
}

inline radians_t degrees_to_radians(degrees_t degrees) { return degrees * (PI /180.0f); }
inline degrees_t radians_to_degrees(radians_t radians) { return radians * (180.0f / PI); }
inline float fraction(float f) { return f - std::floor(f); }
template<typename T> T square(T t) { return t * t; }
template<typename T> T cube(T t) { return t * t * t; }

#pragma region easing functions
// https://github.com/warrenm/AHEasing/blob/master/AHEasing/easing.c
inline float ease_in_out(float t, float alpha) {
    const float tPowAlpha = std::powf(t, alpha);
    return tPowAlpha / (tPowAlpha + std::powf(1 - t, alpha));
}

inline float quadratic_ease_in(float t) { return t * t; }
inline float quadratic_ease_out(float t) { return t * (2 - t); }
inline float quadratic_ease_inOut(float t) {
    return (t < 0.5)
        ? (2 * quadratic_ease_in(t))
        : ((-2 * t * t) + (4 * t) - 1);
}

inline float cubic_ease_in(float t) { return t * t * t; }
inline float cubic_ease_out(float t) { return cubic_ease_in(t - 1) + 1; }
inline float cubic_ease_inOut(float t) {
    return (t < 0.5)
        ? (4 * cubic_ease_in(t))
        : (4 * cubic_ease_in(t - 1) + 1);
}

inline float quartic_ease_in(float t) { return t * t * t * t; }
inline float quartic_ease_out(float t) { return 1 - quartic_ease_in(t - 1); }
inline float quartic_ease_inOut(float t) {
    return t < 0.5
        ? (8 * quartic_ease_in(t))
        : (-8 * quartic_ease_in(t - 1) + 1);
}

inline float quintic_ease_in(float t) { return t * t * t * t * t; }
inline float quintic_ease_out(float t) { return quintic_ease_in(t - 1) + 1; }
inline float quintic_ease_inOut(float t) {
    return (t < 0.5)
        ? (16 * quintic_ease_in(t))
        : (16 * quintic_ease_in(t - 1) + 1);
}

inline float sine_ease_in(float t) { return xpf::math::sin((t - 1) * math::PI_HALF) + 1; }
inline float sine_ease_out(float t) { return xpf::math::sin(t * math::PI_HALF); }
inline float sine_ease_inOut(float t) { return 0.5 * (1 - xpf::math::cos(t * math::PI)); }

inline float circular_ease_in(float t) { return 1 - std::sqrtf(1 - (t * t)); }
inline float circular_ease_out(float t) { return sqrt((2 - t) * t); }
inline float circular_ease_inOut(float t) {
    if (t < 0.5) return 0.5 * (1 - std::sqrtf(1 - 4 * (t * t)));
    return 0.5 * (std::sqrtf(-((2 * t) - 3) * ((2 * t) - 1)) + 1);
}

inline float exponential_ease_in(float t) { return t == 0 ? 0 : std::powf(2, 10 * (t - 1)); }
inline float exponential_ease_out(float t) { return (t == 1.0) ? 1.0 : (1 - std::powf(2, -10 * t)); }
inline float exponential_ease_inOut(float t) {
    if(t == 0.0 || t == 1.0) return t;
    if(t < 0.5f) return 0.5 * std::powf(2, (20 * t) - 10);
    return -0.5 * std::powf(2, (-20 * t) + 10) + 1;
}

inline float elastic_ease_in(float t) {
    return math::sin(13 * math::PI_HALF * t) * std::powf(2, 10 * t - 10);
}

inline float elastic_ease_out(float t) {
    return math::sin(-13 * math::PI_HALF * (t + 1)) * pow(2, -10 * t) + 1;
}

inline float elastic_ease_inOut(float t) {
    return (t < 0.5f)
        ? (0.5 *
            math::sin(13 * math::PI * t) *
            pow(2, 10 * (t - 1)))
        : (0.5 * (
            math::sin(-13 * math::PI * t) *
            std::powf(2, 10 - 20 * t)
            + 2));
}

inline float back_ease_in(float t) { return xpf::math::cube(t) - t * math::sin(t * math::PI); }
inline float back_ease_out(float t) {
    const float f = (1 - t);
    return 1 - (math::cube(f) - f * math::sin(f * math::PI));
}

inline float back_ease_inOut(float t) {
    if (t < 0.5) {
        const float f = 2 * t;
        return 0.5 * (math::cube(f) - f * math::sin(f * math::PI));
    }

    const float f = 2 * (1 - t);
    return 0.5 * (1 - (math::cube(f) - f * math::sin(f * math::PI))) + 0.5;
}

inline float bounce_ease_out(float t) {
    if (t < 4/11.0) return (121 * t * t)/16.0;
    if (t < 8/11.0) return (363/40.0 * t * t) - (99/10.0 * t) + 17/5.0;
    if (t < 9/10.0) return (4356/361.0 * t * t) - (35442/1805.0 * t) + 16061/1805.0;
    return (54/5.0 * t * t) - (513/25.0 * t) + 268/25.0;
}

inline float bounce_ease_in(float t) { return 1 - bounce_ease_out(1 - t); }

inline float bounce_ease_inOut(float t) {
    return t < 0.5f
        ? (0.5f * bounce_ease_in(t * 2))
        : (0.5f * bounce_ease_out(t * 2 - 1) + 0.5f);
}
#pragma endregion

template<typename T> T round_up(T f) { return round(f + .4999f); };
template<typename T> T round_down(T f) { return round(f - .4999f); };

inline float clamp_min(float minvalue, float t) { return t <= minvalue ? minvalue : t; }
inline float clamp_max(float maxvalue, float t) { return t >= maxvalue ? maxvalue : t; }

} // xpf::math

namespace xpf {

template<typename T> struct math_trait
{
    static T lerp(float a, float b, float t) { return xpf::math::lerp<T>(a, b, t); }
};

} // xpf
