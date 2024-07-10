#include "RenderBatchBuilder.h"
#include "FormattedText.h"
#include <core/Quad.h>
#include <math/geometry.h>

namespace xpf {

void RenderBatchBuilder::AppendCommand(const std::shared_ptr<RenderCommand>& spCmd)
{
    m_commands.push_back(spCmd);
}

RenderBatch RenderBatchBuilder::Build()
{
    Flush();
    return {std::move(m_commands)};
}

void RenderBatchBuilder::Flush()
{
    if (!m_vertices.empty())
    {
        m_commands.push_back(std::make_shared<RenderDrawCommand>(
            m_commandId, std::move(m_vertices), m_vertex_count, m_spTexture
        ));

        m_vertices.clear();
    }

    m_vertex_count = 0;
    m_spTexture = nullptr;
    m_commandId = RenderCommandId::position;
}

void RenderBatchBuilder::RunAction(std::function<RenderBatch()>&& fn)
{
    Flush();
    m_commands.push_back(std::make_shared<RenderCallbackCommand>(std::move(fn)));
}

StackGuard RenderBatchBuilder::Transform(const m4_t& transform, bool multiply)
{
    Flush();
    m4_t prevTransform = std::move(m_currentTransform);
    if (multiply) [[likely]]
        m_currentTransform = prevTransform * transform;
    else
        m_currentTransform = transform;

    m_commands.push_back(std::make_shared<RenderTransformCommand>(std::move(transform), multiply ? RenderTransformCommand::MultiplyPush : RenderTransformCommand::Push));
    return StackGuard([this, prevTransform = std::move(prevTransform)]()
    {
        Flush();
        m_currentTransform = std::move(prevTransform);
        m_commands.push_back(std::make_shared<RenderTransformCommand>(m4_t::identity, RenderTransformCommand::Pop));
    });
}

StackGuard RenderBatchBuilder::TranslateTransfrom(float x, float y)
{
    if (xpf::math::is_negligible(x) && xpf::math::is_negligible(y)) return StackGuard();

    Flush();

    m4_t translation = m4_t::translation_matrix(x, y);
    m4_t prevTransform = std::move(m_currentTransform);
    m_currentTransform = prevTransform * translation;

    m_commands.push_back(std::make_shared<RenderTransformCommand>(std::move(translation), RenderTransformCommand::MultiplyPush));
    return StackGuard([this, prevTransform = std::move(prevTransform)]()
    {
        Flush();
        m_currentTransform = std::move(prevTransform);
        m_commands.push_back(std::make_shared<RenderTransformCommand>(m4_t::identity, RenderTransformCommand::Pop));
    });
}

StackGuard RenderBatchBuilder::RotateTransform(radians_t angle)
{
    if (xpf::math::is_negligible(angle)) return StackGuard();

    Flush();

    m4_t rotation = m4_t::rotation_matrix_z(angle);
    m4_t prevTransform = std::move(m_currentTransform);
    m_currentTransform = prevTransform * rotation;

    m_commands.push_back(std::make_shared<RenderTransformCommand>(std::move(rotation), RenderTransformCommand::MultiplyPush));
    return StackGuard([this, prevTransform = std::move(prevTransform)]()
    {
        Flush();
        m_currentTransform = std::move(prevTransform);
        m_commands.push_back(std::make_shared<RenderTransformCommand>(m4_t::identity, RenderTransformCommand::Pop));
    });
}

const m4_t& RenderBatchBuilder::GetCurrentTransform() const
{
    return m_currentTransform;
}

StackGuard RenderBatchBuilder::Clip(rectui_t clipRegion)
{
    Flush();

    rectui_t prevClip = std::move(m_currentClipRegion);
    m_currentClipRegion = prevClip.intersection(clipRegion);

    m_commands.push_back(std::make_shared<RenderClipCommand>(m_currentClipRegion, RenderClipCommand::Push));
    return StackGuard([this, prevClip = std::move(prevClip)]()
    {
        Flush();
        m_currentClipRegion = std::move(prevClip);
        m_commands.push_back(std::make_shared<RenderClipCommand>(m_currentClipRegion, RenderClipCommand::Pop));
    });
}

void RenderBatchBuilder::Push(v2_t pos, const xpf::Color color) {
    v4_t c = color.get_vec4();
    m_vertices.push_back(pos.x);
    m_vertices.push_back(pos.y);
    m_vertices.push_back(c.r);
    m_vertices.push_back(c.g);
    m_vertices.push_back(c.b);
    m_vertices.push_back(c.a);
    m_vertices.push_back(0);
    m_vertices.push_back(0);
    m_vertex_count++;
}

void RenderBatchBuilder::Push3(v2_t p1, v2_t p2, v2_t p3, xpf::Color color) {
    Push(p1, color);
    Push(p2, color);
    Push(p3, color);
}

void RenderBatchBuilder::PushQuad(v2_t topLeft, v2_t topRight, v2_t bottomRight, v2_t bottomLeft, xpf::Color color) {
    Push3(topLeft, topRight, bottomRight, color);
    Push3(bottomRight, bottomLeft, topLeft, color);
}

void RenderBatchBuilder::Push(
    v2_t pos,
    v4_t color,
    v2_t textureCoords)
{
    m_vertices.push_back(pos.x);
    m_vertices.push_back(pos.y);
    m_vertices.push_back(color.x); // r
    m_vertices.push_back(color.y); // g
    m_vertices.push_back(color.z); // b
    m_vertices.push_back(color.w); // a
    m_vertices.push_back(textureCoords.x);
    m_vertices.push_back(textureCoords.y);

    m_vertex_count++;
}

void RenderBatchBuilder::Push3(
    VertexPositionColorTextureCoords&& p0,
    VertexPositionColorTextureCoords&& p1,
    VertexPositionColorTextureCoords&& p2)
{
    Push(p0.position, p0.color, p0.textureCoords);
    Push(p1.position, p1.color, p1.textureCoords);
    Push(p2.position, p2.color, p2.textureCoords);
}

void RenderBatchBuilder::PushQuad(
    VertexPositionColorTextureCoords&& topLeft,
    VertexPositionColorTextureCoords&& topRight,
    VertexPositionColorTextureCoords&& bottomRight,
    VertexPositionColorTextureCoords&& bottomLeft)
{
    Push3(std::move(topLeft), std::move(topRight), std::move(bottomRight));
    Push3(std::move(bottomRight), std::move(bottomLeft), std::move(topLeft));
}

void RenderBatchBuilder::PushQuad(const xpf::Quad& q, xpf::Color stroke)
{
    PushQuad(
        q.top_left,
        q.top_right,
        q.bottom_right,
        q.bottom_left,
        stroke);
}

void RenderBatchBuilder::DrawTriangle(v2_t p0, v2_t p1, v2_t p2, xpf::Color color)
{
    Push3(p0, p1, p2, color);
}

bool RenderBatchBuilder::DrawBezierQuadraticTriangle(v2_t p0, v2_t ctrl, v2_t p1, xpf::Color color)
{
    const bool ccw = xpf::math::SideOfLine(p0, ctrl, p1) < 0;
    if (!m_vertices.empty()) 
    {
        if (m_commandId != RenderCommandId::bezier_quadratic_triangle) {
            Flush();
        } else if (ccw && m_commandId != RenderCommandId::bezier_quadratic_triangle_ccw) {
            Flush();
        }
    }

    m_commandId = ccw ? RenderCommandId::bezier_quadratic_triangle_ccw : RenderCommandId::bezier_quadratic_triangle;
    m_spTexture = nullptr;

    v4_t c = color.get_vec4();
    // set UVs such that the curve becomes a simple y=x^2 in texture space.
    Push3(
        {p0, c, {0, 0}},
        {ctrl, c, {0.5, 0}},
        {p1, c, {1, 1}});

    return ccw;
}

void RenderBatchBuilder::DrawBezierCubicQuad(v2_t p0, v2_t ctrl0, v2_t ctrl1, v2_t p1, xpf::Color color)
{
    const bool ccw = !(xpf::math::isLeftWinding(p0, ctrl0, ctrl1) && xpf::math::isLeftWinding(ctrl0, ctrl1, p1));
    if (!m_vertices.empty()) 
    {
        if (ccw && m_commandId != RenderCommandId::bezier_cubic_quad_ccw) {
            Flush();
        } else if (m_commandId != RenderCommandId::bezier_cubic_quad) {
            Flush();
        }
    }

    m_commandId = ccw ? RenderCommandId::bezier_cubic_quad_ccw : RenderCommandId::bezier_cubic_quad;
    m_spTexture = nullptr;

    v4_t c = color.get_vec4();
    PushQuad(
        {p0, c, {0, 0}},
        {ctrl0, c, {0, 1}},
        {ctrl1, c, {1, 1}},
        {p1, c, {1, 0}});
}

} // xpf