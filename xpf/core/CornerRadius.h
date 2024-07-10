#pragma once
#include <math/v4_t.h>

namespace xpf {

struct CornerRadius {
    union {
        struct { float top_left, top_right, bottom_right, bottom_left; };
        v4_t v;
    };

    CornerRadius() : CornerRadius(0) { }
    CornerRadius(float f) : top_left(f), top_right(f), bottom_right(f), bottom_left(f) { }
    CornerRadius(float x, float y) : top_left(x), top_right(y), bottom_right(x), bottom_left(y) { }
    CornerRadius(float top_left, float top_right, float bottom_right, float bottom_left) : top_left(top_left), top_right(top_right), bottom_right(bottom_right), bottom_left(bottom_left) { }

    bool operator==(const CornerRadius& o) const { return top_left == o.top_left && top_right == o.top_right && bottom_right == o.bottom_right && bottom_left == o.bottom_left; }
    bool operator!=(const CornerRadius& o) const { return !(*this == o); }

    v4_t get_v4() const { return v4_t(top_left, top_right, bottom_right, bottom_left); }

    bool is_zero() const {
        return
            math::is_negligible(top_left) &&
            math::is_negligible(top_right) &&
            math::is_negligible(bottom_right) &&
            math::is_negligible(bottom_left); }

    bool is_zero_or_negative() const {
        return
            math::is_negligible_or_negative(top_left) ||
            math::is_negligible_or_negative(top_right) ||
            math::is_negligible_or_negative(bottom_right) ||
            math::is_negligible_or_negative(bottom_left); }

    static CornerRadius AutoRadius() { return {-1}; }
};

} // xpf