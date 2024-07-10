#pragma once
#include "v2_t.h"
#include "v3_t.h"
#include "m4_t.h"

namespace xpf {

class stringex;
struct mat4_t;
struct dont_initialize { };

struct m3_t {
    union {
        // The first index is the column index, the second the row index. The memory
        // layout of nested arrays in c++ matches the memory layout expected by OpenGL.
        v3_t m[3];
        float f[9];
        // OpenGL expects the first 4 floats to be the first column of the matrix.
        // So we need to define the named members column by column for the names to
        // match the memory locations of the array elements.
        struct {
            float m00, m01, m02; // col1
            float m10, m11, m12; // col2
            float m20, m21, m22; // col3
        };
    };

    constexpr m3_t()
        : m00(0), m01(0), m02(0)
        , m10(0), m11(0), m12(0)
        , m20(0), m21(0), m22(0)
    {}

    constexpr m3_t(
        float m00, float m10, float m20,
        float m01, float m11, float m21,
        float m02, float m12, float m22)
        : m00(m00), m01(m01), m02(m02)
        , m10(m10), m11(m11), m12(m12)
        , m20(m20), m21(m21), m22(m22)
    { }

    constexpr m3_t(v3_t col0, v3_t col1, v3_t col2)
        : m{col0, col1, col2}
    { }

    static const m3_t identity;

    static m3_t translation_matrix(float x, float y);
    static m3_t rotation_matrix(radians_t theta);
    static m3_t scale_matrix(float sx, float sy);
    static m3_t rotate_about(float centerX, float centerY, radians_t theta);

    m3_t operator*(float f) const{ return multiply(f); }
    m3_t operator*(const m3_t& o) const { return multiply(o); }
    m3_t operator/(float f) const { return divide(f); }
    m3_t operator/(const m3_t& o) const { return divide(o); }

    v2_t operator*(const v2_t rhs) const;

    m3_t& operator*=(float f);
    m3_t& operator*=(const m3_t& o);

    bool equals(const m3_t& o, float epsilon = xpf::math::Epsilon) const;

    v3_t& operator[](size_t i) { return m[i]; }
    const v3_t& operator[](size_t i) const { return m[i]; }
    m4_t to_4x4() const;
    void to_4x4(m4_t& result) const;
    const float* to_4x4_floats() const;
    stringex to_string(uint8_t precision = 3) const;

    v2_t to_position() const { return {m02, m12}; }
    radians_t rotation_angle() const;

    m3_t multiply(float f) const;
    m3_t multiply(const m3_t& o) const;
    v2_t multiply(float x, float y) const;
    m3_t divide(float f) const { return multiply(1.0f / f); }
    m3_t divide(const m3_t& o) const { return multiply(o.inverse()); }

    m3_t inverse() const;
};

} // xpf