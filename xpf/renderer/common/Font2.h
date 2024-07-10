#pragma once
#include <stdint.h>
#include <string>
#include <functional>
#include <memory>

#include <unordered_map>
#include <unordered_set>
#include <core/Rectangle.h>
#include <core/Size.h>
#include <core/Macros.h>
#include <renderer/IBuffer.h>
#include <renderer/ITexture.h>

namespace xpf {

enum class FontRenderMode : uint8_t
{
    Texture = 0,
    LineRastered  = 1,
    TriRastered = 2,
};

struct FontSize
{
    uint16_t size = 10;
    bool inPixels = true;
};

struct FontMetrics2
{
    float ascent = 0; // aka baseline
    float descent = 0;
    float lineGap = 0;
    float fontHeight = 0;
    float lineHeight = 0;
    float advanceNewLineX = 0;
    float advanceNewLineY = 0;
    float scaledBy = 1.0;
};

struct Codepoint2
{
    int32_t glyphIndex = 0;
    int32_t xoffset = 0;
    int32_t yoffset = 0;
    int32_t bearingX = 0;
    int32_t width = 0;
    int32_t height = 0;
    int32_t advance = 0;
    rectf_t texCoords;
    uint32_t glyphStartOffset = 0; // start offset in number of 'short's for rendered glyphs
};

struct CodepointPage2
{
    uint32_t pageIndex = 0;
    std::unordered_map<uint16_t, std::shared_ptr<xpf::ITexture>> textures; // size -> texture
    std::shared_ptr<xpf::IBuffer> spLineRasterData;
    std::shared_ptr<xpf::IBuffer> spTriRasterData;
    std::unordered_map<char32_t, Codepoint2> codepoints;
};

class Font2
{
protected:
    struct FontInfo2;
    struct HideConstructor { };

    static constexpr uint32_t c_pageSize = 128;
    static inline std::unordered_map<std::string, std::shared_ptr<xpf::Font2>> s_installedFonts;
    std::shared_ptr<FontInfo2> m_spFontInfo;
    std::vector<byte_t> m_rawFontData;
    std::string m_name;
    bool m_isDefaultFont = false;
    bool m_supportsTextureRendering = true; // for now all fonts support this
    bool m_supportsRasterRendering = false;
    bool m_supportsTriangleRendering = false;
    char32_t m_defaultCharacter = '?';

public:
    explicit Font2(HideConstructor) {}
    static const std::shared_ptr<Font2>& GetFont(std::string_view fontName, uint16_t fontIndex = 0);
    static const std::shared_ptr<Font2>& GetDefaultFont();
    FontMetrics2 GetFontMetrics(FontSize fontSize) const;

    void ForEachCodepoint(
        std::string_view text,
        FontSize fontSize,
        FontRenderMode mode,
        std::function<void(
            char32_t,
            const std::shared_ptr<xpf::ITexture>&,
            const std::shared_ptr<xpf::IBuffer>&,
            const Codepoint2&,
            int32_t kern)>&& fn);

protected:
    Font2() = default;
    static std::shared_ptr<Font2> LoadDefaultFont();
    float GetScale(FontSize fontSize) const;
};

} // xpf