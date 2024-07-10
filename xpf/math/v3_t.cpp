#include "v3_t.h"
#include "v2_t.h"
#include <cmath>
#include <limits>

namespace xpf {

static_assert(sizeof(v3_t) == 12, "must be 12");

v3_t v3_t::normal() const {
    float length = magnitude_squared();
    if (length > xpf::math::Epsilon) {
        length = std::sqrtf(length);
        return {x / length, y / length, z / length};
    }

    return {};
}

/*static*/ v3_t v3_t::project(v3_t v, v3_t onto) {
    return mul(onto, dot(v, onto) / dot(onto, onto));
}

/*static*/ v3_t v3_t::cross(v3_t a, v3_t b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

/*static*/ float v3_t::angle_between(v3_t a, v3_t b) {
    return acosf(dot(a, b) / (a.length() * b.length()));
}

/*static*/ float v3_t::Distance(v3_t a, v3_t b) {
    return sqrtf(DistanceSquared(a, b));
}

/*static*/ float v3_t::DistanceSquared(v3_t a, v3_t b) {
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    const float dz = a.z - b.z;
    return (dx * dx + dy * dy + + dz * dz);
}

std::string v3_t::to_string() const {
    return "{x:" + std::to_string(x) + " y:" + std::to_string(y) + " z:" + std::to_string(z) + "}";
}

} // xpf