#pragma once
#include <array>
#include <vector>
#include <stdint.h>
#include <math/v2_t.h>
#include <math/v4_t.h>
#include <math/xpfmath.h>

namespace xpf {

struct Thickness;

template<typename T>
struct rectangle
{
    T x = 0, y = 0;
    union {
        T w = 0;
        T width;
    };
    union {
        T h = 0;
        T height;
    };

    static rectangle from_points(T left, T top, T right, T bottom) { return rectangle{left, top, right - left, bottom - top}; }

    bool operator==(const rectangle& r) const { return x == r.x && y == r.y && w == r.w && h == r.h; }
    bool operator!=(const rectangle& r) const { return !(*this == r); }

    bool is_empty() const { return math::is_negligible(w) == 0 || math::is_negligible(h); }

    rectangle intersection(const rectangle& other) const {
        // gives bottom-left point
        // of intersection rectangle
        auto topLeftX = std::max(x, other.x);
        auto topLeftY = std::max(y, other.y);

        // gives top-right point
        // of intersection rectangle
        auto bottomRightX = std::min(x + w, other.x + other.w);
        auto bottomRightY = std::min(y + h, other.y + other.h);

        // no intersection
        if (topLeftX > bottomRightX || topLeftY > bottomRightY)
            return rectangle();

        return {
            topLeftX, topLeftY,
            bottomRightX - topLeftX, bottomRightY - topLeftY };
    }

    bool is_inside(v2_t v) const {
        return
            x < v.x && v.x < x + w &&
            y < v.y && v.y < y + h;
    }

    rectangle round_up() const {
        return { xpf::math::round_up(x), xpf::math::round_up(y), xpf::math::round_up(w), xpf::math::round_up(h) };
    }
    rectangle roundf() const {
        return { std::roundf(x), std::roundf(y), std::roundf(w), std::roundf(h) };
    }

    rectangle multiply(T m) const {
        return Multiply(m, m);
    }

    rectangle multiply(T wIn, T hIn) const {
        return { x * wIn, y * hIn, w * wIn, h * hIn };
    }

    rectangle expand(T wIn, T hIn) const {
        return { x - wIn, y - hIn, w + wIn + wIn, h + hIn + hIn};
    }

    rectangle expand(T size) const {
        return { x - size, y - size, w + size + size, h + size + size};
    }

    rectangle shrink(T size) const {
        return expand(-size);
    }

    rectangle shrink(T wIn, T hIn) const {
        return expand(-wIn, -hIn);
    }

    rectangle shrink(v4_t v) const {
        return { x + v.x, y + v.y, w - v.x - v.z, h - v.y - v.w};
    }

    rectangle shrink(const Thickness& t) const;
    rectangle expand(const Thickness& t) const;

    rectangle move(T wIn, T hIn) const {
        return { x + wIn, y + hIn, w, h };
    }

    void set_position(v2_t pos) {
        x = pos.x; y = pos.y;
    }

    inline void populate_triangles(std::array<T, 12>& target) const {
        target = {
            // 1
            left(),  top(),
            left(),  bottom(),
            right(), top(),
            // 2
            right(), top(),
            left(),  bottom(),
            right(), bottom(),
        };    }

    inline void populate_triangles(std::vector<v2_t>& target) const {
        target.push_back(v2_t { left(),  top() });
        target.push_back(v2_t { left(),  bottom() });
        target.push_back(v2_t { right(), top() });

        target.push_back(v2_t { right(), top() });
        target.push_back(v2_t { left(),  bottom() });
        target.push_back(v2_t { right(), bottom() });
    }

    inline void populate_triangle_fan(std::array<T, 8>& target) const {
        target = {
            left(),  top(),
            right(), top(),
            right(), bottom(),
            left(),  bottom(),
        };
    }

    inline void populate_triangle_fan(std::array<v2_t, 4>& target) const {
        target = {
            v2_t{left(),  top()},
            v2_t{right(), top()},
            v2_t{right(), bottom()},
            v2_t{left(),  bottom()},
        };
    }

    constexpr T left() const { return x; }
    constexpr T right() const { return x + w; }
    constexpr T top() const { return y; }
    constexpr T bottom() const { return y + h; }

    v2_t size() const { return {float(w), float(h)}; }
    v2_t size_half() const { return {float(w) * .5, float(h) * .5}; }
    v2_t center() const { return {float(x + w * .5), float(y + h * .5)}; }

    v2_t top_left() const { return { float(x), float(y) }; }
    v2_t top_right() const { return { float(x + w), float(y) }; }
    v2_t bottom_right() const { return { float(x + w), float(y + h) }; }
    v2_t bottom_left() const { return { float(x), float(y + h) }; }

    std::string to_string(uint8_t precision = 0) const;
};

typedef rectangle<float> rectf_t;
typedef rectangle<int32_t> recti_t;
typedef rectangle<uint32_t> rectui_t;

inline rectf_t floor(rectf_t r) { return rectf_t{::floor(r.x), ::floor(r.y), ::ceil(r.w), ::ceil(r.h)}; }
inline rectf_t ceil(rectf_t r) { return rectf_t{::ceil(r.x), ::ceil(r.y), ::ceil(r.w), ::ceil(r.h)}; }
inline rectf_t floorxy_ceilwh(rectf_t r) { return rectf_t{::floor(r.x), ::floor(r.y), ::ceil(r.w), ::ceil(r.h)}; }

template<> struct math_trait<xpf::rectf_t>
{
    static xpf::rectf_t lerp(
        const xpf::rectf_t& a,
        const xpf::rectf_t& b,
        float t)
    {
        if (t<0.0f) t = 0.0f;
        if (t>1.0f) t = 1.0f;

        const float kt = 1.0f - t;
        return rectf_t{
            a.x * kt + b.x * t,
            a.y * kt + b.y * t,
            a.w * kt + b.w * t,
            a.h * kt + b.h * t
        };
    }
};

} // xpf
