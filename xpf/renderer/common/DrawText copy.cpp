#include "Common_Renderer.h"
#include "Font.h"
#include "Glyph.h"
#include "FormattedText.h"

namespace xpf {

void RenderBatchBuilder::DrawText(std::string_view text, float x, float y, std::string_view fontName, int16_t fontSize, xpf::Color color)
{
    m_commandId = RenderCommandId::text;

    float xorg = x;
    float yorg = y;

    if (fontSize < 0)
    {
        if (!(m_pRenderer->GetCapabilities() & RendererCapability::ShaderRenderedText))
            fontSize = -fontSize;
    }

    Typeface typeface;
    typeface.fontName = fontName;
    typeface.fontSize = fontSize;

    auto spFont = Font::GetFont(typeface);
    typeface = spFont->GetTypeface();
    const auto& metrics = spFont->GetMetrics();
    const float baseline = metrics.baseline;
    const float lineHeight = metrics.lineHeight;
    const float advanceNewLine = metrics.advanceNewlineY;
    const float scale = spFont->GetScale(fontSize);
    v4_t tint = color.get_vec4();

    if (typeface.fontSize <= 0)
    {
        Flush();

//        std::vector<RenderGlyphsCommand::glyph> glyphs;
//        glyphs.reserve(text.length());

        int32_t prevGlyphIndex = 0;
        const CodepointPage* ppage = nullptr;
        spFont->ForEachCodepoint(text, [&](char32_t ch, const CodepointPage& page, const Codepoint& cp)
        {
            float kern = spFont->GetKern(prevGlyphIndex, cp.glyphindex);
            prevGlyphIndex = cp.glyphindex;
            x += kern;

            // if (ppage != &page)
            // {
            //     if (ppage != nullptr && !glyphs.empty())
            //     {
            //         m_commands.push_back(
            //             std::make_shared<RenderGlyphsCommand>(
            //                 IBuffer::BufferLoader(reinterpret_cast<const byte_t*>(glyphs.data()), glyphs.size() * sizeof(glyphs[0])),
            //                 ppage->spBuffer));
            //         glyphs.clear();
            //     }
            //     ppage = &page;
            // }

            if (cp.width != 0)
            {
                float left = x + (cp.bearingX * scale);
                float top = y;
                float right = left + cp.width * scale;
                float bottom = y + lineHeight * scale;

                // glyphs.push_back({
                //     left, top, cp.width * scale, lineHeight * scale,
                //     cp.glyphStartOffset,
                //     cp.glyphPointCount,
                //     tint
                //     });

                PushQuad(
                    {{left,  top}, tint,    {left-x,top-y} },
                    {{right, top}, tint,    {right-x,top-y} },
                    {{right, bottom}, tint, {right-x,bottom-y} },
                    {{left,  bottom}, tint, {left-x,bottom-y} });

                m_commands.push_back(
                    std::make_shared<RenderGlyphCommand>(
                        std::move(m_vertices),
                        m_vertex_count,
                        page.spBuffer,
                        cp.glyphStartOffset,
                        v2_t(right - left, bottom - top),
                        scale));
                Flush();
            }

            x += cp.advance * scale;
        });

        // if (ppage != nullptr && !glyphs.empty())
        // {
        //     m_commands.push_back(
        //         std::make_shared<RenderGlyphsCommand>(
        //             IBuffer::BufferLoader(reinterpret_cast<const byte_t*>(glyphs.data()), glyphs.size() * sizeof(glyphs[0])),
        //             ppage->spBuffer));
        // }
    }
    else
    {
        if (m_commandId != RenderCommandId::text)
            Flush();

        spFont->ForEachCodepoint(text, [&](char32_t ch, const CodepointPage& page, const Codepoint& cp)
        {
            if (m_spTexture != page.spTexture)
            {
                Flush();
                m_commandId = RenderCommandId::text;
                m_spTexture = page.spTexture;
            }

            if (ch != '\n')
            {
                if (cp.width != 0)
                {
                    float left = x + (cp.bearingX * scale);
                    float top = round(y + baseline * scale + cp.yoffset * scale);
                    float right = left + cp.width * scale;
                    float bottom = top + cp.height * scale;
                    PushQuad(
                        {{left,  top}, tint, cp.texCoords.top_left() },
                        {{right, top}, tint, cp.texCoords.top_right() },
                        {{right, bottom}, tint, cp.texCoords.bottom_right() },
                        {{left,  bottom}, tint, cp.texCoords.bottom_left() });
                }
                
                x += (cp.advance) * scale;
            }
            else
            {
                x = xorg;
                y += advanceNewLine * scale;
            }
        });
    }
}

rectf_t RenderBatchBuilder::DrawText(std::string_view text, float x, float y, const TextDescription& description)
{
    if (text.starts_with("Text"))
        x +=0;

    if (m_commandId != RenderCommandId::text)
        Flush();

    m_commandId = RenderCommandId::text;

    auto spFont = Font::GetFont({description.fontName, description.fontSize, description.fontIndex});
    const auto& typeface = spFont->GetTypeface();
    const auto& metrics = spFont->GetMetrics();
    const float baseline = metrics.baseline;
    const float advanceNewLine = metrics.advanceNewlineY;
    const float scale = (typeface.fontSize == 0) ? 1.0 : description.fontSize / float(typeface.fontSize);
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

    float xorg = x;
    float yorg = y;
    v4_t tint = description.foreground.get_vec4();

    spFont->ForEachCodepoint(text, [&](char32_t ch, const CodepointPage& page, const Codepoint& cp)
    {
        if (m_spTexture != page.spTexture)
        {
            Flush();
            m_commandId = RenderCommandId::text;
            m_spTexture = page.spTexture;
        }

        if (ch != '\n')
        {
            if (cp.width != 0)
            {
                float left  = round(x + cp.bearingX * scale);
                float top   = round(y + baseline * scale + cp.yoffset * scale);
                float right = round(left + cp.width * scale);
                float bottom = round(top + cp.height * scale);

                PushQuad(
                    {{left,  top}, tint, cp.texCoords.top_left() },
                    {{right, top}, tint, cp.texCoords.top_right() },
                    {{right, bottom}, tint, cp.texCoords.bottom_right() },
                    {{left,  bottom}, tint, cp.texCoords.bottom_left() });
            }

            x += cp.advance * scale;
        }
        else
        {
            x = xorg;
            y += advanceNewLine * scale;
        }
    });

    return {xorg, yorg, bounds.width, bounds.height};
}

void RenderBatchBuilder::DrawText(float x, float y, FormattedText& ft, xpf::Color color)
{
    if (ft.GetText().starts_with("Text"))
        x +=0;

    if (m_commandId != RenderCommandId::text)
        Flush();

    m_commandId = RenderCommandId::text;
    float xorg = x;
    float baseline = ft.GetBaseline();
    v4_t tint = color.get_vec4();

    auto renderGlyph = [&](const Glyph& g, float x, float y)
    {
        if (g.width != 0)
        {
            if (m_spTexture != g.spTexture)
            {
                Flush();
                m_commandId = RenderCommandId::text;
                m_spTexture = g.spTexture;
            }

            float left  = x + g.bearing_x;
            float top   = y + baseline + g.yoffset;
            float right = left + g.width;
            float bottom = top + g.height;

            PushQuad(
                {{left,  top}, tint, g.texture_coordinates.top_left() },
                {{right, top}, tint, g.texture_coordinates.top_right() },
                {{right, bottom}, tint, g.texture_coordinates.bottom_right() },
                {{left,  bottom}, tint, g.texture_coordinates.bottom_left() });
        }

        return g.advance_to_right(x);
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