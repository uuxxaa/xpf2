#pragma once
#include <stdint.h>
#include <string>
#include "xpfmath.h"

namespace xpf {

enum class intersection_type{
    LinesParallel = 0,
    LinesIntersect = 1,
    LinesCoincide = 2,
    LinesIntersectOutsideSegment1 = 3, //the intersection lies outside segment 1
    LinesIntersectOutsideSegment2 = 4, //the intersection lies outside segment 2
    LinesIntersectBothOutside = 5,
};

enum class winding_direction {
    ClockWise,
    CounterClockWise,
};

struct v2_t
{
    union {
        float f[2];
        struct { float x, y; };
        struct { float w, h; };
    };

#pragma region static properties
    static const v2_t down;
    static const v2_t left;
    static const v2_t negative_infinity;
    static const v2_t one;
    static const v2_t positive_infinity;
    static const v2_t right;
    static const v2_t up;
    static const v2_t zero;
#pragma endregion

#pragma region constructors
public:
    constexpr v2_t() : v2_t(0) {}
    constexpr v2_t(float x) : v2_t(x, x) {}
    constexpr v2_t(float x, float y) : x(x), y(y) {}
    v2_t(v2_t&&) = default;
    v2_t(const v2_t&) = default;

    v2_t& operator=(v2_t&&) = default;
    v2_t& operator=(const v2_t&) = default;
#pragma endregion

#pragma region operators
public:
    v2_t operator-() const { return v2_t(-x, -y); }

    v2_t operator+(float value) const { return v2_t(x + value, y + value); }
    v2_t operator-(float value) const { return v2_t(x - value, y - value); }
    v2_t operator*(float value) const { return v2_t(x * value, y * value); }
    v2_t operator/(float value) const { return v2_t(x / value, y / value); }

    v2_t& operator=(float value) { x = value; y = value; return *this; }
    v2_t& operator+=(float value) { x += value; y += value; return *this; }
    v2_t& operator-=(float value) { x -= value; y -= value; return *this; }
    v2_t& operator*=(float value) { x *= value; y *= value; return *this; }
    v2_t& operator/=(float value) { x /= value; y /= value; return *this; }

    v2_t operator+(v2_t v) const { return v2_t(x + v.x, y + v.y); }
    v2_t operator-(v2_t v) const { return v2_t(x - v.x, y - v.y); }
    v2_t operator*(v2_t v) const { return v2_t(x * v.x, y * v.y); }
    v2_t operator/(v2_t v) const { return v2_t(x / v.x, y / v.y); }

    v2_t& operator+=(v2_t v) { x += v.x; y += v.y; return *this; }
    v2_t& operator-=(v2_t v) { x -= v.x; y -= v.y; return *this; }
    v2_t& operator*=(v2_t v) { x *= v.x; y *= v.y; return *this; }
    v2_t& operator/=(v2_t v) { x /= v.x; y /= v.y; return *this; }

    bool operator==(v2_t v) { return xpf::math::is_negligible(x - v.x) && xpf::math::is_negligible(y - v.y); }
    bool operator!=(v2_t v) { return !xpf::math::is_negligible(x - v.x) || !xpf::math::is_negligible(y - v.y); }

    float operator[](size_t i) { return i == 0 ? x : y; }

    static v2_t add(v2_t a, v2_t b) { return a + b; }
    static v2_t sub(v2_t a, v2_t b) { return a - b; }
    static v2_t mul(v2_t a, v2_t b) { return a * b; }
    static v2_t div(v2_t a, v2_t b) { return a / b; }
#pragma endregion

#pragma region methods (unity compatibility)
public:
    float magnitude() const;
    float magnitude_squared() const;

    v2_t normal() const;
    static inline v2_t normal(v2_t v) { return v.normal(); }
    static inline v2_t normalize(v2_t v) { return v.normal(); }

    bool equals(v2_t v) { return *this == v; }
    std::string to_string(uint8_t precision = 3) const;
#pragma endregion

#pragma region additional methods (unity compatibility)
    static radians_t angle(v2_t from, v2_t to);

    // ClampMagnitude - missing

    static float cross(v2_t a, v2_t b);
    float cross(v2_t other) const;

    static float cross3(v2_t a, v2_t b, v2_t c);
    static float distance(v2_t a, v2_t b);
    static float distance_squared(v2_t a, v2_t b);
    static float dot(v2_t a, v2_t b) { return a.x * b.x + a.y * b.y; }
    float dot(v2_t other) const { return dot(*this, other); }

