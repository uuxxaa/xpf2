#include "m3_t.h"
#include "m4_t.h"
#include <core/stringex.h>

namespace xpf {

const m3_t m3_t::identity = m3_t(
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
);

/*static*/ m3_t m3_t::translation_matrix(float x, float y) {
    return {
        1.0f, 0.0f, x,
        0.0f, 1.0f, y,
        0.0f, 0.0f, 1.0f,
    };
}

/*static*/ m3_t m3_t::rotation_matrix(radians_t theta)
{
    const float cosTheta = xpf::math::cos(theta);
    const float sinTheta = xpf::math::sin(theta);
    return {
        cosTheta, -sinTheta, 0.0f,
        sinTheta, +cosTheta, 0.0f,
        0.0f,     0.0f,      1.0f,
    };
}

/*static*/ m3_t m3_t::scale_matrix(float sx, float sy)
{
    return {
        sx,   0.0f, 0.0f,
        0.0f, sy,   0.0f,
        0.0f, 0.0f, 1.0f,
    };
}

/*static*/ m3_t m3_t::rotate_about(float centerX, float centerY, radians_t theta)
{
    // https://math.stackexchange.com/questions/2093314/rotation-matrix-of-rotation-around-a-point-other-than-the-origin
    // return TranslationMatrix(centerX, centerY)
    //        * RotationMatrix(theta)
    //        * TranslationMatrix(-centerX, -centerY);
    const float cosTheta = xpf::math::cos(theta);
    const float sinTheta = xpf::math::sin(theta);
    return {
        cosTheta, -sinTheta, -centerX * cosTheta + centerY * sinTheta + centerX,
        sinTheta, +cosTheta, -centerX * sinTheta - centerY * cosTheta + centerY,
        0.0f, 0.0f, 1.0f,
    };
}

//-------------

m4_t m3_t::to_4x4() const
{
    return m4_t(
        m00,  m10,  0.0f, m20,
        m01,  m11,  0.0f, m21,
        m02,  m12,  1.0f, m22,
        0.0f, 0.0f, 0.0f, 1.0f);
}

void m3_t::to_4x4(m4_t& result) const {
    result.m00 = m00; result.m10 = m10; result.m20 = 0; result.m30 = m20;
    result.m01 = m01; result.m11 = m11; result.m21 = 0; result.m31 = m21;
    result.m02 = m02; result.m12 = m12; result.m22 = 1; result.m32 = m22;
    result.m03 = 0;   result.m13 = 0;   result.m23 = 0; result.m33 = 1;
}

const float* m3_t::to_4x4_floats() const {
    thread_local m4_t s_m4 = m4_t::identity;
    s_m4.m00 = m00; s_m4.m10 = m10;       s_m4.m30 = m20;
    s_m4.m01 = m01; s_m4.m11 = m11;       s_m4.m31 = m21;
    s_m4.m02 = m02; s_m4.m12 = m12;       s_m4.m32 = m22;
    return s_m4.to_4x4_floats();
}

xpf::stringex m3_t::to_string(uint8_t precision) const {
    return xpf::stringex()
        .appendex("[").appendex(m[0][0], precision).appendex(", ").appendex(m[1][0], precision).appendex(", ").appendex(m[2][0], precision).appendex("]").newline()
        .appendex("[").appendex(m[0][1], precision).appendex(", ").appendex(m[1][1], precision).appendex(", ").appendex(m[2][1], precision).appendex("]").newline()
        .appendex("[").appendex(m[0][2], precision).appendex(", ").appendex(m[1][2], precision).appendex(", ").appendex(m[2][2], precision).appendex("]").newline();
}

bool m3_t::equals(const m3_t& o, float epsilon) const {
    for (int col = 0; col < 3; col++) {
        for (int row = 0; row < 3; row++) {
            if (!math::is_equals_almost(m[col][row], o.m[col][row], epsilon))
                return false;
        }
    }

    return true;
}

radians_t m3_t::rotation_angle() const {
    // https://stackoverflow.com/questions/15022630/how-to-calculate-the-angle-from-rotation-matrix
    return xpf::math::atan2(m10, m00);
}

m3_t& m3_t::operator*=(const m3_t& o) {
    *this = *this * o;
    return *this;
}

v2_t m3_t::operator*(const v2_t rhs) const {
    return multiply(rhs.x, rhs.y);
}

v2_t m3_t::multiply(float x, float y) const {
    return {
        m00 * x + m01 * y + m02,
        m10 * x + m11 * y + m12
    };
}

m3_t m3_t::multiply(float f) const {
    m3_t result;
    for (int col = 0; col < 3; col++) {
        for (int row = 0; row < 3; row++) {
            result.m[col][row] = m[col][row] * f;
        }
    }

    return result;
}

m3_t m3_t::multiply(const m3_t& o) const {
    return m3_t(
        // row 0
        m00 * o.m00 + m01 * o.m10 + m02 * o.m20,
        m00 * o.m01 + m01 * o.m11 + m02 * o.m21,
        m00 * o.m02 + m01 * o.m12 + m02 * o.m22,

        // row 1
        m10 * o.m00 + m11 * o.m10 + m12 * o.m20,
        m10 * o.m01 + m11 * o.m11 + m12 * o.m21,
        m10 * o.m02 + m11 * o.m12 + m12 * o.m22,

        // row 2
        m20 * o.m00 + m21 * o.m10 + m22 * o.m20,
        m20 * o.m01 + m21 * o.m11 + m22 * o.m21,
        m20 * o.m02 + m21 * o.m12 + m22 * o.m22);
}

m3_t m3_t::inverse() const {
    float one_over_determinant = 1.0f / (
        + m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2])
        - m[1][0] * (m[0][1] * m[2][2] - m[2][1] * m[0][2])
        + m[2][0] * (m[0][1] * m[1][2] - m[1][1] * m[0][2]));

    m3_t inverse;
    inverse[0][0] = + (m[1][1] * m[2][2] - m[2][1] * m[1][2]) * one_over_determinant;
    inverse[1][0] = - (m[1][0] * m[2][2] - m[2][0] * m[1][2]) * one_over_determinant;
    inverse[2][0] = + (m[1][0] * m[2][1] - m[2][0] * m[1][1]) * one_over_determinant;
    inverse[0][1] = - (m[0][1] * m[2][2] - m[2][1] * m[0][2]) * one_over_determinant;
    inverse[1][1] = + (m[0][0] * m[2][2] - m[2][0] * m[0][2]) * one_over_determinant;
    inverse[2][1] = - (m[0][0] * m[2][1] - m[2][0] * m[0][1]) * one_over_determinant;
    inverse[0][2] = + (m[0][1] * m[1][2] - m[1][1] * m[0][2]) * one_over_determinant;
    inverse[1][2] = - (m[0][0] * m[1][2] - m[1][0] * m[0][2]) * one_over_determinant;
    inverse[2][2] = + (m[0][0] * m[1][1] - m[1][0] * m[0][1]) * one_over_determinant;

    return inverse;
}

} // xpf