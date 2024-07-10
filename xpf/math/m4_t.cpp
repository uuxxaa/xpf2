#include "m4_t.h"
#include <stdexcept>
#include <core/stringex.h>

namespace xpf {

const m4_t m4_t::identity = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};

const float* m4_t::to_4x4_floats() const {
    static_assert(sizeof(m4_t) == sizeof(float)*16);
    return reinterpret_cast<const float*>(this);
}

stringex m4_t::to_string(uint8_t precision) const {
    return xpf::stringex().newline()
        .appendex("[").appendex(m[0][0], precision).appendex(", ").appendex(m[1][0], precision).appendex(", ").appendex(m[2][0], precision).appendex(", ").appendex(m[3][0], precision).appendex(", ").newline()
        .appendex("[").appendex(m[0][1], precision).appendex(", ").appendex(m[1][1], precision).appendex(", ").appendex(m[2][1], precision).appendex(", ").appendex(m[3][1], precision).appendex(", ").newline()
        .appendex("[").appendex(m[0][2], precision).appendex(", ").appendex(m[1][2], precision).appendex(", ").appendex(m[2][2], precision).appendex(", ").appendex(m[3][2], precision).appendex(", ").newline()
        .appendex("[").appendex(m[0][3], precision).appendex(", ").appendex(m[1][3], precision).appendex(", ").appendex(m[2][3], precision).appendex(", ").appendex(m[3][3], precision).appendex("]");
}

m4_t& m4_t::operator*=(const m4_t& o) {
    *this = *this * o;
    return *this;
}

bool m4_t::equals(const m4_t& o) const {
    for (int i = 0; i < 16; i++) {
        if (!math::is_equals_almost(f[i], o.f[i]))
            return false;
    }

    return true;
}

m4_t m4_t::operator*(float value) const {
    m4_t copy;
    for (int i = 0; i < 16; i++) {
        copy.f[i] = f[i] * value;
    }

    return copy;
}

m4_t& m4_t::operator*=(float value) {
    for (int i = 0; i < 16; i++) {
        f[i] *= value;
    }
    return *this;
}

// https://github.com/ARM-software/EndpointAI/blob/master/Kernels/Migrating_to_Helium_from_Neon_Companion_SW/matrix.c
// Tried the link above matrix_multiply_4x4_neon to multiply and transpose the perf on M1 is worse, I am keeping this
m4_t m4_t::multiply(const m4_t& o) const {
    return {
        // row 0
        m00 * o.m00 + m10 * o.m01 + m20 * o.m02 + m30 * o.m03,
        m00 * o.m10 + m10 * o.m11 + m20 * o.m12 + m30 * o.m13,
        m00 * o.m20 + m10 * o.m21 + m20 * o.m22 + m30 * o.m23,
        m00 * o.m30 + m10 * o.m31 + m20 * o.m32 + m30 * o.m33,
        // row 1
        m01 * o.m00 + m11 * o.m01 + m21 * o.m02 + m31 * o.m03,
        m01 * o.m10 + m11 * o.m11 + m21 * o.m12 + m31 * o.m13,
        m01 * o.m20 + m11 * o.m21 + m21 * o.m22 + m31 * o.m23,
        m01 * o.m30 + m11 * o.m31 + m21 * o.m32 + m31 * o.m33,
        // row 2
        m02 * o.m00 + m12 * o.m01 + m22 * o.m02 + m32 * o.m03,
        m02 * o.m10 + m12 * o.m11 + m22 * o.m12 + m32 * o.m13,
        m02 * o.m20 + m12 * o.m21 + m22 * o.m22 + m32 * o.m23,
        m02 * o.m30 + m12 * o.m31 + m22 * o.m32 + m32 * o.m33,
        // row 3
        m03 * o.m00 + m13 * o.m01 + m23 * o.m02 + m33 * o.m03,
        m03 * o.m10 + m13 * o.m11 + m23 * o.m12 + m33 * o.m13,
        m03 * o.m20 + m13 * o.m21 + m23 * o.m22 + m33 * o.m23,
        m03 * o.m30 + m13 * o.m31 + m23 * o.m32 + m33 * o.m33,
    };
}

