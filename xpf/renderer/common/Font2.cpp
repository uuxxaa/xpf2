#include "Font2.h"
#include <renderer/IBuffer.h>
#include <renderer/ITexture.h>
#include <core/Image.h>
#include <core/Log.h>
#include <core/FileSystem.h>
#include <core/stringex.h>

#define STB_TRUETYPE_IMPLEMENTATION
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#include <stb/stb_truetype.h>
#pragma clang diagnostic pop
#include <earcut.hpp>

struct Point16
{
    int16_t x = 0, y = 0;
    Point16 operator-(Point16 o) const { return Point16{(int16_t)(x - o.x), (int16_t)(y - o.y)}; }
    static int16_t cross(Point16 a, Point16 b) { return (a.x * b.y - a.y * b.x); }
    // which side of the a-c line lies the point b
    static int16_t SideOfLine(Point16 a, Point16 b, Point16 c)
    {
        return xpf::math::sign<int16_t>((c.x - a.x) * (-b.y + a.y) + (c.y - a.y) * (b.x - a.x));
    }
};

static void earcut(
    std::vector<std::vector<Point16>>& polygon,
    int16_t ascent16,
    size_t countOfContoursIndex,
    std::vector<int16_t>& tris)
{
    auto pushPoint = [&tris, &polygon, ascent16](size_t i)
    {
        for (const auto& part : polygon)
        {
            if (i < part.size())
            {
                const auto& v = part[i];
                tris.push_back(v.x);
                tris.push_back(ascent16 - v.y);
                return;
            }

            i -= part.size();
        }

        throw std::exception(); // invalid earcut result;
    };

    std::vector<size_t> indices = mapbox::earcut<size_t>(polygon);
    tris.push_back(0); // type = 0 filler triangles
    tris.push_back(indices.size()); // count of points
    for(size_t j = 0; j < indices.size(); j+=3)
    {
        pushPoint(indices[j + 0]);
        pushPoint(indices[j + 1]);
        pushPoint(indices[j + 2]);
    }

    tris[countOfContoursIndex]++;
}

namespace xpf {

struct Font2::FontInfo2
{
    stbtt_fontinfo info;
    std::unordered_map<uint32_t, CodepointPage2> pages;
    uint32_t pageSize = 128;
    int16_t descent = 0;
    int16_t ascent = 0; // aka baseline
    int16_t fontHeight = 0;
    int16_t lineHeight = 0;
    int16_t advanceNewLineX = 0;
    int16_t advanceNewLineY = 0;
    int16_t lineGap = 0;
    bool defaultFont = false;

    FontInfo2() = default;

    bool InitializeFontInfo(std::string_view name, uint16_t fontIndex, const byte_t* pdata)
    {
        if (!stbtt_InitFont(
            &info,
            pdata,
            stbtt_GetFontOffsetForIndex(pdata, fontIndex)))
        {
            Log::error(
                "Font: " + name +
                " did not have font at index " + std::to_string(fontIndex) +
                " trying at index 0.");

            if (!stbtt_InitFont(
                &info,
                pdata,
                stbtt_GetFontOffsetForIndex(pdata, 0)))
            {

                Log::error("Font: " + name + " failed to initialize.");
                return false;
            }
        }

        int32_t vascent, vdescent, vlineGap;
        stbtt_GetFontVMetrics(&info, &vascent, &vdescent, &vlineGap);

        ascent = vascent;
        descent = -vdescent;
        fontHeight = vascent - vdescent;
        lineHeight = fontHeight + vlineGap;
        lineGap = vlineGap;
        return true;
    }

    int32_t GetKern(uint32_t g1, uint32_t g2) const
    {
        return stbtt__GetGlyphKernInfoAdvance(&info, g1, g2);
    }

    auto GetPageIter(uint32_t pageIndex)
    {
        auto pageIter = pages.find(pageIndex);
        if (pageIter == pages.cend()) [[likely]]
        {
            CodepointPage2 page;
            page.pageIndex = pageIndex;
            pages[pageIndex] = std::move(page);
            pageIter = pages.find(pageIndex);
        }

        return pageIter;
    }

