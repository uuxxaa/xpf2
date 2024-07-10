#pragma once
#include <stdint.h>
#include <core/stringex.h>
#include <math/v3_t.h>
#include <math/v4_t.h>
#include <math/xpfrandom.h>
// #include <core/xpfmath.h>

namespace xpf {

struct Color
{
    union {
        struct {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;
        };

        uint32_t rgba;
    };

    Color() : Color(255,255,255,0) { static_assert(sizeof(Color) == 4); }
    Color(uint8_t gray, uint8_t alpha) : Color(gray, gray, gray, alpha) { }
    Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) : r(red), g(green), b(blue), a(alpha) { }
    Color(const v4_t& v) : Color(v.r * 255, v.g * 255, v.b * 255, v.a * 255) {}
    Color(uint32_t argb)
        : Color(
            uint8_t(argb >> 16),
            uint8_t(argb >> 8) ,
            uint8_t(argb),
            uint8_t(argb >> 24)) { }

    static Color from_gray(uint8_t gray) { return Color(gray, gray, gray, 255); }

    constexpr static float s_one_over_255 = 1.0f / 255.0f;
    static float to_color_fraction(uint8_t v) { return float(v) * s_one_over_255; }

    float rf() const { return to_color_fraction(r); }
    float gf() const { return to_color_fraction(g); }
    float bf() const { return to_color_fraction(b); }
    float af() const { return to_color_fraction(a); }

    Color operator*(float t) const { return Color(r*t, g*t, b*t, a*t); }
    Color operator+(Color other) const { return Color(r + other.r, g + other.g, b + other.b, a + other.a); }
    Color operator-(Color c) const { return Color(r - c.r, g - c.g, b - c.b, a - c.a); }
    Color& operator*=(float t) { r *= t; g *= t; b *= t; a *= t; return *this; }
    Color& operator+=(Color other) { r +=other.r; g += other.g; b += other.b; a += other.a; return *this; }
    Color& operator=(const v4_t& c) { r = c.r; g = c.g; b = c.b; a = c.a; return *this; }
    bool operator==(Color c) const { return r == c.r && g == c.g && b == c.b && a == c.a; }
    bool operator!=(Color c) const { return !(*this == c); }

    static Color from_float(float red, float green, float blue, float alpha = 1.0) { return Color(red * 255, green * 255, blue * 255, alpha * 255); }

    Color get_r(uint8_t alpha = 255) const { return { r, 0, 0, alpha }; }
    Color get_g(uint8_t alpha = 255) const { return { 0, g, 0, alpha }; }
    Color get_b(uint8_t alpha = 255) const { return { 0, 0, b, alpha }; }
    Color get_rgb(uint8_t alpha = 255) const { return { r, g, b, alpha }; }

    Color get_rg(uint8_t blue = 0, uint8_t alpha = 255) const { return { r, g, blue, alpha }; }
    Color get_gr(uint8_t blue = 0, uint8_t alpha = 255) const { return { g, r, blue, alpha }; }

    Color get_rb(uint8_t green = 0, uint8_t alpha = 255) const { return { r, green, b, alpha }; }
    Color get_br(uint8_t green = 0, uint8_t alpha = 255) const { return { b, green, r, alpha }; }

    Color get_gb(uint8_t red = 0, uint8_t alpha = 255) const { return { red, g, b, alpha }; }
    Color get_bg(uint8_t red = 0, uint8_t alpha = 255) const { return { red, b, g, alpha }; }

    Color get_rrr(uint8_t alpha = 255) const { return { r, r, r, alpha}; }
    Color get_ggg(uint8_t alpha = 255) const { return { g, g, g, alpha}; }
    Color get_bbb(uint8_t alpha = 255) const { return { b, b, b, alpha}; }

    v4_t get_vec4() const { return {to_color_fraction(r), to_color_fraction(g), to_color_fraction(b), to_color_fraction(a) }; }
    v3_t get_vec3() const { return {to_color_fraction(r), to_color_fraction(g), to_color_fraction(b) }; }

    Color invert() const { return { uint8_t(255 - r), uint8_t(255 - g), uint8_t(255 - b), a}; }
    Color invert(uint8_t alpha) const { return { uint8_t(255 - r), uint8_t(255 - g), uint8_t(255 - b), alpha }; }

    Color with_alpha(uint8_t alpha) const { return Color(r, g, b, alpha); }
    Color multiply_rgb(float m) const { return { uint8_t(r * m), uint8_t(g * m), uint8_t(b * m), a }; }
    Color multiply_rgb(Color m) const { return { uint8_t(r * m.r), uint8_t(g * m.g), uint8_t(b * m.b), a }; }
    Color multiply_a(float m) const { return { r, g, b, uint8_t(a * m) }; }

    bool is_transparent() const { return a == 0; }

    Color mix(Color bcolor, double foregroundOpacity) {
        float backgroundOpacity = 1 - foregroundOpacity;

        return Color(
            (uint8_t)(foregroundOpacity * r + backgroundOpacity * bcolor.r),
            (uint8_t)(foregroundOpacity * g + backgroundOpacity * bcolor.g),
            (uint8_t)(foregroundOpacity * b + backgroundOpacity * bcolor.g),
            255);
    }

    xpf::stringex to_hex_rgb() const {
        return "#" + xpf::stringex::to_hex(r, 2) + xpf::stringex::to_hex(g, 2) + xpf::stringex::to_hex(b, 2);
    }

