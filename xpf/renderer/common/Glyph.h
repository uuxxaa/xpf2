
#pragma once
#include <vector>
#include <core/Rectangle.h>

namespace xpf {

class ITexture;
class Codepoint;

struct Glyph
{
    std::shared_ptr<ITexture> spTexture;
    rectf_t texture_coordinates;
    float xoffset = 0;
    float yoffset = 0;
    float bearing_x = 0;
    float width = 0;
    float height = 0;
    float advance_x = 0;
    char32_t ch;
    bool is_over_budget = false;

    Glyph() = default;
    Glyph(Glyph&&) = default;
    Glyph(const Glyph&) = default;
    Glyph(char32_t ch, const std::shared_ptr<ITexture>& spTextureIn, const Codepoint& codepoint, float scale);

    Glyph& operator=(Glyph&&) = default;
    Glyph& operator=(const Glyph&) = default;

    float x_start(float x_left) const { return x_left /*+ bearing_x*/ /*- xoffset*/; }
    float y_start(float y_top, float baseline) const { return y_top + baseline + yoffset; }
    float advance_to_right(float x_left) const { return x_left + advance_x; }

    bool is_newline() const { return ch == '\n'; }
};

} // xpf