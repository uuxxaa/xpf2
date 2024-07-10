#include "RenderBatchBuilder.h"
#include <external/polyline2d/Polyline2D.h>

using namespace crushedpixel;

namespace xpf {

Polyline2D::JointStyle GetJointStyle(LineOptions lineOptions) {
    if (lineOptions & LineOptions::JointBevel) return Polyline2D::JointStyle::BEVEL;
    if (lineOptions & LineOptions::JointMiter) return Polyline2D::JointStyle::MITER;
    if (lineOptions & LineOptions::JointRound) return Polyline2D::JointStyle::ROUND;
    return Polyline2D::JointStyle::MITER;
}

Polyline2D::EndCapStyle GetCapStyle(LineOptions lineOptions) {
    if (lineOptions & LineOptions::CapButt) return Polyline2D::EndCapStyle::BUTT;
    if (lineOptions & LineOptions::CapSquare) return Polyline2D::EndCapStyle::SQUARE;
    if (lineOptions & LineOptions::CapRound) return Polyline2D::EndCapStyle::ROUND;
    if (lineOptions & LineOptions::Closed) return Polyline2D::EndCapStyle::JOINT;
    return Polyline2D::EndCapStyle::BUTT;
}

class PolylineBackInserter
{
private:
    RenderBatchBuilder& m_builder;

public:
    explicit PolylineBackInserter(RenderBatchBuilder& b) : m_builder(b) { }
    PolylineBackInserter& operator*() { return *this; }
    PolylineBackInserter& operator++() { return *this; }
    PolylineBackInserter& operator++(int) { return *this; }

    PolylineBackInserter& operator=(const PolyLineVertex& item)
    {
        m_builder.Push(item.position, item.color);
        return *this;
    }
};

void RenderBatchBuilder::DrawLine(float x0, float y0, float x1, float y1, float width, xpf::Color color, LineOptions lineOptions)
{
    std::vector<PolyLineVertex> points = {PolyLineVertex{v2_t(x0,y0), color, width}, PolyLineVertex{v2_t(x1,y1), color, width}};
    DrawLine(points, lineOptions);
}

void RenderBatchBuilder::DrawLine(v2_t p0, v2_t p1, float width, xpf::Color color, LineOptions lineOptions)
{
    std::vector<PolyLineVertex> points = {PolyLineVertex{p0, color, width}, PolyLineVertex{p1, color, width}};
    DrawLine(points, lineOptions);
}

void RenderBatchBuilder::DrawLine(const std::vector<PolyLineVertex>& points, LineOptions lineOptions)
{
    if (!m_vertices.empty() && m_commandId != RenderCommandId::position_color) {
        Flush();
    }

    m_commandId = RenderCommandId::position;

    Polyline2D::create<PolyLineVertex>(
        PolylineBackInserter(*this),
        points,
        GetJointStyle(lineOptions),
        GetCapStyle(lineOptions));
}

void RenderBatchBuilder::DrawBezierQuadratic(v2_t p1, v2_t p2, v2_t p3, float width, xpf::Color color, LineOptions lineOptions, uint32_t detail)
{
    DrawBezierQuadratic(
        {p1, color, width},
        {p2, color, width},
        {p3, color, width},
        lineOptions,
        detail);
}

void RenderBatchBuilder::DrawBezierQuadratic(const PolyLineVertex& v1, const PolyLineVertex& v2, const PolyLineVertex& v3, LineOptions lineOptions, uint32_t detail)
{
    const auto& tss = detail == 40 ? math::s_lerpPoints40 : math::calculate_lerp_points(detail);
    lineOptions &= LineOptions(0xF0F); // remove joint type - we don't want to add additional tris for line joints for bevel etc.

    uint32_t i = 0;
    m_polylineTempCache.resize(tss.size());
    for (const auto& ts : tss)
    {
        auto& entry = m_polylineTempCache[i++];
        entry.position = math::lerp_quadratic(v1.position, v2.position, v3.position, ts);
        entry.color = math::lerp_quadratic(v1.color, v2.color, v3.color, ts);
        entry.width = math::lerp_quadratic(v1.width, v2.width, v3.width, ts);
    }

    DrawLine(m_polylineTempCache, lineOptions);
}

void RenderBatchBuilder::DrawBezierCubic(v2_t p1, v2_t p2, v2_t p3, v2_t p4, float width, xpf::Color color, LineOptions lineOptions, uint32_t detail)
{
    DrawBezierCubic(
        {p1, color, width},
        {p2, color, width},
        {p3, color, width},
        {p4, color, width},
        lineOptions,
        detail);
}

void RenderBatchBuilder::DrawBezierCubic(const PolyLineVertex& v1, const PolyLineVertex& v2, const PolyLineVertex& v3, const PolyLineVertex& v4, LineOptions lineOptions, uint32_t detail)
{
    const auto& tss = detail == 40 ? math::s_lerpPoints40 : math::calculate_lerp_points(detail);
    lineOptions &= LineOptions(0xF0F); // remove joint type - we don't want to add additional tris for line joints for bevel etc.

    uint32_t i = 0;
    m_polylineTempCache.resize(tss.size());
    for (const auto& ts : tss)
    {
        auto& entry = m_polylineTempCache[i++];
        entry.position = math::lerp_cubic(v1.position, v2.position, v3.position, v4.position, ts);
        entry.color = math::lerp_cubic(v1.color, v2.color, v3.color, v4.color, ts);
        entry.width = math::lerp_cubic(v1.width, v2.width, v3.width, v4.width, ts);
    }

    DrawLine(m_polylineTempCache, lineOptions);
}

} // xpf