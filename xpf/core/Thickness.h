#pragma once
#include <core/Rectangle.h>
#include <math/v2_t.h>

namespace xpf {

struct Thickness {
    float l = 0, t = 0, r = 0, b = 0;
    Thickness() = default;
    Thickness(float f) : l(f), t(f), r(f), b(f) { }
    Thickness(float x, float y) : l(x), t(x), r(y), b(y) { }
    Thickness(float l, float t, float r, float b) : l(l), t(t), r(r), b(b) { }

    bool operator==(const Thickness& o) const { return l == o.l && t == o.t && r == o.r && b == o.b; }
    bool operator!=(const Thickness& o) const { return !(*this == o); }

    static v2_t expand(v2_t size, Thickness o) { return { size.x + o.l + o.r, size.y + o.t + o.b }; }
    static rectf_t expand(rectf_t r, Thickness o) { return {r.x - o.l, r.y - o.t, r.w + o.l + o.r, r.h + o.t + o.b }; }

    static v2_t shrink(v2_t size, Thickness o) { return { size.x - o.l - o.r, size.y - o.t - o.b }; }
    static rectf_t shrink(rectf_t r, Thickness o) { return {r.x + o.l, r.y + o.t, r.w - o.l - o.r, r.h - o.t - o.b }; }

    v4_t get_v4() const { return {l, t, r, b}; }
    float left_right() const { return l + r; }
    float top_bottom() const { return t + b; }

    bool is_zero() const {
        return
            math::is_negligible(l) &&
            math::is_negligible(t) &&
            math::is_negligible(r) &&
            math::is_negligible(b); }

    bool is_zero_or_negative() const {
        return
            math::is_negligible_or_negative(l) &&
            math::is_negligible_or_negative(t) &&
            math::is_negligible_or_negative(r) &&
            math::is_negligible_or_negative(b); }
};

inline v2_t operator+(v2_t v, const Thickness t) {
    return {
        v.x + t.l + t.r,
        v.y + t.t + t.b
    };
}

inline v2_t operator-(v2_t v, const Thickness t) {
    return {
        v.x - t.l - t.r,
        v.y - t.t - t.b
    };
}

} // xpf