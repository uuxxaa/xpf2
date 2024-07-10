#pragma once
#include <vector>
#include <renderer/common/Glyph.h>
#include <renderer/common/Font.h>

namespace xpf {

class Font;
enum class TextTrimming;

enum class TextAlignment
{
    Center,
    Justify,
    Left,
    Right,
};

static inline std::string_view to_string(TextAlignment t)
{
    switch(t) {
        case TextAlignment::Center: return "Center";
        case TextAlignment::Justify: return "Justify";
        case TextAlignment::Left: return "Left";
        case TextAlignment::Right: return "Right";
    }
}

class FormattedText;
struct Glyph;

struct FormattedLine
{
    float lineWidth = 0;
    float lineWidthIncludingTrailingspaces = 0;
    float leftBearingX = 0;
    bool isOverBudgetX = false;
    bool isOverBudgetY = false;
    std::vector<Glyph> glyphs;

    FormattedLine() = default;
    FormattedLine(FormattedLine&&) = default;
    FormattedLine& operator=(FormattedLine&&) = default;
    void AddGlyph(Glyph&& g, float kern);
};

class FormattedText
{
protected:
    std::string m_text;
    Typeface m_typeface;
    std::shared_ptr<xpf::Font> m_spFont;
    TextTrimming m_textTrimming = TextTrimming(0);
    TextAlignment m_textAlignment = TextAlignment::Left;
    std::vector<FormattedLine> m_lines;
    Glyph m_ellipsisGlyph;

    float m_lineHeight = 0;                   // acquired from font
    float m_baseline = 0;                     // acquired from font
    float m_advanceNewlineX = 0;
    float m_advanceNewlineY = 0;
    float m_ellipsisWidth = 0;
    float m_width = 0;                        // computed
    float m_widthIncludingTrailingspaces = 0; // computed
    float m_height = 0;                       // computed
    float m_minHeight = 0;                    // computed
    float m_tabWidth = 0;
    v2_t m_bounds;
    v2_t m_boundsNoBearings;

    float m_maxWidth = std::numeric_limits<float>::max();  // provided by user
    float m_maxHeight = std::numeric_limits<float>::max(); // provided by user
    uint32_t m_maxLineCount = std::numeric_limits<uint32_t>::max(); // provided by user

    bool m_isGeometryBuilt = false;

public:
    FormattedText() = default;
    FormattedText(FormattedText&&) = default;
    FormattedText(const FormattedText&) = delete;
    FormattedText(std::string_view text, const Typeface& typeface);

    FormattedText& operator=(FormattedText&&) = default;
    FormattedText& operator=(const FormattedText&) = delete;

    bool IsEmpty() const { return m_spFont == nullptr && m_text.empty(); }

    void SetText(std::string_view text) { if (m_text != text) { m_isGeometryBuilt = false; m_text = text; } }
    void SetFont(const Typeface& typeface) { if (typeface != m_typeface) { m_isGeometryBuilt = false; m_typeface = typeface; } }
    void SetTextTrimming(TextTrimming value) { if (m_textTrimming != value) { m_isGeometryBuilt = false; m_textTrimming = value; } }
    void SetMaxWidth(float value) { if (m_maxWidth != value) { m_isGeometryBuilt = false; m_maxWidth = value; } }
    void SetMaxHeight(float value) { if (m_minHeight != value) { m_isGeometryBuilt = false; m_minHeight = value; } }
    void SetMaxLineCount(uint32_t count) { if (m_maxLineCount != count) { m_isGeometryBuilt = false; m_maxLineCount = count; } }
    void SetTabWidth(float tab_width) { if (m_tabWidth != tab_width) { m_isGeometryBuilt = false; m_tabWidth = tab_width; } }

    const std::string& GetText() const { return m_text; }
    const Typeface& GetTypeface() const { return m_typeface; }

    float GetWidth() const { return m_width; }
    float GetHeight() const { return m_height; }
    v2_t GetSize() const { return {m_width, m_height}; }
    float GetMaxWidth() const { return m_maxWidth; }
    float GetWidthIncludingTrailingspaces() const { return m_widthIncludingTrailingspaces; }
    float GetMaxHeight() const { return m_maxHeight; }

    float GetAdvanceNewlineX() const { return m_advanceNewlineX; }
    float GetAdvanceNewlineY() const { return m_advanceNewlineY; }

    uint32_t GetLineCount() const { return uint32_t(m_lines.size()); }
    float GetLineHeight() const { return m_lineHeight; }
    float GetBaseline() const { return m_baseline; }
    float getEllipsisWidth() const { return m_ellipsisWidth; }

    void BuildGeometry();
    const std::vector<FormattedLine>& GetLines();
    const Glyph& GetEllipsisGlyph();
    float AdvanceToNewline(float y) const;
    v2_t GetBounds() const { return m_bounds; }
    v2_t GetBoundsNoBearings() const { return m_boundsNoBearings; }
    v2_t GetIdealBounds() const { return { GetWidth(), m_lines.size() * GetLineHeight() }; }

protected:
    void FinalizeLineCalculations(FormattedLine& line);
};

} // xpf