    // lerp is in xpf::Math

    static v2_t max(v2_t a, v2_t b) { return v2_t(std::max(a.x, b.x), std::max(a.y, b.y)); }
    static v2_t min(v2_t a, v2_t b) { return v2_t(std::min(a.x, b.x), std::min(a.y, b.y)); }

    // move_towards - missing

    v2_t perpendicular() const { return {-y, x}; } // counter clockwise 90 degrees
    static v2_t perpendicular(v2_t a) { return a.perpendicular(); } // counter clockwise 90 degrees

    v2_t perpendicular(bool negate) const;
    v2_t negate() const { return { x *  -1.0f, y * -1.0f }; }

    // reflect - missing
    static v2_t scale(v2_t a, v2_t b) { return v2_t(a.x * b.x, a.y * b.y); }

    // SignedAngle missing
    // SmoothDamp missing

    inline float slope() const { return y / x;}

    v2_t add_polar(radians_t angle, float radius) const { return {x + radius * cos(angle), y + radius * sin(angle)}; }
    v2_t add_x(float u) const { return {x + u, y}; }
    v2_t add_y(float v) const { return {x, y + v}; }

    inline bool is_negligible() const { return xpf::math::is_negligible(x) && xpf::math::is_negligible(y); }
    inline bool is_zero() const { return x == 0.0f && y == 0.0f; }

    static inline float signed_area(v2_t v1, v2_t v2, v2_t v3) {
        return (v2.x - v1.x) * (v3.y-v1.y) - (v3.x - v1.x) * (v2.y-v1.y);
    }

    static inline bool is_intersecting(v2_t a, v2_t b, v2_t c, v2_t d) {
        //return true if AB intersects CD
        return signed_area(a,b,c) > 0 != signed_area(a,b,d) > 0;
    }

    static v2_t mid_point(v2_t a, v2_t b) {
        return (a + b) * 0.5f;
    }

    static bool is_opposite_quadrant(v2_t v1, v2_t v2);

    static intersection_type is_intersecting(
        v2_t v1, v2_t v2, //line 1
        v2_t v3, v2_t v4, //line 2
        v2_t& vout,         //the output point
        float* ua_out = nullptr, float* ub_out = nullptr);

    static bool is_point_in_quad(v2_t m, v2_t A, v2_t B, v2_t C, v2_t D);
    bool is_point_in_quad(v2_t A, v2_t B, v2_t C, v2_t D) const { return v2_t::is_point_in_quad(*this, A, B, C, D); }

    static bool is_point_in_triangle(v2_t p, v2_t a, v2_t b, v2_t c);
    static bool is_point_on_line(v2_t p, v2_t a, v2_t b);
    static v2_t project_point_on_line(v2_t p, v2_t a, v2_t b);

    radians_t angle_in_radians() const { // from origin
        return xpf::math::atan2(y, x);
    }

    static radians_t angle_in_radians(v2_t a, v2_t b) {
        return (b - a).angle_in_radians();
    }

    static float normal(v2_t p1, v2_t p2, v2_t p3);

    static xpf::winding_direction winding_direction(v2_t p1, v2_t p2, v2_t p3) {
        return normal(p1, p2, p3) < 0 ? xpf::winding_direction::CounterClockWise : xpf::winding_direction::ClockWise;
    }

    v2_t rotate(radians_t angle) const;
    v2_t rotate_about(radians_t angle, v2_t point) const;

    const float* to_floats() const {
        static_assert(sizeof(v2_t) == sizeof(float) * 2);
        return reinterpret_cast<const float*>(this);
    }

    v2_t floor() const { return v2_t(::floor(x), ::floor(y)); }
    v2_t ceil() const { return v2_t(::ceil(x), ::ceil(y)); }
    v2_t round() const { return v2_t(::round(x), ::round(y)); }
    v2_t round_up() const { return v2_t(xpf::math::round_up(x), xpf::math::round_up(y)); }
    v2_t round_down() const { return v2_t(xpf::math::round_down(x), xpf::math::round_up(y)); }
    v2_t abs() const { return v2_t(std::abs(x), std::abs(y)); }
};

struct v2ui_t {
    uint32_t x = 0, y = 0;
};

struct v2i_t {
    int32_t x = 0, y = 0;
};

} // xpf