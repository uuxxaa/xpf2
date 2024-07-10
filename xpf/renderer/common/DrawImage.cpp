#include "Common_Renderer.h"

namespace xpf {

void RenderBatchBuilder::DrawImage(
    float x, float y, float w, float h,
    const std::shared_ptr<ITexture>& spTexture,
    const rectf_t& coords,
    xpf::Color color)
{
    if (spTexture == nullptr)
        return DrawRectangle(x, y, w, h, xpf::Colors::Purple);

    if (!m_vertices.empty() && m_spTexture != spTexture) {
        Flush();
    }

    m_commandId = RenderCommandId::position_color_texture;
    m_spTexture = spTexture;

    v4_t tint = color.get_vec4();
    PushQuad(
        {{x,  y}, tint, coords.top_left() },
        {{x+w, y}, tint, coords.top_right() },
        {{x+w, y+h}, tint, coords.bottom_right() },
        {{x,  y+h}, tint, coords.bottom_left() });
}

} // xpf