    const Codepoint2* GetCodepoint(auto pageIter, char32_t ch, char32_t errorChar) const
    {
        uint32_t codepointIndex = ch % c_pageSize;
        const CodepointPage& page = pageIter->second;
        auto codepointIter = page.codepoints.find(codepointIndex);
        if (codepointIter != page.codepoints.cend()) [[likely]]
            return codepointIter->second;

        if (errorChar == 0)
            return nullptr;

        return GetCodepoint(GetPageIter(mode, errorChar/c_pageSize), errorChar, 0);
    }

    const std::shared_ptr<xpf::ITexture>& EnsureFontTexture(CodepointPage2& page, uint16_t fontSize)
    {
        if (defaultFont) return page.textures[0];

        auto iter = page.textures.find(fontSize);
        if (iter != page.textures.cend())
            return iter->second;

        uint32_t pageStart = page.pageIndex * pageSize;
        uint32_t pageEnd = pageStart + pageSize;
        int32_t atlasWidth = 0, atlasHeight = 0;
        const float scale = float(fontSize) / fontHeight;

        const stbtt_fontinfo* pfont_info = &info;
        float atlasWidthScale = 0;
        float atlasHeightScale = 0;

        for (char32_t ch32 = pageStart; ch32 < pageEnd; ch32++)
        {
            int32_t ix0 = 0, iy0 = 0, ix1 = 0, iy1 = 0;
            stbtt_GetCodepointBitmapBox(pfont_info, int32_t(ch32), scale, scale, &ix0, &iy0, &ix1, &iy1);
            atlasWidth += (ix1 -ix0);
            int32_t height = (iy1 -iy0);
            if (height > atlasHeight)
                atlasHeight = height;
        }

        atlasWidthScale = 1.0f / float(atlasWidth);
        atlasHeightScale = 1.0f / float(atlasHeight);
        std::vector<byte_t> atlas = std::vector<byte_t>(atlasWidth * atlasHeight * sizeof(uint8_t));

        uint32_t atlasX = 0;
        for (uint32_t ch32 = pageStart; ch32 < pageEnd; ch32++)
        {
            const int32_t glyphIndex = stbtt_FindGlyphIndex(pfont_info, int32_t(ch32));

            int32_t advanceX = 0;
            int32_t bearingX = 0;
            stbtt_GetGlyphHMetrics(pfont_info, glyphIndex, &advanceX, &bearingX);

            int32_t width = 0, height = 0, xoffset = 0, yoffset = 0;
            byte_t* pageData = stbtt_GetGlyphBitmap(pfont_info, scale, scale, glyphIndex, &width, &height, &xoffset, &yoffset);
            byte_t* pdata = pageData;

            uint8_t* pImageData = (uint8_t*)atlas.data();
            for (int32_t h = 0; h < height; h++) {
                for (int32_t p = 0; p < width; p++) {
                    uint8_t val = *(pdata++);
                    pImageData[atlasX + p + h * atlasWidth] = val;
                }
            }

            stbtt_FreeBitmap(pageData, /*userdata:*/ nullptr);

            Codepoint2& codepoint = page.codepoints[ch32 - pageStart];
            codepoint.glyphIndex = glyphIndex;
            codepoint.xoffset = xoffset;
            codepoint.yoffset = yoffset;
            codepoint.width = width;
            codepoint.height = height;
            codepoint.bearingX = bearingX;
            codepoint.advance = advanceX;
            rectf_t rect;
            rect.x = atlasX;
            rect.y = 0;
            rect.w = width;
            rect.h = height;
            codepoint.texCoords = rect.multiply(atlasWidthScale, atlasHeightScale);

            atlasX += width;
        }

        return page.textures[fontSize] = ITexture::TextureLoader(
            std::move(atlas),
            atlasWidth, atlasHeight,
            PixelFormat::GrayScale,
            /*mipMapCount*/ 1);
    }