    static bool try_get_color(std::string_view name, xpf::Color& Color);
    static xpf::Color to_color(std::string_view name);
};

struct Colors
{
    // wpf Colors --------
    static Color AliceBlue;
    static Color AntiqueWhite;
    static Color Aqua;
    static Color Aquamarine;
    static Color Azure;
    static Color Beige;
    static Color Bisque;
    static Color Black;
    static Color BlanchedAlmond;
    static Color Blue;
    static Color BlueViolet;
    static Color Brown;
    static Color BurlyWood;
    static Color CadetBlue;
    static Color Chartreuse;
    static Color Chocolate;
    static Color Coral;
    static Color CornflowerBlue;
    static Color Cornsilk;
    static Color Crimson;
    static Color Cyan;
    static Color DarkBlue;
    static Color DarkCyan;
    static Color DarkGoldenrod;
    static Color DarkGray;
    static Color DarkGreen;
    static Color DarkKhaki;
    static Color DarkMagenta;
    static Color DarkOliveGreen;
    static Color DarkOrange;
    static Color DarkOrchid;
    static Color DarkRed;
    static Color DarkSalmon;
    static Color DarkSeaGreen;
    static Color DarkSlateBlue;
    static Color DarkSlateGray;
    static Color DarkTurquoise;
    static Color DarkViolet;
    static Color DeepPink;
    static Color DeepSkyBlue;
    static Color DimGray;
    static Color DodgerBlue;
    static Color Firebrick;
    static Color FloralWhite;
    static Color ForestGreen;
    static Color Fuchsia;
    static Color Gainsboro;
    static Color GhostWhite;
    static Color Gold;
    static Color Goldenrod;
    static Color Gray;
    static Color Green;
    static Color GreenYellow;
    static Color Honeydew;
    static Color HotPink;
    static Color IndianRed;
    static Color Indigo;
    static Color Ivory;
    static Color Khaki;
    static Color Lavender;
    static Color LavenderBlush;
    static Color LawnGreen;
    static Color LemonChiffon;
    static Color LightBlue;
    static Color LightCoral;
    static Color LightCyan;
    static Color LightGoldenrodYellow;
    static Color LightGray;
    static Color LightGreen;
    static Color LightPink;
    static Color LightSalmon;
    static Color LightSeaGreen;
    static Color LightSkyBlue;
    static Color LightSlateGray;
    static Color LightSteelBlue;
    static Color LightYellow;
    static Color Lime;
    static Color LimeGreen;
    static Color Linen;
    static Color Magenta;
    static Color Maroon;
    static Color MediumAquamarine;
    static Color MediumBlue;
    static Color MediumOrchid;
    static Color MediumPurple;
    static Color MediumSeaGreen;
    static Color MediumSlateBlue;
    static Color MediumSpringGreen;
    static Color MediumTurquoise;
    static Color MediumVioletRed;
    static Color MidnightBlue;
    static Color MintCream;
    static Color MistyRose;
    static Color Moccasin;
    static Color NavajoWhite;
    static Color Navy;
    static Color OldLace;
    static Color Olive;
    static Color OliveDrab;
    static Color Orange;
    static Color OrangeRed;
    static Color Orchid;
    static Color PaleGoldenrod;
    static Color PaleGreen;
    static Color PaleTurquoise;
    static Color PaleVioletRed;
    static Color PapayaWhip;
    static Color PeachPuff;
    static Color Peru;
    static Color Pink;
    static Color Plum;
    static Color PowderBlue;
    static Color Purple;
    static Color Red;
    static Color RosyBrown;
    static Color RoyalBlue;
    static Color SaddleBrown;
    static Color Salmon;
    static Color SandyBrown;
    static Color SeaGreen;
    static Color SeaShell;
    static Color Sienna;
    static Color Silver;
    static Color SkyBlue;
    static Color SlateBlue;
    static Color SlateGray;
    static Color Snow;
    static Color SpringGreen;
    static Color SteelBlue;
    static Color Tan;
    static Color Teal;
    static Color Thistle;
    static Color Tomato;
    static Color Transparent;
    static Color Turquoise;
    static Color Violet;
    static Color Wheat;
    static Color White;
    static Color WhiteSmoke;
    static Color Yellow;
    static Color YellowGreen;
    // xpf Colors -------------------
    static Color XpfBlack;
    static Color XpfGray;
    static Color XpfWhite;
    static Color XpfRed;
    static Color Honey;
    static Color BlueSuede;

    static Color Random() { return Color(random::random_255(), random::random_255(), random::random_255()); }
};

} // xpf

namespace xpf::math {

template <>
inline Color lerp(Color a, Color b, float t) {
    if (t<0.0f) t = 0.0f;
    if (t>1.0f) t = 1.0f;

    const float kt = 1.0f - t;
    return Color(
        a.r * kt + b.r * t,
        a.g * kt + b.g * t,
        a.b * kt + b.b * t,
        a.a * kt + b.a * t);
}

template <>
inline Color midpoint(Color a, Color b) {
    return Color(
        (a.r + b.r) * 0.5f,
        (a.g + b.g) * 0.5f,
        (a.b + b.b) * 0.5f,
        (a.a + b.a) * 0.5f);
}

} // xpf::Math

namespace xpf {

template<> struct math_trait<xpf::Color>
{
    static xpf::Color lerp(xpf::Color a, xpf::Color b, float t) { return xpf::math::lerp(a, b, t); }
};

} // xpf