/*static*/ m4_t m4_t::translation_matrix(float x, float y, float z) {
    return m4_t(
        1, 0, 0, x,
        0, 1, 0, y,
        0, 0, 1, z,
        0, 0, 0, 1
    );
}

/*static*/ m4_t m4_t::rotation_matrix_x(radians_t theta) {
    const float c = xpf::math::cos(theta);
    const float s = xpf::math::sin(theta);
    return m4_t(
        1,  0,  0,  0,
        0,  c, -s,  0,
        0,  s,  c,  0,
        0,  0,  0,  1
    );
}

/*static*/ m4_t m4_t::rotation_matrix_y(radians_t theta) {
    const float c = xpf::math::cos(theta);
    const float s = xpf::math::sin(theta);
    return m4_t(
        c, -s, 0, 0,
        s,  c, 0, 0,
        0,  0, 1, 0,
        0,  0, 0, 1
    );
}

/*static*/ m4_t m4_t::rotation_matrix_z(radians_t theta) {
    const float c = xpf::math::cos(theta);
    const float s = xpf::math::sin(theta);
    return m4_t(
        c, -s, 0, 0,
        s,  c, 0, 0,
        0,  0, 1, 0,
        0,  0, 0, 1
    );
}

/*static*/ m4_t m4_t::scale_matrix(float sx, float sy, float sz) {
    return m4_t(
        sx, 0,  0,  0,
        0,  sy, 0,  0,
        0,  0,  sz, 0,
        0,  0,  0,  1
    );
}

m4_t m4_t::transpose() const {
	return m4_t(
		m00, m01, m02, m03,
		m10, m11, m12, m13,
		m20, m21, m22, m23,
		m30, m31, m32, m33
	);
}

//  Creates an orthographic projection matrix. It maps the right handed cube
//  defined by left, right, bottom, top, back and front onto the screen and
//  z-buffer. You can think of it as a cube you move through world or camera
//  space and everything inside is visible.
//
//  This is slightly different from the traditional glOrtho() and from the linked
//  sources. These functions require the user to negate the last two arguments
//  (creating a left-handed coordinate system). We avoid that here so you can
//  think of this function as moving a right-handed cube through world space.
//
//  The arguments are ordered in a way that for each axis you specify the minimum
//  followed by the maximum. Thats why it's bottom to top and back to front.
//
//  Implementation details:
//
//  To be more exact the right-handed cube is mapped into normalized device
//  coordinates, a left-handed cube where (-1 -1) is the lower left corner,
//  (1, 1) the upper right corner and a z-value of -1 is the nearest point and
//  1 the furthest point. OpenGL takes it from there and puts it on the screen
//  and into the z-buffer.
//
//  Sources:
//
//  https://msdn.microsoft.com/en-us/library/windows/desktop/dd373965(v=vs.85).aspx
//  https://unspecified.wordpress.com/2012/06/21/calculating-the-gluperspective-matrix-and-other-opengl-matrix-maths/
/*static*/ m4_t m4_t::ortho(float left, float right, float bottom, float top, float back, float front) {
    const float l = left, r = right, b = bottom, t = top, n = front, f = back;
    const float tx = -(r + l) / (r - l);
    const float ty = -(t + b) / (t - b);
    const float tz = -(f + n) / (f - n);

    return m4_t(
            2 / (r - l),  0,            0,            tx,
            0,            2 / (t - b),  0,            ty,
            0,            0,            2 / (f - n),  tz,
            0,            0,            0,            1
    );
}

/*static*/ m4_t m4_t::ortho(float left, float right, float bottom, float top) {
    const float l = left, r = right, b = bottom, t = top;
    const float tx = -(r + l) / (r - l);
    const float ty = -(t + b) / (t - b);

    return m4_t(
        2 / (r - l), 0,           0, tx,
        0,           2 / (t - b), 0, ty,
        0,           0,          -1,  0,
        0,           0,           0,  1
    );
}

