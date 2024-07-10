#include "FormattedText.h"
#include "RenderBatchBuilder.h"

namespace xpf {

// static bool is_whitespace(char32_t ch) {
//     return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
// }

static bool is_horizontal_whitespace(char32_t ch) {
    return ch == ' ' || ch == '\t';
}

static bool is_newline(char32_t ch) {
    return ch == '\n';
}

FormattedText::FormattedText(
    std::string_view text,
    const Typeface& typeface)
    : m_text(text)
    , m_typeface(typeface)
{
}

const Glyph& FormattedText::GetEllipsisGlyph()
{
    if (m_isGeometryBuilt)
        BuildGeometry();

    return m_ellipsisGlyph;
}

const std::vector<FormattedLine>& FormattedText::GetLines()
{
    if (m_isGeometryBuilt)
        BuildGeometry();

    return m_lines;
}

void FormattedLine::AddGlyph(Glyph&& gIn, float kern)
{
    if (glyphs.empty())
    {
        leftBearingX = gIn.bearing_x;
    }
    else if (kern > xpf::math::Epsilon)
    {
        lineWidth += kern;
        glyphs.back().advance_x += kern;
    }

    Glyph& g = glyphs.emplace_back(std::move(gIn));
    if (g.is_newline())
        return;

    lineWidth = g.advance_to_right(lineWidth);

    if (!is_horizontal_whitespace(g.ch))
        lineWidthIncludingTrailingspaces = lineWidth;
}

void FormattedText::FinalizeLineCalculations(FormattedLine& line)
{
    float x = 0;
    float bounds_x = line.lineWidth;
    float leftBearing = 0;
    if (!line.glyphs.empty())
        leftBearing = line.glyphs.front().bearing_x;
    
    if (line.glyphs.size() >= 1)
    {
        line.lineWidth -= line.glyphs.front().bearing_x;
        line.lineWidth -= line.glyphs.back().advance_x;
        line.lineWidth += line.glyphs.back().width;
    }

    if (line.lineWidth > m_maxWidth)
    {
        for (size_t i = 0; i < line.glyphs.size(); i++)
        {
            float newx = line.glyphs[i].advance_to_right(x);
            if (newx > m_maxWidth - m_ellipsisWidth)
            {
                line.glyphs[i].is_over_budget = true;
                line.glyphs.erase(line.glyphs.begin() + i, line.glyphs.cend());
                const Glyph ellipsis_glyph = GetEllipsisGlyph();
                line.glyphs.push_back(ellipsis_glyph);
                if (ellipsis_glyph.ch == '.') {
                    line.glyphs.push_back(ellipsis_glyph);
                    line.glyphs.push_back(ellipsis_glyph);
                }
                break;
            }
            else
            {
                bounds_x = newx + m_ellipsisWidth;
            }
            x = newx;
        }
    }

    m_bounds.x = std::max(m_bounds.x, bounds_x);

    m_width = std::max(line.lineWidth, m_width);
    m_widthIncludingTrailingspaces = std::max(line.lineWidthIncludingTrailingspaces, m_widthIncludingTrailingspaces);

    if (m_lineHeight + m_height <= m_maxHeight + math::Epsilon  && m_lines.size() < m_maxLineCount)
    {
        m_height += m_lineHeight + m_advanceNewlineY;
        m_bounds.y = m_height;
    }
    else
    {
        line.isOverBudgetY = true;
    }

    m_boundsNoBearings.x = std::max(m_boundsNoBearings.x, bounds_x + leftBearing);
    m_boundsNoBearings.y = m_bounds.y;

    m_minHeight += m_lineHeight;
}

void FormattedText::BuildGeometry()
{
    if (m_isGeometryBuilt)
        return;

    m_isGeometryBuilt = true;

    m_lines.clear();

    m_spFont = Font::GetFont(m_typeface);
    const float scale = m_spFont->GetScale(m_typeface.size);
    const auto& metrics = m_spFont->GetMetrics();
    m_lineHeight = metrics.lineHeight * scale;
    m_baseline = metrics.baseline * scale;
    m_advanceNewlineX = metrics.advanceNewlineX * scale;
    m_advanceNewlineY = metrics.advanceNewlineY * scale;
    m_bounds = v2_t(0);

    if (m_textTrimming == TextTrimming::None)
    {
        m_ellipsisWidth = 0;
    }
    else
    {
        auto ci = m_spFont->GetCodepoint(0x2026);
        if (ci.first.spTexture == nullptr)
        {
            auto ci2 = m_spFont->GetCodepoint('.');
            m_ellipsisGlyph = Glyph('.', ci2.first.spTexture, ci2.second, scale);
            m_ellipsisWidth = m_ellipsisGlyph.advance_x * 3;
        }
        else
        {
            m_ellipsisGlyph = Glyph(0x2026, ci.first.spTexture, ci.second, scale);
            m_ellipsisWidth = m_ellipsisGlyph.advance_x;
        }
    }

    m_width = 0;                        // computed
    m_widthIncludingTrailingspaces = 0; // computed
    m_height = 0;                       // computed
    m_minHeight = 0;                    // computed

    FormattedLine* pline = &m_lines.emplace_back();

    m_spFont->ForEachCodepoint(m_text, [&](char32_t ch, const CodepointPage& page, const Codepoint& codepoint, float kern)
    {
        pline->AddGlyph(Glyph(ch, page.spTexture, codepoint, scale), kern * scale);

        if (is_newline(ch))
        {
            FinalizeLineCalculations(*pline);
            pline = &m_lines.emplace_back();
        }
    });

    FinalizeLineCalculations(*pline);
}

float FormattedText::AdvanceToNewline(float y) const
{
    return y + m_lineHeight + m_advanceNewlineY;
}

} // xpf
