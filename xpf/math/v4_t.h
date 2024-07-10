#pragma once
#include "xpfmath.h"
#include "v2_t.h"

namespace xpf {

struct v4_t {
    union {
        float f[4];
        struct { float x, y, z, w; };
        struct { float r, g, b, a; };
    };

#pragma region constructors
    constexpr v4_t() : v4_t(0) {}
    constexpr v4_t(float x) : v4_t(x, x, x, x) {}
    constexpr v4_t(float x, float y) : v4_t(x, y, 0) {}
    constexpr v4_t(float x, float y, float z) : v4_t(x, y, z, 0) { }
    constexpr v4_t(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    constexpr v4_t(v2_t v, float z = 0, float w = 0) : x(v.x), y(v.y), z(z), w(w) {}
    v4_t(v4_t&&) = default;
    v4_t(const v4_t&) = default;
#pragma endregion

#pragma region data access
    float& operator[](size_t i) { return f[i]; }
    const float& operator[](size_t i) const { return f[i]; }
    const float* to_floats() const { return reinterpret_cast<const float*>(this); }
#pragma endregion

#pragma region operators
    v4_t& operator=(v4_t&&) = default;
    v4_t& operator=(const v4_t&) = default;

    v4_t operator+(const v4_t& o) const { return add(o); }
    v4_t operator-(const v4_t& o) const { return sub(o); }
    v4_t operator*(const v4_t& o) const { return mul(o); }
    v4_t operator/(const v4_t& o) const { return div(o); }

    v4_t& operator+=(const v4_t& o) { x += o.x; y += o.y; z += o.z; w += o.w; return *this; }
    v4_t& operator-=(const v4_t& o) { x -= o.x; y -= o.y; z -= o.z; w -= o.w; return *this; }
    v4_t& operator*=(const v4_t& o) { x *= o.x; y *= o.y; z *= o.z; w *= o.w; return *this; }
    v4_t& operator/=(const v4_t& o) { x /= o.x; y /= o.y; z /= o.z; w /= o.w; return *this; }
#pragma endregion

#pragma region comparison
    bool operator==(const v4_t& o) const { return x == o.x && y == o.y && z == o.z && w == o.w; }
    bool operator!=(const v4_t& o) const { return !(*this == o); }
#pragma endregion

#pragma region operations
    v4_t add(const v4_t& o) const { return {x + o.x, y + o.y, z + o.z, w + o.w}; }
    v4_t sub(const v4_t& o) const { return {x - o.x, y - o.y, z - o.z, w - o.w}; }
    v4_t mul(const v4_t& o) const { return {x * o.x, y * o.y, z * o.z, w * o.w}; }
    v4_t div(const v4_t& o) const { return {x / o.x, y / o.y, z / o.z, w / o.w}; }

#pragma region static operators / operations
    static v4_t add(const v4_t& a, const v4_t& b) { return a + b; }
    static v4_t sub(const v4_t& a, const v4_t& b) { return a - b; }
    static v4_t mul(const v4_t& a, const v4_t& b) { return a * b; }
    static v4_t div(const v4_t& a, const v4_t& b) { return a / b; }
#pragma endregion

};

} // xpf