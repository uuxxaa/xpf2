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

enum class FontRenderOptions
{
    Texture = 0x0,
    Shaded  = 0x1,
    ShadedByTris = 0x2,
    SizeInPixels = 0x4,
};
ENUM_CLASS_FLAG_OPERATORS(FontRenderOptions);

struct Typeface
{
    std::string name;
    uint16_t size = 10;
    uint16_t index = 0;
    char32_t defaultCharacter = '?';
    bool fixedFont = false;
    bool defaultFont = false;
    FontRenderOptions renderOptions = FontRenderOptions::Texture;
};

inline bool operator==(const Typeface& t1,const Typeface& t2) {
    return
        t1.size == t2.size &&
        t1.index == t2.index &&
        t1.name == t2.name &&
        t1.fixedFont == t2.fixedFont &&
        t1.renderOptions == t2.renderOptions;
}

inline bool operator!=(const Typeface& t1,const Typeface& t2) {
    return !(t1 == t2);
}

struct Codepoint
{
    int32_t glyphindex = 0;
    float xoffset = 0;
    float yoffset = 0;
    float bearingX = 0;
    float width = 0;
    float height = 0;
    float advance = 0;
    rectf_t texCoords;
    uint32_t glyphStartOffset = 0; // start offset in number of 'short's
};

struct CodepointPage
{
    std::shared_ptr<xpf::ITexture> spTexture;
    std::shared_ptr<xpf::IBuffer> spBuffer;
    std::unordered_map<uint32_t, Codepoint> codepoints;
};

struct FontMetrics
{
    float charWidth = 0;
    float lineHeight = 0;
    float fontHeight = 0;
    float ascent = 0;
    float descent = 0;
    float baseline = 0;
    float advanceNewlineX = 0;
    float advanceNewlineY = 0;
};

struct FontInfo;

class Font
{
protected:
    struct FontData
    {
        std::vector<byte_t> data;
        bool supportsLineShading = true;
        bool supportsTriShading = true;
        bool supportsTextureShading = true;
    };

    struct HideConstructor { };
    Font() = default;

    Typeface m_typeface;
    FontMetrics m_metrics;
    uint32_t m_pageSize = 0;
    uint32_t m_codepointPadding = 0;
    bool m_isDefaultFont = false;
    bool m_supportsLineShading = true;
    bool m_supportsTriShading = true;
    bool m_supportsTextureShading = true;
    float m_scale = 1.0;

    mutable std::unordered_map<uint32_t, CodepointPage> m_pages;
    std::shared_ptr<FontInfo> m_spFontInfo;
    static constexpr uint32_t c_pageSize = 128;

    static inline std::function<std::vector<byte_t>(std::string_view)> s_fontLoader;
    static inline std::unordered_map<std::string, FontData> s_loadedTrueTypeFile;
    static inline std::unordered_map<std::string, std::shared_ptr<xpf::Font>> s_installedFonts;
    static inline std::unordered_set<std::string> s_failedToLoadFonts;

public:
    explicit Font(HideConstructor) {}

    static void SetFontLoader(std::function<std::vector<byte_t>(std::string_view)>&& fn) { s_fontLoader = std::move(fn); }

    static const std::shared_ptr<Font>& GetFont(const Typeface& typeface);
    static const std::shared_ptr<Font>& GetDefaultFont();

    const Typeface& GetTypeface() const { return m_typeface; }
    const FontMetrics& GetMetrics() const { return m_metrics; }
    float GetScale(int16_t fontSize) const;
    void ForEachCodepoint(std::string_view text, std::function<void(char32_t, const CodepointPage&, const Codepoint&, float kern)>&& fn);
    const CodepointPage& GetCodepointPage(uint32_t ch) const;
    const std::pair<CodepointPage&, const Codepoint&> GetCodepoint(char32_t ch) const;
    int32_t GetKern(char32_t ch1, char32_t ch2) const;
    SizeF Measure(std::string_view text) const;

protected:
    CodepointPage LoadPage(uint32_t pageIndex) const;
    CodepointPage LoadPageForShadedTris(uint32_t pageIndex) const;
    static std::shared_ptr<Font> LoadDefaultFont();
    static std::shared_ptr<Font> LoadFont(const Typeface& typeface);
};

} // xpf
