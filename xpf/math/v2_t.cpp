#include "v2_t.h"
#include <cmath>
#include <limits>
#include <core/stringex.h>

namespace xpf {

static_assert(sizeof(v2_t) == 8, "must be 8");

constexpr v2_t v2_t::down = v2_t(0.0f, -1.0f);
constexpr v2_t v2_t::left = v2_t(-1.0f, 0.0f);
constexpr v2_t v2_t::negative_infinity = v2_t(-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity());
constexpr v2_t v2_t::one = v2_t(1.0f, 1.0f);
constexpr v2_t v2_t::positive_infinity = v2_t(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
constexpr v2_t v2_t::right = v2_t(1.0f, 0.0f);
constexpr v2_t v2_t::up = v2_t(0.0f, 1.0f);
constexpr v2_t v2_t::zero = v2_t(0.0f, 0.0f);

// Negative infinity checks
// https://stackoverflow.com/questions/20016600/negative-infinity
static_assert(std::numeric_limits<float>::is_iec559, "IEEE 754 required");
//C99
constexpr float negative_infinity1 = -INFINITY;
constexpr float negative_infinity2 = -1 * INFINITY;

constexpr float negative_infinity3 = -std::numeric_limits<float>::infinity();
constexpr float negative_infinity4 = -1 * std::numeric_limits<float>::infinity();

static_assert(negative_infinity1 < std::numeric_limits<float>::lowest(), "");
static_assert(negative_infinity2 < std::numeric_limits<float>::lowest(), "");
static_assert(negative_infinity3 < std::numeric_limits<float>::lowest(), "");
static_assert(negative_infinity4 < std::numeric_limits<float>::lowest(), "");

#pragma region methods (unity compatibility)
float v2_t::magnitude() const { return std::sqrtf(magnitude_squared()); }
float v2_t::magnitude_squared() const { return x * x + y * y; }

v2_t v2_t::normal() const {
    float length = magnitude();
    if (length > math::Epsilon)
        return v2_t(x / length, y / length);

    return v2_t();
}

std::string v2_t::to_string(uint8_t precision) const {
    return "{x:" + xpf::stringex::to_string(x, precision) + " y:" + xpf::stringex::to_string(y, precision) + "}";
}
#pragma endregion

#pragma region additional methods (unity compatibility)
/*static*/ radians_t v2_t::angle(v2_t from, v2_t to) {
    return xpf::math::acos(xpf::math::clamp(v2_t::dot(from.normal(), to.normal()), -1.0f, 1.0f));
}

// ClampMagnitude - missing

/*static*/ float v2_t::cross(v2_t a, v2_t b) { return a.x * b.y - a.y * b.x; }
float v2_t::cross(v2_t other) const { return cross(*this, other); }

/*static*/ float v2_t::cross3(v2_t a, v2_t b, v2_t c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
};

/*static*/ float v2_t::distance(v2_t a, v2_t b) {
    return sqrtf(distance_squared(a, b));
}

/*static*/ float v2_t::distance_squared(v2_t a, v2_t b) {
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    return (dx * dx + dy * dy);
}

/*static*/ bool v2_t::is_opposite_quadrant(v2_t v1, v2_t v2)
{
    const int8_t p1x = v1.x > 0 ? 1 : ( v1.x < 0 ? -1 : 0);
    const int8_t p1y = v1.y > 0 ? 1 : ( v1.y < 0 ? -1 : 0);
    const int8_t p2x = v2.x > 0 ? 1 : ( v2.x < 0 ? -1 : 0);
    const int8_t p2y = v2.y > 0 ? 1 : ( v2.y < 0 ? -1 : 0);

    if (p1x != p2x) {
        if (p1y != p2y)
            return true;
        else if (p1y == 0 || p2y == 0)
            return true;
    }

    if (p1y != p2y) {
        if (p1x==0 || p2x==0)
            return true;
    }

    return false;
}

/*static*/ intersection_type v2_t::is_intersecting(
    v2_t v1, v2_t v2, //line 1
    v2_t v3, v2_t v4, //line 2
    v2_t& vout,         //the output point
    float* ua_out, float* ub_out)
{
    const float denom  = (v4.y-v3.y) * (v2.x-v1.x) - (v4.x-v3.x) * (v2.y-v1.y);
    const float numera = (v4.x-v3.x) * (v1.y-v3.y) - (v4.y-v3.y) * (v1.x-v3.x);
    const float numerb = (v2.x-v1.x) * (v1.y-v3.y) - (v2.y-v1.y) * (v1.x-v3.x);

    if (xpf::math::is_negligible(numera) &&
        xpf::math::is_negligible(numerb) &&
        xpf::math::is_negligible(denom))
    {
        vout.x = (v1.x + v2.x) * 0.5;
        vout.y = (v1.y + v2.y) * 0.5;
        return intersection_type::LinesCoincide;
    }

    if (xpf::math::is_negligible(denom)) {
        vout.x = 0;
        vout.y = 0;
        return intersection_type::LinesParallel; // no intersection
    }

    float mua = numera / denom;
    float mub = numerb / denom;
    if (ua_out != nullptr) *ua_out = mua;
    if (ub_out != nullptr) *ub_out = mub;

    vout.x = v1.x + mua * (v2.x - v1.x);
    vout.y = v1.y + mua * (v2.y - v1.y);

    const bool out1 = mua < 0 || mua > 1;
    const bool out2 = mub < 0 || mub > 1;

    if (out1 & out2) {
        return intersection_type::LinesIntersectBothOutside; // the intersection lies outside both segments
    } else if ( out1) {
        return intersection_type::LinesIntersectOutsideSegment1; // the intersection lies outside segment 1
    } else if ( out2) {
        return intersection_type::LinesIntersectOutsideSegment2; // the intersection lies outside segment 2
    } else {
        return intersection_type::LinesIntersect; // intersects
    }
    //http://paulbourke.net/geometry/lineline2d/
}

static v2_t vect2d(v2_t p1, v2_t p2) {
    return v2_t(p2.x - p1.x, p1.y - p2.y);
}

// https://stackoverflow.com/questions/2752725/finding-whether-a-point-lies-inside-a-rectangle-or-not
/*static*/ bool v2_t::is_point_in_quad(v2_t m, v2_t A, v2_t B, v2_t C, v2_t D) {
    v2_t AB = vect2d(A, B); float D1 = (AB.y*m.x + AB.x*m.y) - (AB.y*A.x + AB.x*A.y);
    v2_t AD = vect2d(A, D); float D2 = (AD.y*m.x + AD.x*m.y) - (AD.y*A.x + AD.x*A.y);
    v2_t BC = vect2d(B, C); float D3 = (BC.y*m.x + BC.x*m.y) - (BC.y*B.x + BC.x*B.y);
    v2_t CD = vect2d(C, D); float D4 = (CD.y*m.x + CD.x*m.y) - (CD.y*C.x + CD.x*C.y);
    return 0 >= D1 && 0 >= D4 && 0 <= D2 && 0 >= D3;
}

// https://github.com/SebLague/Ear-Clipping-Triangulation/blob/master/Scripts/Maths2D.cs
/*static*/ bool v2_t::is_point_in_triangle(v2_t p, v2_t a, v2_t b, v2_t c) {
    float area = 0.5f * (-b.y * c.x + a.y * (-b.x + c.x) + a.x * (b.y - c.y) + b.x * c.y);
    float s = 1 / (2 * area) * (a.y * c.x - a.x * c.y + (c.y - a.y) * p.x + (a.x - c.x) * p.y);
    float t = 1 / (2 * area) * (a.x * b.y - a.y * b.x + (a.y - b.y) * p.x + (b.x - a.x) * p.y);
    return s >= 0 && t >= 0 && (s + t) <= 1;
}

/*static*/ bool v2_t::is_point_on_line(v2_t p, v2_t a, v2_t b) {
    return distance(a, b) == distance(a, p) + distance(b, p);
}

/*static*/ v2_t v2_t::project_point_on_line(v2_t p, v2_t a, v2_t b) {
    v2_t u = p - a;
    v2_t v = b - a;
    return a + (v * v2_t::dot(u, v) / v2_t::dot(v,v));
}

/*static*/ float v2_t::normal(v2_t p1, v2_t p2, v2_t p3) {
    v2_t v1 = (p1 - p2);
    v2_t v2 = (p1 - p3);

    float crossProduct = cross(v1, v2);
    if (xpf::math::is_negligible(crossProduct))
        return 0;

    return crossProduct < 0 ? -1 : 1;
}

// Rotate about (0, 0)
v2_t v2_t::rotate(radians_t angle) const {
    const float cos = xpf::math::cos(angle);
    const float sin = xpf::math::sin(angle);

    return {
        x * cos - y * sin,
        x * sin + y * cos };
}

v2_t v2_t::rotate_about(radians_t angle, v2_t point) const {
    const float cos = xpf::math::cos(angle);
    const float sin = xpf::math::sin(angle);

    return {
        point.x + ((x - point.x) * cos - (y - point.y) * sin),
        point.y + ((x - point.x) * sin + (y - point.y) * cos) };
}
#pragma endregion

} // xpf