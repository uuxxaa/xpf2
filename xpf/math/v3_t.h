#pragma once
#include "xpfmath.h"
#include "v2_t.h"

namespace xpf {

class stringex;
struct v4_t;

struct v3_t
{
    union {
        float f[3];
        struct { float x, y, z; };
        struct { float r, g, b; };
    };

#pragma region constructors
public:
    constexpr v3_t() : v3_t(0) {}
    constexpr v3_t(float x) : v3_t(x, x) {}
    constexpr v3_t(float x, float y) : v3_t(x, y, 0) {}
    constexpr v3_t(float x, float y, float z) : x(x), y(y), z(z) {}
    constexpr v3_t(v2_t v, float z = 0) : x(v.x), y(v.y), z(z) {}
    v3_t(v3_t&&) = default;
    v3_t(const v3_t&) = default;

    float& operator[](size_t i) { return f[i]; }
    const float& operator[](size_t i) const { return f[i]; }

    v3_t& operator=(v3_t&&) = default;
    v3_t& operator=(const v3_t&) = default;
#pragma endregion

#pragma region operators
public:
    v3_t operator-() const { return v3_t(-x, -y, -z); }

    v3_t operator+(float value) const { return v3_t(x + value, y + value, z + value); }
    v3_t operator-(float value) const { return v3_t(x - value, y - value, z - value); }
    v3_t operator*(float value) const { return v3_t(x * value, y * value, z * value); }
    v3_t operator/(float value) const { return v3_t(x / value, y / value, z / value); }

    v3_t& operator=(float value)  { x = value;  y = value;  z = value;  return *this; }
    v3_t& operator+=(float value) { x += value; y += value; z += value; return *this; }
    v3_t& operator-=(float value) { x -= value; y -= value; z -= value; return *this; }
    v3_t& operator*=(float value) { x *= value; y *= value; z *= value; return *this; }
    v3_t& operator/=(float value) { x /= value; y /= value; z /= value; return *this; }

    v3_t operator+(v3_t v) const { return add(v); }
    v3_t operator-(v3_t v) const { return sub(v); }
    v3_t operator*(v3_t v) const { return mul(v); }
    v3_t operator/(v3_t v) const { return div(v); }

    v3_t& operator+=(v3_t v) { x += v.x; y += v.y; z += v.z; return *this; }
    v3_t& operator-=(v3_t v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    v3_t& operator*=(v3_t v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
    v3_t& operator/=(v3_t v) { x /= v.x; y /= v.y; z /= v.z; return *this; }

    bool operator==(v3_t v) { return xpf::math::is_negligible(x - v.x) && xpf::math::is_negligible(y - v.y) && xpf::math::is_negligible(z - v.z); }
    bool operator!=(v3_t v) { return !xpf::math::is_negligible(x - v.x) || !xpf::math::is_negligible(y - v.y) || !xpf::math::is_negligible(z - v.z); }
#pragma endregion

#pragma region swizzle
public:
    v2_t xy() { return {x, y}; }
#pragma endregion

#pragma region operations
public:
    v3_t add(const v3_t& o) const { return {x + o.x, y + o.y, z + o.z}; }
    v3_t sub(const v3_t& o) const { return {x - o.x, y - o.y, z - o.z}; }
    v3_t mul(const v3_t& o) const { return {x * o.x, y * o.y, z * o.z}; }
    v3_t div(const v3_t& o) const { return {x / o.x, y / o.y, z / o.z}; }
#pragma endregion

#pragma region static operators / operations
public:
    static v3_t add(v3_t a, v3_t b) { return a + b; }
    static v3_t sub(v3_t a, v3_t b) { return a - b; }
    static v3_t mul(v3_t a, v3_t b) { return a * b; }
    static v3_t div(v3_t a, v3_t b) { return a / b; }
#pragma endregion

#pragma region methods
public:
    static float angle_between(v3_t a, v3_t b);

    static float Distance(v3_t a, v3_t b);
    static float DistanceSquared(v3_t a, v3_t b);

    v3_t cross(v3_t b) const { return v3_t::cross(*this, b); }
    static v3_t cross(v3_t a, v3_t b);

    float dot(v3_t b) const { return x * b.x + y * b.y + z * b.z; }
    static float dot(v3_t a, v3_t b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

    bool equals(v3_t v) { return *this == v; }

    float length() const { return magnitude(); }
    static float  length(v3_t v) { return v.length(); }

    float magnitude() const { return std::sqrtf(magnitude_squared()); }
    float magnitude_squared() const { return x * x + y * y + z * z; }

    v3_t normal() const;
    static v3_t normal(v3_t v) { return v.normal(); }

    v3_t project(v3_t onto) { return project(*this, onto); }
    static v3_t project(v3_t a, v3_t onto);

    // Lerp is in xpf::math

    static v3_t max(v3_t a, v3_t b) { return {std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)}; }
    static v3_t min(v3_t a, v3_t b) { return {std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)}; }

    // move_towards - missing
    // perpendicular - missing

    v3_t negate() const { return { x *  -1.0f, y * -1.0f, z * -1.0f }; }

    // reflect - missing

    static v3_t scale(v3_t a, v3_t b) { return mul(a, b); }

    // SignedAngle missing
    // SmoothDamp missing

    v3_t add_x(float u) { return {x + u, y, z}; }
    v3_t add_y(float v) { return {x, y + v, z}; }
    v3_t add_z(float w) { return {x, y, z + w}; }

    inline bool is_negligible() const { return math::is_negligible(x) && math::is_negligible(y) && math::is_negligible(z); }
    inline bool is_zero() const { return x == 0.0f && y == 0.0f && z == 0.0f; }

    std::string to_string() const;
#pragma endregion

public:
    const float* to_floats() const {
        static_assert(sizeof(v3_t) == sizeof(float)*3);
        return reinterpret_cast<const float*>(this);
    }
};

} // xpf