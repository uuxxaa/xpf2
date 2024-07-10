#include "Glyph.h"
#include <renderer/ITexture.h>
#include <renderer/common/Font.h>

namespace xpf {

Glyph::Glyph(char32_t ch_arg, const std::shared_ptr<ITexture>& spTextureIn, const Codepoint& codepoint, float scale)
    : spTexture(spTextureIn)
    , texture_coordinates(codepoint.texCoords)
    , xoffset(codepoint.xoffset * scale)
    , yoffset(codepoint.yoffset * scale)
    , bearing_x(codepoint.bearingX * scale)
    , width(codepoint.width * scale)
    , height(codepoint.height * scale)
    , advance_x(codepoint.advance * scale)
    , ch(ch_arg)
{}

} // xpf