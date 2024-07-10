#include "Common_Renderer.h"
#include <core/Quad.h>

namespace xpf {

static float cap_length(float value)
{
    return xpf::math::clamp<float>(value, -10000000, 10000000);
}

void RenderBatchBuilder::DrawRectangle(rectf_t rect, xpf::Color color)
{
    DrawRectangle(rect.x, rect.y, rect.w, rect.h, color);
}

void RenderBatchBuilder::DrawRectangle(float x, float y, float w, float h, xpf::Color color)
{
    if (!m_vertices.empty() && m_commandId != RenderCommandId::position_color) {
        Flush();
    }

    m_commandId = RenderCommandId::position_color;
    m_spTexture = nullptr;

    v4_t tint = color.get_vec4();
    w = cap_length(w);
    h = cap_length(h);

    PushQuad(
        {{x,  y}, tint, {0,0} },
        {{x+w, y}, tint, {1,0} },
        {{x+w, y+h}, tint, {1,1} },
        {{x,  y+h}, tint, {0,1} });
}

void RenderBatchBuilder::DrawRectangle(float x, float y, const RectangleDescription& description)
{
    if (xpf::math::is_negligible_or_negative(description.width) ||
        xpf::math::is_negligible_or_negative(description.height))
        return;
    
    switch (description.horizontalOrientation)
    {
        case HorizontalOrientation::Left:
            break;
        case HorizontalOrientation::Center:
            x -= description.width * .5;
            break;
        case HorizontalOrientation::Right:
            x -= description.width;
            break;
    }

    switch (description.verticalOrientation)
    {
        case VerticalOrientation::Top:
            break;
        case VerticalOrientation::Center:
            y -= description.height * .5;
            break;
        case VerticalOrientation::Bottom:
            y -= description.height;
            break;
    }

    const bool hasBorder = !description.borderThickness.is_zero();
    const bool hasCornerRadius = !description.cornerRadius.is_zero();
    if (!hasBorder && !hasCornerRadius)
        return DrawRectangle(x, y, description.width, description.height, description.fillColor);

    Flush();

    const xpf::Color& fill = description.fillColor;
    const xpf::Color& borderColor = description.borderColor;
    xpf::CornerRadius radius = description.cornerRadius;
    const xpf::Thickness& borderThickness = description.borderThickness;
    const float width = cap_length(description.width);
    const float height = cap_length(description.height);

    if (radius.top_left < 0) {
        radius.top_left = width > height ? height * .5 : width * .5;
    }
    if (radius.top_right < 0) {
        radius.top_right = width > height ? height * .5 : width * .5;
    }
    if (radius.bottom_left < 0) {
        radius.bottom_left = width > height ? height * .5 : width * .5;
    }
    if (radius.bottom_right < 0) {
        radius.bottom_right = width > height ? height * .5 : width * .5;
    }

    v4_t tint = fill.get_vec4();
    rectf_t coords = {0,0,1,1};
    PushQuad(
        {{x,       y},        tint, coords.top_left() },
        {{x+width, y},        tint, coords.top_right() },
        {{x+width, y+height}, tint, coords.bottom_right() },
        {{x,       y+height}, tint, coords.bottom_left() });

    m_commands.push_back(std::make_shared<RenderRoundedRectangleCommand>(
        std::move(m_vertices), m_vertex_count,
        v2_t(width, height),
        radius,
        borderThickness,
        borderColor.get_vec4(),
        description.spTexture,
        description.borderType == RectangleDescription::Dot
    ));

    m_vertices.clear();
    m_vertex_count = 0;
}

#if 0
void RenderBatchBuilder::DrawRectangle(float x, float y, const RectangleDescription& description)
{
    m_commandId = RenderCommandId::position_color;
    m_spTexture = nullptr;

    const uint32_t detailX = 10, detailY = 10;
//    const Thickness& borderThickness = description.borderThickness;

//    if (!radius.is_zero_or_negative())
//    {
//        borderThickness = std::min(borderThickness.l, radius.top_left);
//        borderThickness = std::min(borderThickness.l, radius.top_right);
//        borderThickness = std::min(borderThickness.l, radius.bottom_left);
//        borderThickness = std::min(borderThickness.l, radius.bottom_right);
//        borderThickness = std::max(0.0f, borderThickness.l);
//    }

    if (!radius.is_zero_or_negative() || !borderThickness.is_zero_or_negative())
    {
        v2_t topLeftInner = {x + radius.top_left, y + radius.top_left};
        v2_t topRightInner = {x + width - radius.top_right, y + radius.top_right};
        v2_t bottomRightInner = {x + width - radius.bottom_right, y + height - radius.bottom_right};
        v2_t bottomLeftInner = {x + radius.bottom_left, y + height - radius.bottom_left};

        v2_t topLeftOutter = {x, y + radius.top_left};
        v2_t bottomLeftOutter = {x, y + height - radius.bottom_left};

        // left edge
        {
            xpf::Quad leftEdge(
                topLeftOutter,
                topLeftInner,
                bottomLeftInner,
                bottomLeftOutter);

            // left edge border
            if (!math::is_negligible(borderThickness.l)) {
                xpf::Quad leftBorder = leftEdge.SliceFromLeft(borderThickness.l);
                PushQuad(leftBorder, borderColor);
            }

            PushQuad(leftEdge, edgeColor);
        }

        // right edge
        {
            xpf::Quad rightEdge(
                topRightInner,
                {x + width, y + radius.top_right},
                {x + width, y + height - radius.bottom_right},
                bottomRightInner);

            // right edge border
            if (!math::is_negligible(borderThickness.r)) {
                xpf::Quad rightBorder = rightEdge.SliceFromRight(borderThickness.r);
                PushQuad(rightBorder, borderColor);
            }

            PushQuad(rightEdge, edgeColor);
        }

        // top edge
        {
            xpf::Quad topEdge(
                {x + radius.top_left, y},
                {x + width - radius.top_right, y},
                topRightInner,
                topLeftInner);

            // top edge border
            if (!math::is_negligible(borderThickness.t)) {
                xpf::Quad topBorder = topEdge.SliceFromTop(borderThickness.t);
                PushQuad(topBorder, borderColor);
            }

            PushQuad(topEdge, edgeColor);
        }

        // bottom edge
        {
            xpf::Quad bottomEdge(
                bottomLeftInner,
                bottomRightInner,
                {x + width - radius.bottom_right, y + height},
                {x + radius.bottom_left, y + height});

            // bottom edge border
            if (!math::is_negligible(borderThickness.b)) {
                xpf::Quad bottomBorder = bottomEdge.SliceFromBottom(borderThickness.b);
                PushQuad(bottomBorder, borderColor);
            }

            PushQuad(bottomEdge, edgeColor);
        }

        // middle
        PushQuad(topLeftInner, topRightInner, bottomRightInner, bottomLeftInner, fill);

        float startAngle = math::PI;
        float angleInc = math::PI / 12;

        // corner top left
        PushCorner(
            topLeftInner,
            startAngle, angleInc,
            radius.top_left, radius.top_left - borderThickness.l, borderColor, fill);

        // corner top right
        startAngle += math::PI_HALF;
        PushCorner(
            topRightInner,
            startAngle, angleInc,
            radius.top_right, radius.top_right - borderThickness.t, borderColor, fill);

        // corner bottom right
        startAngle += math::PI_HALF;
        PushCorner(
            bottomRightInner,
            startAngle, angleInc,
            radius.bottom_right, radius.bottom_right - borderThickness.r, borderColor, fill);

        // corner bottom left
        startAngle = math::PI_HALF;
        PushCorner(
            bottomLeftInner,
            startAngle, angleInc,
            radius.bottom_left, radius.bottom_left - borderThickness.b, borderColor, fill);
    }
    else if (detailX == 0 && detailY == 0)
    {
        PushQuad(x, y, width, height, fill);
    }
    else
    {
        const float xInc = width / detailX;
        const float yInc = height / detailY;

        float xpos = x;
        float ypos = y;
        for (uint32_t j = 0; j < detailY; j++) {
            for (uint32_t i = 0; i < detailX; i++) {
                PushQuad(xpos, ypos, xInc, yInc, fill);
                xpos += xInc;
            }
            ypos += yInc;
            xpos = x;
        }
    }
}

void RenderBatchBuilder::PushCorner(
    v2_t center,
    radians_t startAngle,
    radians_t angleInc,
    float outterRadius,
    float innerRadius,
    xpf::Color stroke,
    xpf::Color innerColor) {

    v2_t topLeft = center.add_polar(startAngle, outterRadius);
    v2_t bottomLeft = center.add_polar(startAngle, innerRadius);

    for (
        radians_t a = startAngle + angleInc;
        a <= startAngle + math::PI_HALF + math::Epsilon;
        a += angleInc) {

        v2_t topRight = center.add_polar(a, outterRadius);
        v2_t bottomRight = center.add_polar(a, innerRadius);
        PushQuad(topLeft, topRight, bottomRight, bottomLeft, stroke);

        if (!math::is_negligible(innerColor.a)) {
            Push3(bottomLeft, bottomRight, center, innerColor);
        }

        topLeft = topRight;
        bottomLeft = bottomRight;
    }
}
#endif

} // xpf