    const std::shared_ptr<xpf::IBuffer>& EnsureFontLineRasterData(CodepointPage2& page)
    {
        // return page.lineRasterData;
        return nullptr;
    }

    const std::shared_ptr<xpf::IBuffer>& EnsureFontTriRasterData(CodepointPage2& page)
    {
        if (page.spTriRasterData != nullptr)
            return page.spTriRasterData;

        uint32_t pageStart = page.pageIndex * pageSize;
        uint32_t pageEnd = pageStart + pageSize;

        std::vector<int16_t> tris;
        tris.push_back(1); // triangle rastering

        for (uint32_t ch32 = pageStart; ch32 < pageEnd; ch32++)
        {
            const int32_t glyphIndex = stbtt_FindGlyphIndex(&info, int32_t(ch32));
            int32_t advanceX = 0;
            int32_t bearingX = 0;
            stbtt_GetGlyphHMetrics(&info, glyphIndex, &advanceX, &bearingX);

            int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
            stbtt_GetGlyphBitmapBoxSubpixel(&info, glyphIndex, 1.0, 1.0, 0.0, 0.0, &x0,&y0,&x1,&y1);
            int32_t width = x1 - x0;
            int32_t height = y1 - y0;
            int32_t xoffset = x0;
            int32_t yoffset = y0;

            stbtt_vertex* pvertices = nullptr;
            int32_t num_vertices = stbtt_GetGlyphShape(&info, glyphIndex, &pvertices);

            Codepoint2& codepoint = page.codepoints[ch32 - pageStart];
            codepoint.glyphIndex = glyphIndex;
            codepoint.xoffset = xoffset;
            codepoint.yoffset = yoffset;
            codepoint.width = width;
            codepoint.height = height;
            codepoint.bearingX = bearingX;
            codepoint.advance = advanceX;
            codepoint.glyphStartOffset = static_cast<uint32_t>(tris.size());

            size_t countOfContoursIndex = tris.size();
            tris.push_back(0); // countOfContoursIndex;

            int16_t x = 0, y = 0;
            std::vector<std::vector<Point16>> polygon;
            std::vector<Point16>& contour = polygon.emplace_back();
            std::vector<Point16> beziers;
            std::vector<Point16> beziers_ccw;

            for (int32_t vindex = 0; vindex < num_vertices; vindex++)
            {
                const stbtt_vertex& vrtx = pvertices[vindex];
                if (vrtx.type == STBTT_vmove)
                {
                    if (contour.size() != 0)
                    {
                        if (contour.size() < 3)
                            throw std::exception(); // invalid contour

                        earcut(polygon, ascent, countOfContoursIndex, tris);
                        contour.clear();
                    }

                    x = vrtx.x; y = vrtx.y;
                }
                else if (vrtx.type == STBTT_vline)
                {
                    contour.push_back(Point16{x, y});
                }
                else if (vrtx.type == STBTT_vcurve)
                {
                    Point16 a{x, (int16_t)(y)};
                    Point16 b{vrtx.cx, (int16_t)(vrtx.cy)};
                    Point16 c{vrtx.x, (int16_t)(vrtx.y)};
                    contour.push_back(a);

                    // which side of the a-c line lies the point b
                    bool ccw = Point16::SideOfLine(a, b, c) < 0;
                    if (ccw)
                    {
                        contour.push_back(Point16{vrtx.cx, vrtx.cy});
                        beziers_ccw.push_back(a);
                        beziers_ccw.push_back(b);
                        beziers_ccw.push_back(c);
                    }
                    else
                    {
                        beziers.push_back(a);
                        beziers.push_back(b);
                        beziers.push_back(c);
                    }
                }
                else if (vrtx.type == STBTT_vcubic)
                {
                    throw std::exception(); // NYI
                }

                x = vrtx.x; y = vrtx.y;
            }

            if (!contour.empty())
            {
                earcut(polygon, ascent, countOfContoursIndex, tris);
            }

            if (!beziers.empty())
            {
                tris.push_back(-1);
                tris.push_back(beziers.size());
                for (const auto& pt : beziers)
                {
                    tris.push_back(pt.x);
                    tris.push_back((int16_t)(ascent - pt.y));
                }

                tris[countOfContoursIndex]++;
            }

            if (!beziers_ccw.empty())
            {
                tris.push_back(1);
                tris.push_back(beziers_ccw.size());
                for (const auto& pt : beziers_ccw)
                {
                    tris.push_back(pt.x);
                    tris.push_back((int16_t)(ascent - pt.y));
                }

                tris[countOfContoursIndex]++;
            }

            STBTT_free(pvertices, info.userdata);
        }

        page.spTriRasterData = IBuffer::BufferLoader(
            reinterpret_cast<const byte_t*>(tris.data()),
            tris.size() * sizeof(tris[0]));

        return page.spTriRasterData;
    }
};

#pragma region default font
/*static*/ std::shared_ptr<Font2> Font2::LoadDefaultFont()
{
    #define BIT_CHECK(a,b) ((a) & (1u << (b)))

    constexpr uint32_t c_textureWidth = 128;
    constexpr float c_textureWidthScale = 1.0f / c_textureWidth;

    constexpr uint32_t c_defaultFontData[512] = {
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00200020, 0x0001b000, 0x00000000, 0x00000000, 0x8ef92520, 0x00020a00, 0x7dbe8000, 0x1f7df45f,
        0x4a2bf2a0, 0x0852091e, 0x41224000, 0x10041450, 0x2e292020, 0x08220812, 0x41222000, 0x10041450, 0x10f92020, 0x3efa084c, 0x7d22103c, 0x107df7de,
        0xe8a12020, 0x08220832, 0x05220800, 0x10450410, 0xa4a3f000, 0x08520832, 0x05220400, 0x10450410, 0xe2f92020, 0x0002085e, 0x7d3e0281, 0x107df41f,
        0x00200000, 0x8001b000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc0000fbe, 0xfbf7e00f, 0x5fbf7e7d, 0x0050bee8, 0x440808a2, 0x0a142fe8, 0x50810285, 0x0050a048,
        0x49e428a2, 0x0a142828, 0x40810284, 0x0048a048, 0x10020fbe, 0x09f7ebaf, 0xd89f3e84, 0x0047a04f, 0x09e48822, 0x0a142aa1, 0x50810284, 0x0048a048,
        0x04082822, 0x0a142fa0, 0x50810285, 0x0050a248, 0x00008fbe, 0xfbf42021, 0x5f817e7d, 0x07d09ce8, 0x00008000, 0x00000fe0, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x000c0180,
        0xdfbf4282, 0x0bfbf7ef, 0x42850505, 0x004804bf, 0x50a142c6, 0x08401428, 0x42852505, 0x00a808a0, 0x50a146aa, 0x08401428, 0x42852505, 0x00081090,
        0x5fa14a92, 0x0843f7e8, 0x7e792505, 0x00082088, 0x40a15282, 0x08420128, 0x40852489, 0x00084084, 0x40a16282, 0x0842022a, 0x40852451, 0x00088082,
        0xc0bf4282, 0xf843f42f, 0x7e85fc21, 0x3e0900bf, 0x00000000, 0x00000004, 0x00000000, 0x000c0180, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x04000402, 0x41482000, 0x00000000, 0x00000800,
        0x04000404, 0x4100203c, 0x00000000, 0x00000800, 0xf7df7df0, 0x514bef85, 0xbefbefbe, 0x04513bef, 0x14414500, 0x494a2885, 0xa28a28aa, 0x04510820,
        0xf44145f0, 0x474a289d, 0xa28a28aa, 0x04510be0, 0x14414510, 0x494a2884, 0xa28a28aa, 0x02910a00, 0xf7df7df0, 0xd14a2f85, 0xbefbe8aa, 0x011f7be0,
        0x00000000, 0x00400804, 0x20080000, 0x00000000, 0x00000000, 0x00600f84, 0x20080000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0xac000000, 0x00000f01, 0x00000000, 0x00000000, 0x24000000, 0x00000f01, 0x00000000, 0x06000000, 0x24000000, 0x00000f01, 0x00000000, 0x09108000,
        0x24fa28a2, 0x00000f01, 0x00000000, 0x013e0000, 0x2242252a, 0x00000f52, 0x00000000, 0x038a8000, 0x2422222a, 0x00000f29, 0x00000000, 0x010a8000,
        0x2412252a, 0x00000f01, 0x00000000, 0x010a8000, 0x24fbe8be, 0x00000f01, 0x00000000, 0x0ebe8000, 0xac020000, 0x00000f01, 0x00000000, 0x00048000,
        0x0003e000, 0x00000f00, 0x00000000, 0x00008000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000038, 0x8443b80e, 0x00203a03,
        0x02bea080, 0xf0000020, 0xc452208a, 0x04202b02, 0xf8029122, 0x07f0003b, 0xe44b388e, 0x02203a02, 0x081e8a1c, 0x0411e92a, 0xf4420be0, 0x01248202,
        0xe8140414, 0x05d104ba, 0xe7c3b880, 0x00893a0a, 0x283c0e1c, 0x04500902, 0xc4400080, 0x00448002, 0xe8208422, 0x04500002, 0x80400000, 0x05200002,
        0x083e8e00, 0x04100002, 0x804003e0, 0x07000042, 0xf8008400, 0x07f00003, 0x80400000, 0x04000022, 0x00000000, 0x00000000, 0x80400000, 0x04000002,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00800702, 0x1848a0c2, 0x84010000, 0x02920921, 0x01042642, 0x00005121, 0x42023f7f, 0x00291002,
        0xefc01422, 0x7efdfbf7, 0xefdfa109, 0x03bbbbf7, 0x28440f12, 0x42850a14, 0x20408109, 0x01111010, 0x28440408, 0x42850a14, 0x2040817f, 0x01111010,
        0xefc78204, 0x7efdfbf7, 0xe7cf8109, 0x011111f3, 0x2850a932, 0x42850a14, 0x2040a109, 0x01111010, 0x2850b840, 0x42850a14, 0xefdfbf79, 0x03bbbbf7,
        0x001fa020, 0x00000000, 0x00001000, 0x00000000, 0x00002070, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x08022800, 0x00012283, 0x02430802, 0x01010001, 0x8404147c, 0x20000144, 0x80048404, 0x00823f08, 0xdfbf4284, 0x7e03f7ef, 0x142850a1, 0x0000210a,
        0x50a14684, 0x528a1428, 0x142850a1, 0x03efa17a, 0x50a14a9e, 0x52521428, 0x142850a1, 0x02081f4a, 0x50a15284, 0x4a221428, 0xf42850a1, 0x03efa14b,
        0x50a16284, 0x4a521428, 0x042850a1, 0x0228a17a, 0xdfbf427c, 0x7e8bf7ef, 0xf7efdfbf, 0x03efbd0b, 0x00000000, 0x04000000, 0x00000000, 0x00000008,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00200508, 0x00840400, 0x11458122, 0x00014210,
        0x00514294, 0x51420800, 0x20a22a94, 0x0050a508, 0x00200000, 0x00000000, 0x00050000, 0x08000000, 0xfefbefbe, 0xfbefbefb, 0xfbeb9114, 0x00fbefbe,
        0x20820820, 0x8a28a20a, 0x8a289114, 0x3e8a28a2, 0xfefbefbe, 0xfbefbe0b, 0x8a289114, 0x008a28a2, 0x228a28a2, 0x08208208, 0x8a289114, 0x088a28a2,
        0xfefbefbe, 0xfbefbefb, 0xfa2f9114, 0x00fbefbe, 0x00000000, 0x00000040, 0x00000000, 0x00000000, 0x00000000, 0x00000020, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00210100, 0x00000004, 0x00000000, 0x00000000, 0x14508200, 0x00001402, 0x00000000, 0x00000000,
        0x00000010, 0x00000020, 0x00000000, 0x00000000, 0xa28a28be, 0x00002228, 0x00000000, 0x00000000, 0xa28a28aa, 0x000022e8, 0x00000000, 0x00000000,
        0xa28a28aa, 0x000022a8, 0x00000000, 0x00000000, 0xa28a28aa, 0x000022e8, 0x00000000, 0x00000000, 0xbefbefbe, 0x00003e2f, 0x00000000, 0x00000000,
        0x00000004, 0x00002028, 0x00000000, 0x00000000, 0x80000000, 0x00003e0f, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 };

    std::vector<byte_t> data(c_textureWidth * c_textureWidth * sizeof(uint8_t));
    uint8_t* pImageData = (uint8_t*)data.data();
    for (uint32_t i = 0, counter = 0; i < c_textureWidth * c_textureWidth; i += 32) {
        const uint32_t charData = c_defaultFontData[counter];
        for (int j = 31; j >= 0; j--) {
            if (BIT_CHECK(charData, j)) {
                pImageData[i + j] = 0xff;
            } else {
                pImageData[i + j] = 0;
            }
        }

        counter++;
    }

    constexpr uint8_t charsWidth[224] = {
        3, 1, 4, 6, 5, 7, 6, 2, 3, 3, 5, 5, 2, 4, 1, 7, 5, 2, 5, 5, 5, 5, 5, 5, 5, 5, 1, 1, 3, 4, 3, 6,
        7, 6, 6, 6, 6, 6, 6, 6, 6, 3, 5, 6, 5, 7, 6, 6, 6, 6, 6, 6, 7, 6, 7, 7, 6, 6, 6, 2, 7, 2, 3, 5,
        2, 5, 5, 5, 5, 5, 4, 5, 5, 1, 2, 5, 2, 5, 5, 5, 5, 5, 5, 5, 4, 5, 5, 5, 5, 5, 5, 3, 1, 3, 4, 4,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 5, 5, 5, 7, 1, 5, 3, 7, 3, 5, 4, 1, 7, 4, 3, 5, 3, 3, 2, 5, 6, 1, 2, 2, 3, 5, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 7, 6, 6, 6, 6, 6, 3, 3, 3, 3, 7, 6, 6, 6, 6, 6, 6, 5, 6, 6, 6, 6, 6, 6, 4, 6,
        5, 5, 5, 5, 5, 5, 9, 5, 5, 5, 5, 5, 2, 2, 3, 3, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 3, 5 };

    CodepointPage2 codepointPage;
    codepointPage.pageIndex = 0;
    codepointPage.textures[0] = ITexture::TextureLoader(std::move(data), 128, 128, PixelFormat::GrayScale, /*mipMaps*/ 1);

    constexpr uint32_t pageSize = 256;
    constexpr uint32_t codepointCount = 224;
    constexpr uint32_t charsHeight = 10;
    constexpr uint32_t charsDivisor = 1;    // Every char is separated from the consecutive by a 1 pixel divisor, horizontally and vertically
    uint32_t currentLine = 0;
    uint32_t currentPosX = charsDivisor;
    uint32_t testPosX = charsDivisor;

    codepointPage.codepoints.reserve(pageSize);
    for (uint32_t i = 0; i < codepointCount; i++)
    {
        Codepoint2 codepoint;
        codepoint.bearingX = 1;
        codepoint.width = (float)charsWidth[i];
        codepoint.height = 10;
        codepoint.advance = codepoint.width + 1;
        rectf_t rect;
        rect.x = (float)currentPosX;
        rect.y = (float)(charsDivisor + currentLine*(charsHeight + charsDivisor));
        rect.w = (float)charsWidth[i];
        rect.h = (float)charsHeight;

        testPosX += (uint32_t)(rect.w + charsDivisor);

        if (testPosX >= c_textureWidth)
        {
            currentLine++;
            currentPosX = 2 * charsDivisor + charsWidth[i];
            testPosX = currentPosX;

            rect.x = (float)charsDivisor;
            rect.y = (float)(charsDivisor + currentLine*(charsHeight + charsDivisor));
        }
        else
        {
            currentPosX = testPosX;
        }

        codepoint.texCoords = rect.multiply(c_textureWidthScale, c_textureWidthScale);

        codepointPage.codepoints[32 + i] = std::move(codepoint);
    }

    std::shared_ptr<xpf::Font2> spFont = std::make_shared<xpf::Font2>(HideConstructor());
    spFont->m_name = "Default Font";
    spFont->m_isDefaultFont = true;
    spFont->m_supportsTextureRendering = true;
    spFont->m_supportsRasterRendering = false;
    spFont->m_supportsTriangleRendering = false;
    spFont->m_spFontInfo = std::make_shared<FontInfo2>();
    spFont->m_spFontInfo->defaultFont = true;
    spFont->m_spFontInfo->descent = 0;
    spFont->m_spFontInfo->ascent = 8;
    spFont->m_spFontInfo->fontHeight = charsHeight;
    spFont->m_spFontInfo->lineHeight = charsHeight;
    spFont->m_spFontInfo->pageSize = pageSize;
    spFont->m_spFontInfo->pages[0] = std::move(codepointPage);

    return spFont;
}

/*static*/ const std::shared_ptr<Font2>& Font2::GetDefaultFont()
{
    static const std::shared_ptr<xpf::Font2> s_defaultFont = LoadDefaultFont();
    return s_defaultFont;
}
#pragma endregion

static std::vector<byte_t> LoadFontFile(xpf::string_viewex fontName)
{
    std::vector<byte_t> data = FileSystem::LoadFile(fontName);
    if (!data.empty())
        return data;

    if (fontName.starts_with("/"))
        return FileSystem::LoadFile(fontName);

    data = FileSystem::LoadFile("./bin/" + fontName);
    if (!data.empty())
        return data;
#if defined(PLATFORM_APPLE)
    std::string path = "/System/Library/Fonts/" + fontName;
    data = FileSystem::LoadFile(path);
    if (!data.empty())
        return data;

    path = "/System/Library/Fonts/Supplemental/" + fontName;
    return FileSystem::LoadFile(path);
#elif defined(PLATFORM_WINDOWS)
    std::string path = "c:\\windows\\Fonts\\" + fontName;
    return FileSystem::LoadFile(path);
#endif
}

/*static*/ const std::shared_ptr<Font2>& Font2::GetFont(
    std::string_view fontName,
    uint16_t fontIndex)
{
    if (fontName.empty())
        return GetDefaultFont();

    std::string lookup = fontName + "_" + std::to_string(fontIndex);
    auto iter = s_installedFonts.find(lookup);
    if (iter != s_installedFonts.cend())
        return iter->second;

    xpf::string_viewex ext = FileSystem::GetFileExtension(fontName);
    bool ttf = false;
    bool ttfCollection = false;
    bool openTF = false;
    std::vector<byte_t> data;
    if (ext == ".ttf")
    {
        data = LoadFontFile(fontName);
        ttf = true;
    }
    else if (ext == ".ttc")
    {
        data = LoadFontFile(fontName);
        ttf = true;
        ttfCollection = true;
    }
    else if (ext == ".otf")
    {
        data = LoadFontFile(fontName);
        openTF = true;
    }
    else if (ext.empty())
    {
        data = LoadFontFile(fontName + ".ttf");
        if (!data.empty())
        {
            ttf = true;
        }
        else
        {
            data = LoadFontFile(fontName + ".ttc");
            if (!data.empty())
            {
                ttf = true;
                ttfCollection = true;
            }
            else
            {
                data = LoadFontFile(fontName + ".otf");
                openTF = true;
            }
        }
    }

    if (data.empty())
        return GetDefaultFont();

    auto spInfo = std::make_shared<Font2::FontInfo2>();
    if (!spInfo->InitializeFontInfo(fontName, fontIndex, data.data()))
        return GetDefaultFont();

    std::shared_ptr<xpf::Font2> spFont = std::make_shared<xpf::Font2>(HideConstructor());
    spFont->m_rawFontData = std::move(data);
    spFont->m_spFontInfo = std::move(spInfo);

    return s_installedFonts[lookup] = std::move(spFont);
}

FontMetrics2 Font2::GetFontMetrics(FontSize fontSize) const
{
    float scale = GetScale(fontSize);
    FontMetrics2 metrics;
    metrics.scaledBy = scale;
    metrics.advanceNewLineX = m_spFontInfo->advanceNewLineX * scale;
    metrics.advanceNewLineY = m_spFontInfo->advanceNewLineY * scale;
    metrics.ascent = m_spFontInfo->ascent * scale;
    metrics.descent = m_spFontInfo->descent * scale;
    metrics.fontHeight = m_spFontInfo->fontHeight * scale;
    metrics.lineGap = m_spFontInfo->lineGap * scale;
    metrics.lineHeight = m_spFontInfo->lineHeight * scale;
    return metrics;
}

void Font2::ForEachCodepoint(
    std::string_view text,
    FontSize fontSize,
    FontRenderMode mode,
    std::function<void(
        char32_t,
        const std::shared_ptr<xpf::ITexture>&,
        const std::shared_ptr<xpf::IBuffer>&,
        const Codepoint2&,
        int32_t kern)>&& fn)
{
    if (mode != FontRenderMode::Texture && !m_supportsRasterRendering)
    {
        // use texture as this font don't support raster rendering
        mode = FontRenderMode::Texture;
    }
    else if (mode == FontRenderMode::TriRastered && !m_supportsTriangleRendering)
    {
        // OTF fonts have cubic curves which triangle based rastering is not implemented yet
        // use line rastering instead
        mode = FontRenderMode::LineRastered;
    }

    uint32_t currentPageIndex = UINT32_MAX;
    auto pageIter = m_spFontInfo->pages.end();
    const size_t length = text.length();
    uint32_t prevGlyphIndex = 0;

    size_t i = 0;
    while (i < length)
    {
        char32_t ch = stringex::utf8_to_utf32(text, i);
        if (ch == 0) [[unlikely]]
            break;
        else if (ch == '\n') [[unlikely]]
             prevGlyphIndex = 0;

        uint32_t pageIndex = ch / m_spFontInfo->pageSize;
        if (currentPageIndex != pageIndex)
        {
            pageIter = m_spFontInfo->GetPageIter(pageIndex);
            currentPageIndex = pageIndex;
        }

        const Codepoint2* pCodePoint = m_spFontInfo->GetCodepoint(pageIter, ch, m_defaultCharacter);
        if (pCodePoint == nullptr)
            continue;

        const int32_t newGlyphIndex = pCodePoint->glyphIndex;
        const int32_t kern = m_spFontInfo->GetKern(prevGlyphIndex, newGlyphIndex);
        prevGlyphIndex = newGlyphIndex;

        switch (mode)
        {
            case FontRenderMode::Texture:
                fn(ch, m_spFontInfo->EnsureFontTexture(pageIter->second, fontSize.size), nullptr, *pCodePoint, kern);
                break;
            case FontRenderMode::LineRastered:
                fn(ch, nullptr, m_spFontInfo->EnsureFontLineRasterData(pageIter->second), *pCodePoint, kern);
                break;
            case FontRenderMode::TriRastered:
                fn(ch, nullptr, m_spFontInfo->EnsureFontTriRasterData(pageIter->second), *pCodePoint, kern);
                break;
        }
    }
}

float Font2::GetScale(FontSize fontSize) const
{
    if (m_isDefaultFont || fontSize.inPixels)
        return float(fontSize.size) / m_spFontInfo->fontHeight;

    const float fontSizePixels = fontSize.size * (4.0f / 3.0f);
    return stbtt_ScaleForMappingEmToPixels(&m_spFontInfo->info, fontSizePixels);
}


} // xpf