// Creates a perspective projection matrix for a camera.
//
// The camera is at the origin and looks in the direction of the negative Z axis.
// `near_view_distance` and `far_view_distance` have to be positive and > 0.
// They are distances from the camera eye, not values on an axis.
//
// `near_view_distance` can be small but not 0. 0 breaks the projection and
// everything ends up at the max value (far end) of the z-buffer. Making the
// z-buffer useless.
//
// The matrix is the same as `gluPerspective()` builds. The view distance is
// mapped to the z-buffer with a reciprocal function (1/x). Therefore the z-buffer
// resolution for near objects is very good while resolution for far objects is
// limited.
//
// Sources:
//
// https://unspecified.wordpress.com/2012/06/21/calculating-the-gluperspective-matrix-and-other-opengl-matrix-maths/
/*static*/ m4_t m4_t::perspective(
    float vertical_field_of_view_in_deg,
    float aspect_ratio,
    float near_view_distance,
    float far_view_distance) {
	float fovy_in_rad = vertical_field_of_view_in_deg / 180 * math::PI;
	float f = 1.0f / tanf(fovy_in_rad / 2.0f);
	float ar = aspect_ratio;
	float nd = near_view_distance, fd = far_view_distance;

	return m4_t(
		 f / ar,           0,                0,                0,
		 0,                f,                0,                0,
		 0,                0,               (fd+nd)/(nd-fd),  (2*fd*nd)/(nd-fd),
		 0,                0,               -1,                0
	);
}

// Creates a matrix to rotate around an axis by a given angle. The axis doesn't
// need to be normalized.
// Sources:
// https://en.wikipedia.org/wiki/Rotation_matrix#Rotation_matrix_from_axis_and_angle
m4_t m4_t::rotation(float angle_in_rad, v3_t axis) const {
	const v3_t normalized_axis = axis.normal();
	const float x = normalized_axis.x, y = normalized_axis.y, z = normalized_axis.z;
	const float c = cosf(angle_in_rad), s = sinf(angle_in_rad);

	return m4_t(
		c + x*x*(1-c),            x*y*(1-c) - z*s,      x*z*(1-c) + y*s,  0,
		    y*x*(1-c) + z*s,  c + y*y*(1-c),            y*z*(1-c) - x*s,  0,
		    z*x*(1-c) - y*s,      z*y*(1-c) + x*s,  c + z*z*(1-c),        0,
		    0,                        0,                    0,            1
	);
}

// Multiplies a 4x4 matrix with a 3D vector representing a point in 3D space.
// Before the matrix multiplication the vector is first expanded to a 4D vector
// (x, y, z, 1). After the multiplication the vector is reduced to 3D again by
// dividing through the 4th component (if it's not 0 or 1).
/*static*/ v3_t m4_t::mul_pos(const m4_t& matrix, v3_t position) {
	v3_t result = v3_t(
		matrix.m00 * position.x + matrix.m10 * position.y + matrix.m20 * position.z + matrix.m30,
		matrix.m01 * position.x + matrix.m11 * position.y + matrix.m21 * position.z + matrix.m31,
		matrix.m02 * position.x + matrix.m12 * position.y + matrix.m22 * position.z + matrix.m32
	);

	float w = matrix.m03 * position.x + matrix.m13 * position.y + matrix.m23 * position.z + matrix.m33;
	if (w != 0 && w != 1)
		return result / 3;

	return result;
}

// Multiplies a 4x4 matrix with a 3D vector representing a direction in 3D space.
// Before the matrix multiplication the vector is first expanded to a 4D vector
// (x, y, z, 0). For directions the 4th component is set to 0 because directions
// are only rotated, not translated. After the multiplication the vector is
// reduced to 3D again by dividing through the 4th component (if it's not 0 or
// 1). This is necessary because the matrix might contains something other than
// (0, 0, 0, 1) in the bottom row which might set w to something other than 0
// or 1.
/*static*/ v3_t m4_t::mul_dir(const m4_t& matrix, v3_t direction) {
	v3_t result = v3_t(
		matrix.m00 * direction.x + matrix.m10 * direction.y + matrix.m20 * direction.z,
		matrix.m01 * direction.x + matrix.m11 * direction.y + matrix.m21 * direction.z,
		matrix.m02 * direction.x + matrix.m12 * direction.y + matrix.m22 * direction.z
	);

	float w = matrix.m03 * direction.x + matrix.m13 * direction.y + matrix.m23 * direction.z;
	if (w != 0 && w != 1)
		return result / w;

	return result;
}

