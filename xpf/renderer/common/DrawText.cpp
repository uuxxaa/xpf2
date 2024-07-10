#include "Common_Renderer.h"
#include "Font.h"
#include "Glyph.h"
#include "FormattedText.h"
#include <renderer/IBuffer.h>

namespace xpf {

static void DrawTextImpl(std::string_view text, const std::shared_ptr<Font>& spFont, uint16_t fontSize, float x, float y, const auto& fn)
{
    const float scale = spFont->GetScale(fontSize);
    const auto& metrics = spFont->GetMetrics();
    const float lineHeight = metrics.lineHeight;
    const float advanceNewLine = metrics.advanceNewlineY;

    float xorg = x;

    spFont->ForEachCodepoint(text, [&](char32_t ch, const CodepointPage& page, const Codepoint& cp, float kern)
    {
        x += kern * scale;

        if (ch == '\n')
        {
            x = xorg;
            y += advanceNewLine * scale;
        }
        else
        {
            if (cp.width != 0)
            {
                float left = x + (cp.bearingX * scale);
                float top = y;
                float right = x + (cp.bearingX + cp.width) * scale;
                float bottom = y + lineHeight * scale;

                fn(left, top, right, bottom, page, cp);
            }

            x += cp.advance * scale;
        }
    });
}

void RenderBatchBuilder::DrawText(std::string_view text, float x, float y, std::string_view fontName, uint16_t fontSize, xpf::Color color)
{
    TextDescription desc;
    desc.fontName = fontName;
    desc.fontSize = fontSize;
    desc.foreground = color;
    desc.renderOptions = FontRenderOptions::Texture;
    RenderBatchBuilder::DrawText(text, x, y, desc);
}

rectf_t RenderBatchBuilder::DrawText(std::string_view text, float x, float y, const TextDescription& description)
{
    Flush();

    m_commandId = RenderCommandId::text;
    float xorg = x;
    float yorg = y;

    Typeface typeface;
    typeface.name = description.fontName;
    typeface.size = description.fontSize;
    typeface.index = description.fontIndex;
    typeface.renderOptions = description.renderOptions;
    bool lineShaded = typeface.renderOptions & FontRenderOptions::Shaded;
    bool triShaded = typeface.renderOptions & FontRenderOptions::ShadedByTris;
    bool shaded = lineShaded || triShaded;
    v4_t tint = description.foreground.get_vec4();

    bool instanced = shaded && (m_pRenderer->GetCapabilities() & RendererCapability::ShaderInstancedRenderedText);
    bool rendered = shaded && (m_pRenderer->GetCapabilities() & RendererCapability::ShaderRenderedText);
    if (shaded)
    {
        if (!(instanced || rendered) || description.fontName.empty())
        {
            shaded = lineShaded = triShaded = false;
            instanced = rendered = false;
            typeface.renderOptions &= ~FontRenderOptions::Shaded;
        }
    }

    auto spFont = Font::GetFont(typeface);
    typeface = spFont->GetTypeface();
    const auto& metrics = spFont->GetMetrics();
    const float baseline = metrics.baseline;
    const float lineHeight = metrics.lineHeight;
    float scale = shaded ? 1.0 : description.fontSize / float(typeface.size);
    SizeF bounds = spFont->Measure(text) * scale;

    switch (description.horizontalOrientation)
    {
        case HorizontalOrientation::Left: break;
        case HorizontalOrientation::Center:
            x -= bounds.width * .5;
            break;
        case HorizontalOrientation::Right:
            x -= bounds.width;
            break;
    }

    switch (description.verticalOrientation)
    {
        case VerticalOrientation::Top: break;
        case VerticalOrientation::Center:
            y -= bounds.height * .5;
            break;
        case VerticalOrientation::Bottom:
            y -= bounds.height;
            break;
    }

    if (!description.background.is_transparent())
    {
        DrawRectangle(x, y, bounds.width, bounds.height, description.background);
    }

    if (instanced)
    {
        scale = spFont->GetScale(description.fontSize);
        Flush();
        std::unordered_map<const CodepointPage*, std::vector<RenderGlyphsCommand::GlyphsInstanceData>> data;
        DrawTextImpl(text, spFont, description.fontSize, x, y, [&](float left, float top, float right, float /*bottom*/, const CodepointPage& page, const Codepoint& cp)
        {
            data[&page].push_back({
                tint,
                left - 1,  top + (baseline + cp.yoffset) * scale - 1,
                right + 2, top + (baseline + cp.yoffset + cp.height) * scale + 2,
                (int32_t)(cp.bearingX - 1/scale), (int32_t)(cp.width + 3/scale), // extra one pixel on left and right of glyph
                (int32_t)(-1/scale),              (int32_t)(cp.height + 3/scale),
                cp.glyphStartOffset});
        });

        for (auto& entry : data)
        {
            Push({}, {}, {});
            Push({}, {}, {});
            Push({}, {}, {});
            Push({}, {}, {});

            uint32_t glyphCount = static_cast<uint32_t>(entry.second.size());
            m_commands.push_back(
                std::make_shared<RenderGlyphsCommand>(
                    std::move(m_vertices),
                    std::move(entry.second),
                    entry.first->spBuffer,
                    glyphCount,
                    scale));
            m_vertices.clear();
            m_vertex_count = 0;
        }
    }
    else if (rendered)
    {
        Flush();

        DrawTextImpl(text, spFont, description.fontSize, x, y, [&](float left, float top, float right, float bottom, const CodepointPage& page, const Codepoint& cp)
        {
            PushQuad(
                {{left,  top}, tint,    {cp.bearingX, 0} },
                {{right, top}, tint,    {cp.bearingX + cp.width, 0} },
                {{right, bottom}, tint, {cp.bearingX + cp.width, lineHeight} },
                {{left,  bottom}, tint, {cp.bearingX, lineHeight} });

            m_commands.push_back(
                std::make_shared<RenderGlyphCommand>(
                    std::move(m_vertices),
                    m_vertex_count,
                    page.spBuffer,
                    cp.glyphStartOffset,
                    scale));
            m_vertices.clear();
            m_vertex_count = 0;
        });
    }
    else
    {
        DrawTextImpl(text, spFont, description.fontSize, x, y, [&](float left, float top, float right, float bottom, const CodepointPage& page, const Codepoint& cp)
        {
            if (m_spTexture != page.spTexture || m_vertices.size() > 8000)
            {
                Flush();
                m_commandId = RenderCommandId::text;
                m_spTexture = page.spTexture;
            }

            top += (baseline + cp.yoffset) * scale;
            right = left + cp.width * scale;
            bottom = (top + cp.height * scale);

            PushQuad(
                {{left,  top}, tint, cp.texCoords.top_left() },
                {{right, top}, tint, cp.texCoords.top_right() },
                {{right, bottom}, tint, cp.texCoords.bottom_right() },
                {{left,  bottom}, tint, cp.texCoords.bottom_left() });
        });
    }

    return {xorg, yorg, bounds.width, bounds.height};
}

void RenderBatchBuilder::DrawText(float x, float y, FormattedText& ft, xpf::Color color)
{
    if (m_commandId != RenderCommandId::text)
        Flush();

    m_commandId = RenderCommandId::text;
    const float xorg = x;
    const float baseline = ft.GetBaseline();
    v4_t tint = color.get_vec4();

    auto renderGlyph = [this, baseline, tint](const Glyph& g, float xGlyph, float yGlyph)
    {
        if (g.width != 0)
        {
            if (m_spTexture != g.spTexture || m_vertices.size() > 8000)
            {
                Flush();
                m_spTexture = g.spTexture;
                m_commandId = RenderCommandId::text;
            }

            float left  = (xGlyph + g.bearing_x);
            float top   = yGlyph + baseline + g.yoffset;
            float right = left + g.width;
            float bottom = top + g.height;

            PushQuad(
                {{left,  top}, tint, g.texture_coordinates.top_left() },
                {{right, top}, tint, g.texture_coordinates.top_right() },
                {{right, bottom}, tint, g.texture_coordinates.bottom_right() },
                {{left,  bottom}, tint, g.texture_coordinates.bottom_left() });
        }

        return g.advance_to_right(xGlyph);
    };

    for (const auto& line : ft.GetLines())
    {
        if (line.isOverBudgetY)
            break;

        x = xorg;
        for (const auto& g : line.glyphs)
        {
            if (g.is_newline())
                break;

            x = renderGlyph(g, x, y);
        }

        y = ft.AdvanceToNewline(y);
    }
}

} // xpf