#pragma once
#include "v2_t.h"
#include "v3_t.h"
#include "v4_t.h"
#include "xpfmath.h"

namespace xpf {

class stringex;

struct m4_t {
    union {
        // The first index is the column index, the second the row index. The memory
        // layout of nested arrays in c++ matches the memory layout expected by OpenGL.
        v4_t m[4];
        float f[16];
        // OpenGL expects the first 4 floats to be the first column of the matrix.
        // So we need to define the named members column by column for the names to
        // match the memory locations of the array elements.
        struct {
            float m00, m01, m02, m03;
            float m10, m11, m12, m13;
            float m20, m21, m22, m23;
            float m30, m31, m32, m33;
        };
    };

    static const m4_t identity;

#pragma region constructors
    constexpr m4_t()
        : m00(0), m01(0), m02(0), m03(0)
        , m10(0), m11(0), m12(0), m13(0)
        , m20(0), m21(0), m22(0), m23(0)
        , m30(0), m31(0), m32(0), m33(0)
    {}

    constexpr m4_t(
        float m00, float m10, float m20, float m30,
        float m01, float m11, float m21, float m31,
        float m02, float m12, float m22, float m32,
        float m03, float m13, float m23, float m33)
        : m00(m00), m01(m01), m02(m02), m03(m03)
        , m10(m10), m11(m11), m12(m12), m13(m13)
        , m20(m20), m21(m21), m22(m22), m23(m23)
        , m30(m30), m31(m31), m32(m32), m33(m33)
    {}

    constexpr m4_t(v4_t col0, v4_t col1, v4_t col2, v4_t&& col3)
        : m{col0, col1, col2, col3}
    {}

#pragma endregion

    static m4_t translation_matrix(float x, float y, float z = 0);
    static m4_t rotation_matrix_x(radians_t theta);
    static m4_t rotation_matrix_y(radians_t theta);
    static m4_t rotation_matrix_z(radians_t theta);
    static m4_t scale_matrix(float sx, float sy, float sz = 1.0f);
    static m4_t ortho(float left, float right, float bottom, float top, float back, float front);
    static m4_t ortho(float left, float right, float bottom, float top);
    static m4_t perspective(float vertical_field_of_view_in_deg, float aspect_ratio, float near_view_distance, float far_view_distance);

    v4_t& operator[](size_t i) { return m[i]; }
    const v4_t& operator[](size_t i) const { return m[i]; }

    m4_t operator*(float f) const;
    m4_t operator*(const m4_t& o) const { return multiply(o); }
    m4_t operator/(float f) const { return *this * (1 / f); }
    m4_t operator/(const m4_t& o) const { return *this * o.inverse(); }

    m4_t& operator*=(float f);
    m4_t& operator*=(const m4_t& o);

    bool equals(const m4_t& o) const;

    const float* to_4x4_floats() const;
    xpf::stringex to_string(uint8_t precision = 3) const;

    static m4_t transpose(const m4_t& m) { return m.transpose(); }
    m4_t transpose() const;
    m4_t rotation(float angle_in_rad, v3_t axis) const;

    radians_t get_rotation_angle_z() const;
    v3_t get_position() const;

    static v3_t mul_pos(const m4_t& matrix, v3_t position);
    static v3_t mul_dir(const m4_t& matrix, v3_t direction);

#pragma region matrix operations
    v4_t operator*(const v4_t& v) const { return multiply(*this, v); }
    v3_t operator*(const v3_t& v) const { return multiply(*this, v); }
    v2_t operator*(const v2_t& v) const { return multiply(*this, v.x, v.y); } // this seems wrong...

    m4_t multiply(const m4_t& m) const;
    static m4_t multiply(const m4_t& m, const m4_t& o) { return m.multiply(o); }

    static v4_t multiply(const m4_t& m, v4_t v);
    static v4_t multiply(const m4_t& m, float x, float y, float z, float w);
    v4_t multiply(v4_t v) const { return multiply(*this, v.x, v.y, v.z, v.w); }

    static v3_t multiply(const m4_t& m, v3_t v);
    static v3_t multiply(const m4_t& m, float x, float y, float z);
    v3_t multiply(v3_t v) const { return multiply(*this, v.x, v.y, v.z); }

    static v2_t multiply(const m4_t& m, float x, float y);
    static v2_t multiply(const m4_t& m, v2_t v) { return multiply(m, v.x, v.y); }
    v2_t multiply(v2_t v) const { return multiply(*this, v.x, v.y); }

    m4_t inverse() const;
#pragma endregion
};

} // xpdf