radians_t m4_t::get_rotation_angle_z() const {
    // https://stackoverflow.com/questions/15022630/how-to-calculate-the-angle-from-rotation-matrix
    // col0    col1 col2 col3
    // m[0][0] ...
    // m[0][1]
    // ...
    return xpf::math::atan2(m01, m00);
}

v3_t m4_t::get_position() const {
    return {m30, m31, m32}; // col3
}

/*static*/ v4_t m4_t::multiply(const m4_t& m, float x, float y, float z, float w) {
    return {
        m.m00 * x + m.m10 * y + m.m20 * z + m.m30 * w,
        m.m01 * x + m.m11 * y + m.m21 * z + m.m31 * w,
        m.m02 * x + m.m12 * y + m.m22 * z + m.m32 * w,
        m.m03 * x + m.m13 * y + m.m23 * z + m.m33 * w,
    };
}

/*static*/ v3_t m4_t::multiply(const m4_t& m, float x, float y, float z) {
    return {
        m.m00 * x + m.m10 * y + m.m20 * z,
        m.m01 * x + m.m11 * y + m.m21 * z,
        m.m02 * x + m.m12 * y + m.m22 * z,
    };
}

/*static*/ v2_t m4_t::multiply(const m4_t& m, float x, float y) {
    return {
        m.m00 * x + m.m10 * y,
        m.m01 * x + m.m11 * y,
    };
}

m4_t m4_t::inverse() const {
    throw std::runtime_error("m4_t::inverse not implemented");
/*
    float Coef00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
    float Coef02 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
    float Coef03 = m[1][2] * m[2][3] - m[2][2] * m[1][3];

    float Coef04 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
    float Coef06 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
    float Coef07 = m[1][1] * m[2][3] - m[2][1] * m[1][3];

    float Coef08 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
    float Coef10 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
    float Coef11 = m[1][1] * m[2][2] - m[2][1] * m[1][2];

    float Coef12 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
    float Coef14 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
    float Coef15 = m[1][0] * m[2][3] - m[2][0] * m[1][3];

    float Coef16 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
    float Coef18 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
    float Coef19 = m[1][0] * m[2][2] - m[2][0] * m[1][2];

    float Coef20 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
    float Coef22 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
    float Coef23 = m[1][0] * m[2][1] - m[2][0] * m[1][1];

    v4_t Fac0(Coef00, Coef00, Coef02, Coef03);
    v4_t Fac1(Coef04, Coef04, Coef06, Coef07);
    v4_t Fac2(Coef08, Coef08, Coef10, Coef11);
    v4_t Fac3(Coef12, Coef12, Coef14, Coef15);
    v4_t Fac4(Coef16, Coef16, Coef18, Coef19);
    v4_t Fac5(Coef20, Coef20, Coef22, Coef23);

    v4_t Vec0(m[1][0], m[0][0], m[0][0], m[0][0]);
    v4_t Vec1(m[1][1], m[0][1], m[0][1], m[0][1]);
    v4_t Vec2(m[1][2], m[0][2], m[0][2], m[0][2]);
    v4_t Vec3(m[1][3], m[0][3], m[0][3], m[0][3]);

    v4_t Inv0(Vec1 * Fac0 - Vec2 * Fac1 + Vec3 * Fac2);
    v4_t Inv1(Vec0 * Fac0 - Vec2 * Fac3 + Vec3 * Fac4);
    v4_t Inv2(Vec0 * Fac1 - Vec1 * Fac3 + Vec3 * Fac5);
    v4_t Inv3(Vec0 * Fac2 - Vec1 * Fac4 + Vec2 * Fac5);

    v4_t SignA(+1, -1, +1, -1);
    v4_t SignB(-1, +1, -1, +1);
    m4_t inverse(Inv0 * SignA, Inv1 * SignB, Inv2 * SignA, Inv3 * SignB);

    v4_t Row0(inverse[0][0], inverse[1][0], inverse[2][0], inverse[3][0]);

    v4_t Dot0(m[0] * Row0);
    float Dot1 = (Dot0.x + Dot0.y) + (Dot0.z + Dot0.w);

    float one_over_determinant = 1.0f / Dot1;

    return inverse * one_over_determinant;
*/
}

} // xpf