#include <renderer/common/Font.h>
#include <renderer/IBuffer.h>
#include <renderer/ITexture.h>
#include <core/FileSystem.h>
#include <core/Image.h>
#include <core/Log.h>
#include <core/stringex.h>
#include <math/geometry.h>

#define STB_TRUETYPE_IMPLEMENTATION
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#include <stb/stb_truetype.h>
#pragma clang diagnostic pop

//#include <external/ttf2mesh/ttf2mesh.h>
#include <external/earcut.hpp>

struct Point16
{
    int16_t x = 0, y = 0;
    Point16 operator-(Point16 o) const { return Point16{(int16_t)(x - o.x), (int16_t)(y - o.y)}; }
    static int16_t cross(Point16 a, Point16 b) { return (a.x * b.y - a.y * b.x); }
    // which side of the a-c line lies the point b
    static int16_t normal(Point16 a, Point16 b, Point16 c) { return xpf::math::sign<int16_t>((c.x - a.x) * (-b.y + a.y) + (c.y - a.y) * (b.x - a.x)); }
};

struct Triangle16
{
    Point16 a;
    Point16 b;
    Point16 c;
    uint32_t area;
    int16_t xmin = INT16_MAX;
    int16_t xmax = INT16_MIN;
    int16_t ymin = INT16_MAX;
    int16_t ymax = INT16_MIN;
};

struct triangles
{
    int16_t xmin = INT16_MAX;
    int16_t xmax = INT16_MIN;
    int16_t ymin = INT16_MAX;
    int16_t ymax = INT16_MIN;
    size_t countofpoints_index = 0;
    int16_t point_count = 0;
//    size_t xmin_index = 0;
    std::vector<int16_t> points;

    triangles() = default;

    void initialize(int16_t type)
    {
        points.push_back(type);
        countofpoints_index = points.size();
        points.push_back(0); // count of points
//        xmin_index = points.size();
//        points.push_back(0); // xmin
//        points.push_back(0); // xmax
//        points.push_back(0); // ymin
//        points.push_back(0); // ymax
//        points.push_back(hue); // chue
    }

    void push(const Triangle16 t) { push(t.a, t.b, t.c); }

    void push(Point16 p0, Point16 p1, Point16 p2)
    {
        //points.push_back(type);
        points.push_back(p0.x); points.push_back(p0.y);
        points.push_back(p1.x); points.push_back(p1.y);
        points.push_back(p2.x); points.push_back(p2.y);
        xmin = std::min(p0.x, std::min(p1.x, std::min(p2.x, xmin)));
        xmax = std::max(p0.x, std::max(p1.x, std::max(p2.x, xmax)));
        ymin = std::min(p0.y, std::min(p1.y, std::min(p2.y, ymin)));
        ymax = std::max(p0.y, std::max(p1.y, std::max(p2.y, ymax)));
        point_count += 6;
    }

    void finalize(std::vector<int16_t>& result, size_t countOfContoursIndex)
    {
        if (point_count == 0)
            return;

        points[countofpoints_index] = point_count;
//        points[xmin_index] = xmin;
//        points[xmin_index + 1] = xmax;
//        points[xmin_index + 2] = ymin;
//        points[xmin_index + 3] = ymax;

        for (const auto& value : points)
            result.push_back(value);

        result[countOfContoursIndex]++;
    }
};

struct triangle_range3
{
    int16_t xmin = INT16_MAX;
    int16_t xmax = INT16_MIN;
    int16_t ymin = INT16_MAX;
    int16_t ymax = INT16_MIN;

    std::vector<Triangle16> tris;
    triangle_range3()
    {
    }

    void push(Point16 a, Point16 b, Point16 c)
    {
        Triangle16 tri{a, b, c, 0};
        tri.xmin = std::min(a.x, std::min(b.x, std::min(c.x, tri.xmin)));
        tri.xmax = std::max(a.x, std::max(b.x, std::min(c.x, tri.xmax)));
        tri.ymin = std::min(a.y, std::min(b.y, std::min(c.y, tri.ymin)));
        tri.ymax = std::max(a.y, std::max(b.y, std::min(c.y, tri.ymax)));
        tri.area = abs(int32_t(a.x) * int32_t(b.y - c.y) + int32_t(b.x) * int32_t(c.y - a.y) + int32_t(c.x) * int32_t(a.y - b.y)) / 2;

        xmin = std::min(tri.xmin, xmin);
        xmax = std::max(tri.xmax, xmax);
        ymin = std::min(tri.ymin, ymin);
        ymax = std::max(tri.ymax, ymax);

        tris.push_back(std::move(tri));
    }

    void finalize(int16_t type, std::vector<int16_t>& result, size_t countOfContoursIndex, int16_t xsize = 1, int16_t ysize = 1)
    {
        if (tris.empty())
            return;

        std::sort(tris.begin(), tris.end(), [](const Triangle16& t1, const Triangle16& t2)
        {
            return t1.area > t2.area;
        });

        std::vector<triangles> tricols;
        tricols.resize(1);
//        tricols.resize(xsize * ysize + 1);
        for (triangles& trs : tricols)
            trs.initialize(type);

        const int16_t xw = (xmax - xmin) / xsize;
        const int16_t yw = (ymax - ymin) / ysize;
        for (const auto& tri : tris)
        {
            bool inserted = false;
            for (int16_t y = 0; y < ysize; y++)
            {
                int16_t ys = ymin + (y * yw);
                int16_t ye = ys + yw;
                if (ys <= tri.ymin && tri.ymax <= ye)
                {
                    for (int16_t x = 0; x < xsize; x++)
                    {
                        int16_t xs = xmin + x * xw; int16_t xe = xs + xw;
                        if (xs <= tri.xmin && tri.xmax <= xe)
                        {
                            tricols[y * xsize + x].push(tri);
                            inserted = true;
                            break;
                        }
                    }

                    if (inserted)
                        break;
                }
            }

            if (!inserted)
                tricols.back().push(tri);
        }

        for (triangles& tricol : tricols)
        {
            tricol.finalize(result, countOfContoursIndex);
        }
    }
};

namespace mapbox::util {

template <> struct nth<0, Point16> { inline static auto get(const Point16& p) { return p.x; } };
template <> struct nth<1, Point16> { inline static auto get(const Point16& p) { return p.y; } };

} // mapbox::util

namespace xpf {

// static int stbtt_tesselate_curve(std::vector<Point16>& points, float x0, float y0, float x1, float y1, float x2, float y2, float objspace_flatness_squared = 0.35*.35, int n = 0);
static void stbtt_tesselate_cubic(std::vector<short>& points, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float objspace_flatness_squared = 0.35*.35, int n = 0);

struct FontInfo
{
    stbtt_fontinfo info;
    bool supportsLineShading = false;
    bool supportsTriShading = false;
    bool supportsTextureShading = true;
};

static const CodepointPage s_emptyCodepointPage;

float Font::GetScale(int16_t fontSize) const
{
    if (m_typeface.renderOptions & FontRenderOptions::Shaded || m_typeface.renderOptions & FontRenderOptions::ShadedByTris)
    {
        float font_size_px = std::abs(fontSize) * (4.0f/3.0f);
        return stbtt_ScaleForMappingEmToPixels(&m_spFontInfo->info, font_size_px);
    }

    return ((m_typeface.size == 0) ? 1.0 : fontSize / float(m_typeface.size));
}

void Font::ForEachCodepoint(std::string_view text, std::function<void(char32_t, const CodepointPage&, const Codepoint&, float)>&& fn)
{
    uint32_t currentPageIndex = m_isDefaultFont ? 0 : std::numeric_limits<uint32_t>::max();
    auto pageIter = m_isDefaultFont ? m_pages.cbegin() : m_pages.cend();
    const size_t length = text.length();
    uint32_t prevGlyphIndex = 0;
    const float scaler = (m_typeface.renderOptions & FontRenderOptions::Shaded || m_typeface.renderOptions & FontRenderOptions::ShadedByTris) ? 1.0 : m_scale;

    size_t i = 0;
    while (i < length) {
        const uint32_t ch = stringex::utf8_to_utf32(text, i);
        if (ch == 0) [[unlikely]]
            break;
        else if (ch == '\n') [[unlikely]]
             prevGlyphIndex = 0;

        const uint32_t pageIndex = uint32_t(ch / m_pageSize);
        if (pageIndex != currentPageIndex) [[unlikely]] {
            if (m_isDefaultFont) [[unlikely]] {
                auto questionMark = GetCodepoint(m_typeface.defaultCharacter);
                fn(ch, questionMark.first, questionMark.second, 0);
                continue;
            }

            pageIter = m_pages.find(pageIndex);
            if (pageIter == m_pages.cend()) [[unlikely]] {
                m_pages[pageIndex] = LoadPage(pageIndex);
                pageIter = m_pages.find(pageIndex);
            }

            currentPageIndex = pageIndex;
        }

        const CodepointPage& page = pageIter->second;
        const uint32_t codepointIndex = ch % m_pageSize;

        const auto codepointIter = page.codepoints.find(codepointIndex);
        if (codepointIter == page.codepoints.cend()) [[unlikely]] {
            // not found - draw [?] mark
            auto questionMark = GetCodepoint(m_typeface.defaultCharacter);
            fn(ch, questionMark.first, questionMark.second, 0);
        } else [[likely]] {
            const int32_t kern = GetKern(prevGlyphIndex, codepointIter->second.glyphindex);
            prevGlyphIndex = codepointIter->second.glyphindex;
            fn(ch, page, codepointIter->second, kern * scaler);
        }
    }
}

const CodepointPage& Font::GetCodepointPage(uint32_t ch) const
{
    return m_pages[uint32_t(ch / m_pageSize)];
}

int32_t Font::GetKern(char32_t ch1, char32_t ch2) const
{
    if (m_spFontInfo == nullptr)
        return 0;

    return stbtt__GetGlyphKernInfoAdvance(&m_spFontInfo->info, ch1, ch2);
}

SizeF Font::Measure(std::string_view text) const
{
    float total_width = 0;
    const size_t length = text.length();

    if (m_typeface.fixedFont) {
        size_t i = 0;
        while (i < length) {
            const uint32_t ch = stringex::utf8_to_utf32(text, i);
            if (ch == 0) [[unlikely]]
                break;
            total_width += m_metrics.charWidth;
        }

        return SizeF{total_width, m_metrics.lineHeight};
    }

    uint32_t currentPageIndex = m_isDefaultFont ? 0 : std::numeric_limits<uint32_t>::max();
    auto pageIter = m_isDefaultFont ? m_pages.cbegin() : m_pages.cend();

    size_t i = 0;
    while (i < length) {
        const uint32_t ch = stringex::utf8_to_utf32(text, i);
        if (ch == 0) [[unlikely]]
            break;

        const uint32_t pageIndex = uint32_t(ch / m_pageSize);
        if (pageIndex != currentPageIndex) [[unlikely]] {
            if (m_isDefaultFont) [[unlikely]] {
                auto questionMark = GetCodepoint(m_typeface.defaultCharacter);
                total_width += questionMark.second.advance;
                continue;
            }

            pageIter = m_pages.find(pageIndex);
            if (pageIter == m_pages.cend()) [[unlikely]] {
                m_pages[pageIndex] = LoadPage(pageIndex);
                pageIter = m_pages.find(pageIndex);
            }

            currentPageIndex = pageIndex;
        }

        const CodepointPage& page = pageIter->second;
        const uint32_t codepointIndex = ch % m_pageSize;

        const auto codepointIter = page.codepoints.find(codepointIndex);
        if (codepointIter == page.codepoints.cend()) [[unlikely]] {
            // not found - draw [?] mark
            auto questionMark = GetCodepoint(m_typeface.defaultCharacter);
            total_width += questionMark.second.advance;
        } else [[likely]] {
            total_width += codepointIter->second.advance;
        }
    }

    return SizeF{total_width, m_metrics.lineHeight};
}

const std::pair<CodepointPage&, const Codepoint&> Font::GetCodepoint(char32_t ch) const
{
    static Codepoint s_emptyCodepointInfo;
    static CodepointPage s_emptyPage;

    const uint32_t pageIndex = uint32_t(ch / m_pageSize);
    auto pageIter = m_pages.find(pageIndex);
    if (pageIter == m_pages.cend()) {
        if (m_isDefaultFont)
            return {s_emptyPage, s_emptyCodepointInfo};

        m_pages[pageIndex] = LoadPage(pageIndex);
        pageIter = m_pages.find(pageIndex);
    }

    const CodepointPage& page = pageIter->second;
    const uint32_t codepointIndex = ch % m_pageSize;

    const auto codepointIter = page.codepoints.find(codepointIndex);
    if (codepointIter == page.codepoints.cend())
        return {s_emptyPage, s_emptyCodepointInfo};

    return {pageIter->second, codepointIter->second};
}

void earcut(std::vector<std::vector<Point16>>& polygon, int16_t ascent16, triangle_range3& tris)
{
    std::vector<size_t> indices = mapbox::earcut<size_t>(polygon);

    auto getPoint = [&](size_t i)
    {
        for (const auto& part : polygon)
        {
            if (i < part.size())
            {
                const auto& v = part[i];
                return Point16{v.x, (int16_t)(ascent16 - v.y)};
            }

            i -= part.size();
        }

        throw std::exception(); // invalid earcut result;
    };

    for(size_t j = 0; j < indices.size(); j += 3)
    {
        Point16 p0 = getPoint(indices[j + 0]);
        Point16 p1 = getPoint(indices[j + 1]);
        Point16 p2 = getPoint(indices[j + 2]);
        tris.push(p0, p1, p2);
    }
}

CodepointPage Font::LoadPageForShadedTris(uint32_t pageIndex) const
{
    uint32_t pageStart = pageIndex * c_pageSize;
    uint32_t pageEnd = pageStart + c_pageSize;
    int16_t ascent16 = m_metrics.ascent;
    const stbtt_fontinfo* pfont_info = &(m_spFontInfo->info);

    std::vector<int16_t> tris;
    tris.push_back(1);

    CodepointPage codepointPage;
    for (uint32_t ch32 = pageStart; ch32 < pageEnd; ch32++)
    {
        if (ch32 == 0xe7) // 'ç')
            ch32 += 0;
        const int32_t glyphIndex = stbtt_FindGlyphIndex(pfont_info, int32_t(ch32));
        int32_t advanceX = 0;
        int32_t bearingX = 0;
        stbtt_GetGlyphHMetrics(pfont_info, glyphIndex, &advanceX, &bearingX);

        int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
        stbtt_GetGlyphBitmapBoxSubpixel(pfont_info, glyphIndex, 1.0, 1.0, 0.0, 0.0, &x0,&y0,&x1,&y1);
        int32_t width = x1 - x0;
        int32_t height = y1 - y0;
        int32_t xoffset = x0;
        int32_t yoffset = y0;

        stbtt_vertex* pvertices = nullptr;
        int32_t num_vertices = stbtt_GetGlyphShape(pfont_info, glyphIndex, &pvertices);

        Codepoint& codepoint = codepointPage.codepoints[ch32 - pageStart];
        codepoint.glyphindex = glyphIndex;
        codepoint.xoffset = xoffset;
        codepoint.yoffset = yoffset;
        codepoint.width = width;
        codepoint.height = height;
        codepoint.bearingX = bearingX;
        codepoint.advance = advanceX;
        codepoint.texCoords = {0,0,1,1};
        codepoint.glyphStartOffset = static_cast<uint32_t>(tris.size());
        size_t countOfContoursIndex = tris.size();
        tris.push_back(0); // countOfContoursIndex;

        int16_t x = 0, y = 0;
        std::vector<std::vector<Point16>> polygon;
        std::vector<Point16>& contour = polygon.emplace_back();
        std::vector<triangle_range3> triangles;
        triangle_range3 current;
        triangle_range3 bz;
        triangle_range3 bz_ccw;

        for (int32_t vindex = 0; vindex < num_vertices; vindex++)
        {
            stbtt_vertex& vrtx = pvertices[vindex];
            vrtx.y += m_metrics.baseline + yoffset;
            vrtx.cy += m_metrics.baseline + yoffset;
            vrtx.cy1 += m_metrics.baseline + yoffset;
            if (vrtx.type == STBTT_vmove)
            {
                if (contour.size() != 0)
                {
                    if (contour.size() < 3)
                        throw std::exception(); // invalid contour

                    earcut(polygon, ascent16, current);
                    contour.clear();
                    triangles.push_back(std::move(current));
                    current = triangle_range3();
                }
            }
            else if (vrtx.type == STBTT_vline)
            {
                Point16 a{x, y};
                contour.push_back(a);
            }
            else if (vrtx.type == STBTT_vcurve)
            {
                contour.push_back({x, y});
                Point16 a{x, (int16_t)(ascent16 - y)};
                Point16 b{vrtx.cx, (int16_t)(ascent16 - vrtx.cy)};
                Point16 c{vrtx.x, (int16_t)(ascent16 - vrtx.y)};
                bool ccw = Point16::normal(a, b, c) < 0;
                if (ccw)
                {
                    contour.push_back(Point16{vrtx.cx, vrtx.cy});
                    bz_ccw.push(a, b, c);
                }
                else
                {
                    bz.push(a, b, c);
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
            earcut(polygon, ascent16, current);
            contour.clear();
            triangles.push_back(std::move(current));
        }

        for (auto& bzi : triangles)
            bzi.finalize(0, tris, countOfContoursIndex);
        bz_ccw.finalize(1, tris, countOfContoursIndex);
        bz.finalize(-1,tris, countOfContoursIndex);

        STBTT_free(pvertices, pfont_info->userdata);
    }

    codepointPage.spBuffer = IBuffer::BufferLoader(reinterpret_cast<const byte_t*>(tris.data()), tris.size() * sizeof(tris[0]));

    return codepointPage;
}

CodepointPage Font::LoadPage(uint32_t pageIndex) const
{
    const bool isShaderRendered = m_supportsLineShading && m_typeface.renderOptions & FontRenderOptions::Shaded;
    const bool isShaderRenderedByTris = m_supportsTriShading && m_typeface.renderOptions & FontRenderOptions::ShadedByTris;

    if (isShaderRenderedByTris)
        return LoadPageForShadedTris(pageIndex);

    uint32_t pageStart = pageIndex * c_pageSize;
    uint32_t pageEnd = pageStart + c_pageSize;
    int32_t atlasWidth = 0, atlasHeight = 0;

    int16_t ascent16 = m_metrics.ascent;
    const stbtt_fontinfo* pfont_info = &(m_spFontInfo->info);
    std::vector<byte_t> atlas;
    uint32_t atlasX = 0;
    float atlasWidthScale = 0;
    float atlasHeightScale = 0;

    if (!isShaderRendered) // cpu rendered fonts
    {
        for (char32_t ch32 = pageStart; ch32 < pageEnd; ch32++)
        {
            int32_t ix0 = 0, iy0 = 0, ix1 = 0, iy1 = 0;
            stbtt_GetCodepointBitmapBox(pfont_info, int32_t(ch32), m_scale, m_scale, &ix0, &iy0, &ix1, &iy1);
            atlasWidth += (ix1 -ix0);
            int32_t height = (iy1 -iy0);
            if (height > atlasHeight)
                atlasHeight = height;
        }

        atlasWidthScale = 1.0f / float(atlasWidth);
        atlasHeightScale = 1.0f / float(atlasHeight);
        atlas = std::vector<byte_t>(atlasWidth * atlasHeight * sizeof(uint8_t));
    }

    std::vector<int16_t> points;
    std::vector<int16_t> bezierCurvePoints;
    points.push_back(0);

    CodepointPage codepointPage;
    for (uint32_t ch32 = pageStart; ch32 < pageEnd; ch32++)
    {
        const int32_t glyphIndex = stbtt_FindGlyphIndex(pfont_info, int32_t(ch32));

        int32_t advanceX = 0;
        int32_t bearingX = 0;
        stbtt_GetGlyphHMetrics(pfont_info, glyphIndex, &advanceX, &bearingX);

        Codepoint codepoint;
        // Calculate font basic metrics
        // NOTE: ascent is equivalent to font baseline
        int32_t width = 0, height = 0, xoffset = 0, yoffset = 0;
        if (isShaderRendered) // shader rendered fonts
        {
            codepoint.glyphStartOffset = static_cast<uint32_t>(points.size());

            int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
            stbtt_GetGlyphBitmapBoxSubpixel(pfont_info, glyphIndex, 1.0, 1.0, 0.0, 0.0, &x0,&y0,&x1,&y1);
            width = x1 - x0;
            height = y1 - y0;
            xoffset = x0;
            yoffset = y0;

            stbtt_vertex* pvertices = nullptr;
            int count = stbtt_GetGlyphShape(pfont_info, glyphIndex, &pvertices);
            if (count > 0)
            {
                int16_t x = 0, y = 0;

                size_t glyphStart = points.size();
                points.push_back(0); // reserve space for length of glyph data.
                size_t contourStart = points.size(); // start of contour: len + (x,y: (short * 2)*len)
                for (int i = 0; i < count; i++)
                {
                    stbtt_vertex& vrtx = pvertices[i];
                    vrtx.y += m_metrics.baseline + yoffset;
                    vrtx.cy += m_metrics.baseline + yoffset;
                    vrtx.cy1 += m_metrics.baseline + yoffset;

                    if (vrtx.type == STBTT_vmove)
                    {
                        if (points.size() - contourStart > 0)
                            points[contourStart] = (points.size() - (contourStart + 1)) / 2;

                        contourStart = (int)points.size();

                        points.push_back(0); // space for contour count - sign shows winding direction
                        points.push_back(vrtx.x); points.push_back(ascent16 - vrtx.y);
                    }
                    else if (vrtx.type == STBTT_vline)
                    {
                        points.push_back(vrtx.x); points.push_back(ascent16 - vrtx.y);
                    }
                    else if (vrtx.type == STBTT_vcurve)
                    {
    #if TESSELATE
                        stbtt_tesselate_curve(
                            points,
                            x, (ascent - y),
                            vrtx.cx, (ascent - vrtx.cy),
                            vrtx.x, (ascent - vrtx.y));
    #else
                        bezierCurvePoints.push_back(x);
                        bezierCurvePoints.push_back((ascent16 - y));

                        bezierCurvePoints.push_back(vrtx.cx);
                        bezierCurvePoints.push_back((ascent16 - vrtx.cy));

                        bezierCurvePoints.push_back(vrtx.x);
                        bezierCurvePoints.push_back((ascent16 - vrtx.y));
                        points.push_back(vrtx.x); points.push_back(ascent16 - vrtx.y);
    #endif
                    }
                    else if (vrtx.type == STBTT_vcubic)
                    {
                        stbtt_tesselate_cubic(
                            points,
                            x, (ascent16 - y),
                            vrtx.cx, (ascent16 - vrtx.cy),
                            vrtx.cx1, (ascent16 - vrtx.cy1),
                            vrtx.x, (ascent16 - vrtx.y));
                    }
                    else
                    {
                        throw std::exception();
                    }

                    x = vrtx.x; y = vrtx.y;
                }

                points[contourStart] = (points.size() - (contourStart + 1)) / 2;
                points[glyphStart] = (points.size() - codepoint.glyphStartOffset);

                points.push_back(bezierCurvePoints.size() / 6);
                for (const int16_t v : bezierCurvePoints)
                    points.push_back(v);
                bezierCurvePoints.clear();

                STBTT_free(pvertices, pfont_info->userdata);
            }
        }
        else
        {
            // if (m_use_sdf_to_generate_fonts) {
            //     pageData = stbtt_GetCodepointSDF(pfont_info, m_scale, ch32, /*padding:*/4, /*onedge_value:*/ 255, /*pixel_dist_scale:*/ 64.0f, &width, &height, &xoffset, &yoffset);
            // } else {
            // }

            uint8_t* pImageData = (uint8_t*)atlas.data();
            byte_t* pageData = stbtt_GetGlyphBitmap(pfont_info, m_scale, m_scale, glyphIndex, &width, &height, &xoffset, &yoffset);
            byte_t* pdata = pageData;

            for (int32_t h = 0; h < height; h++) {
                for (int32_t p = 0; p < width; p++) {
                    uint8_t val = *(pdata++);
                    pImageData[atlasX + p + h * atlasWidth] = val;
                }
            }

            stbtt_FreeBitmap(pageData, /*userdata:*/ nullptr);
        }

        codepoint.glyphindex = glyphIndex;
        codepoint.xoffset = xoffset;
        codepoint.yoffset = yoffset;
        codepoint.width = width;
        codepoint.height = height;
        if (isShaderRendered)
        {
            codepoint.bearingX = bearingX;
            codepoint.advance = advanceX;
            codepoint.texCoords = {0,0,1,1};
        }
        else
        {
            codepoint.bearingX = bearingX * m_scale;
            codepoint.advance = advanceX * m_scale;
            rectf_t rect;
            rect.x = atlasX;
            rect.y = 0;
            rect.w = width;
            rect.h = height;
            codepoint.texCoords = rect.multiply(atlasWidthScale, atlasHeightScale);
        }

        codepointPage.codepoints[ch32 - pageStart] = std::move(codepoint);

        atlasX += width;
    }

    if (m_typeface.renderOptions & FontRenderOptions::Shaded)
    {
        codepointPage.spBuffer = IBuffer::BufferLoader(reinterpret_cast<const byte_t*>(points.data()), points.size() * sizeof(points[0]));
    }
    else
    {
        codepointPage.spTexture = ITexture::TextureLoader(std::move(atlas), atlasWidth, atlasHeight, PixelFormat::GrayScale, /*mipMapCount*/ 1);
    }

    return codepointPage;
}

/*static*/ const std::shared_ptr<xpf::Font>& Font::GetDefaultFont()
{
    static const std::shared_ptr<xpf::Font> s_defaultFont = LoadDefaultFont();
    return s_defaultFont;
}

/*static*/ std::shared_ptr<xpf::Font> Font::LoadDefaultFont() {
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

    CodepointPage codepointPage;
    codepointPage.spTexture = ITexture::TextureLoader(std::move(data), 128, 128, PixelFormat::GrayScale, /*mipMaps*/ 1);

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
        Codepoint codepoint;
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

    std::shared_ptr<xpf::Font> spFont = std::make_shared<xpf::Font>(HideConstructor());
    spFont->m_typeface.name = "Default xpf::font";
    spFont->m_typeface.size = 10;
    spFont->m_typeface.index = 0;
    spFont->m_typeface.fixedFont = false;
    spFont->m_typeface.defaultFont = true;
    spFont->m_typeface.renderOptions = FontRenderOptions::Texture;
    spFont->m_metrics.descent = 0;
    spFont->m_metrics.ascent = 8;
    spFont->m_pageSize = pageSize;
    spFont->m_metrics.fontHeight = charsHeight;
    spFont->m_metrics.lineHeight = charsHeight;
    spFont->m_metrics.advanceNewlineX = 0.0f;
    spFont->m_metrics.advanceNewlineY = 1.0f;
    spFont->m_pages[0] = std::move(codepointPage);
    spFont->m_isDefaultFont = true;

    return spFont;
}

/*static*/ std::shared_ptr<Font> Font::LoadFont(const Typeface& typeface)
{
    xpf::Font font;
    font.m_pageSize = c_pageSize;
    font.m_typeface = typeface;
    if (font.m_typeface.size == 0)
        font.m_typeface.size = 10;

    const FontData& fontData = s_loadedTrueTypeFile[typeface.name];
    const std::vector<byte_t>& data = fontData.data;
    font.m_supportsLineShading = fontData.supportsLineShading;
    font.m_supportsTextureShading = fontData.supportsTextureShading;
    font.m_supportsTriShading = fontData.supportsTriShading;

    stbtt_fontinfo fontInfo;
    if (!stbtt_InitFont(
        &fontInfo,
        data.data(),
        stbtt_GetFontOffsetForIndex(data.data(), font.m_typeface.index))) {

        Log::error(
            "Font: " + font.m_typeface.name +
            " did not have font at index " + std::to_string(font.m_typeface.index) +
            " trying at index 0.");

        if (!stbtt_InitFont(
            &fontInfo,
            data.data(),
            stbtt_GetFontOffsetForIndex(data.data(), 0))) {

            Log::error("Font: " + font.m_typeface.name + " failed to initialize.");
            return nullptr;
        }
    }

    // Get the font metrics, the the stbtt_ScaleForMappingEmToPixels() and stbtt_GetFontVMetrics() documentation
    // for details.
    //
    // From "Font Size in Pixels or Points" in stb_truetype.h
    // > Windows traditionally uses a convention that there are 96 pixels per inch, thus making 'inch'
    // > measurements have nothing to do with inches, and thus effectively defining a point to be 1.333 pixels.
    float font_size_px = font.m_typeface.size;
    if (!(typeface.renderOptions & FontRenderOptions::SizeInPixels))
        font_size_px *= (4.0f / 3.0f);
    font.m_scale = stbtt_ScaleForMappingEmToPixels(&fontInfo, font_size_px);

    int32_t ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);

    if (typeface.renderOptions & FontRenderOptions::Shaded || typeface.renderOptions & FontRenderOptions::ShadedByTris)
    {
        font.m_metrics.lineHeight = (ascent - descent + lineGap);  // Based on the docs of stbtt_GetFontVMetrics()
        font.m_metrics.ascent = ascent;
        font.m_metrics.baseline = font.m_metrics.ascent;
        font.m_metrics.descent = -descent;
        font.m_metrics.fontHeight = font.m_metrics.ascent + font.m_metrics.descent;
    }
    else // typeface.renderOptions & FontRenderOptions::Texture
    {
        font.m_metrics.lineHeight = (ascent - descent + lineGap) * font.m_scale;  // Based on the docs of stbtt_GetFontVMetrics()
        font.m_metrics.ascent = ascent * font.m_scale;
        font.m_metrics.baseline = font.m_metrics.ascent;
        font.m_metrics.descent = -descent * font.m_scale;
        font.m_metrics.fontHeight = font.m_metrics.ascent + font.m_metrics.descent;
    }

    font.m_spFontInfo = std::make_shared<FontInfo>();
    font.m_spFontInfo->info = std::move(fontInfo);

    if (font.m_typeface.fixedFont) {
        std::string_view s_textToMeasure = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        float total_width = font.Measure(s_textToMeasure).width;
        font.m_metrics.charWidth = round(total_width / s_textToMeasure.length());
    }

    return std::make_shared<xpf::Font>(std::move(font));
}

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

/*static*/ const std::shared_ptr<Font>& Font::GetFont(const Typeface& typeface)
{
    if (s_failedToLoadFonts.find(typeface.name) != s_failedToLoadFonts.end())
        return Font::GetDefaultFont();

    if (typeface.name.empty())
        return Font::GetDefaultFont();

    std::string lookup = typeface.name + " "
        + std::to_string(typeface.renderOptions & FontRenderOptions::Shaded || typeface.renderOptions & FontRenderOptions::ShadedByTris ? 0 : typeface.size)
        + "_"
        + std::to_string(typeface.index)
        + (typeface.renderOptions & FontRenderOptions::ShadedByTris ? "t" : "_")
        + (typeface.renderOptions & FontRenderOptions::SizeInPixels ? "_p" : "_e");
    const auto iter = s_installedFonts.find(lookup);
    if (iter != s_installedFonts.cend())
        return iter->second;

    if (s_loadedTrueTypeFile.contains(typeface.name))
    {
        const std::shared_ptr<xpf::Font> spFont = Font::LoadFont(typeface);
        if (spFont != nullptr)
        {
            s_installedFonts[lookup] = spFont;
            return s_installedFonts[lookup];
        }
    }

    FontData fontData;
    if (s_fontLoader != nullptr)
        fontData.data = s_fontLoader(typeface.name);

    if (fontData.data.empty())
        fontData.data = LoadFontFile(typeface.name + ".ttf");

    if (fontData.data.empty())
        fontData.data = LoadFontFile(typeface.name + ".ttc");

    if (fontData.data.empty())
    {
        fontData.data = LoadFontFile(typeface.name + ".otf");
        fontData.supportsLineShading = false;
        fontData.supportsTriShading = false;
    }

    if (!fontData.data.empty())
    {
        s_loadedTrueTypeFile[typeface.name] = std::move(fontData);

        const std::shared_ptr<xpf::Font> spFont = Font::LoadFont(typeface);
        if (spFont != nullptr)
        {
            s_installedFonts[lookup] = spFont;
            return s_installedFonts[lookup];
        }
    }

    if (fontData.data.empty())
        s_failedToLoadFonts.insert(typeface.name);

    return Font::GetDefaultFont();
}

#pragma region vector font
/*
static int stbtt_tesselate_curve(std::vector<Point16>& points, float x0, float y0, float x1, float y1, float x2, float y2, float objspace_flatness_squared, int n)
{
    // midpoint
    float mx = (x0 + 2*x1 + x2)/4;
    float my = (y0 + 2*y1 + y2)/4;
    // versus directly drawn line
    float dx = (x0+x2)/2 - mx;
    float dy = (y0+y2)/2 - my;
    if (n > 16) // 65536 segments on one curve better be enough!
        return 1;
    if (dx*dx+dy*dy > objspace_flatness_squared) { // half-pixel error allowed... need to be smaller if AA
        stbtt_tesselate_curve(points, x0,y0, (x0+x1) * .5f, (y0+y1) * .5f, mx,my, objspace_flatness_squared, n+1);
        stbtt_tesselate_curve(points, mx,my, (x1+x2) * .5f, (y1+y2) * .5f, x2,y2, objspace_flatness_squared, n+1);
    } else {
        points.push_back(Point16{(short)x2,(short)y2});
    }
   return 1;
}
*/

static void stbtt_tesselate_cubic(std::vector<short>& points, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float objspace_flatness_squared, int n)
{
    // @TODO this "flatness" calculation is just made-up nonsense that seems to work well enough
    float dx0 = x1-x0;
    float dy0 = y1-y0;
    float dx1 = x2-x1;
    float dy1 = y2-y1;
    float dx2 = x3-x2;
    float dy2 = y3-y2;
    float dx = x3-x0;
    float dy = y3-y0;
    float longlen = (std::sqrtf(dx0*dx0+dy0*dy0)+std::sqrtf(dx1*dx1+dy1*dy1)+std::sqrtf(dx2*dx2+dy2*dy2));
    float shortlen_squared = (dx*dx+dy*dy);
    float flatness_squared = longlen*longlen-shortlen_squared;

    if (n > 16) // 65536 segments on one curve better be enough!
        return;

    if (flatness_squared > objspace_flatness_squared) {
        float x01 = (x0+x1)/2;
        float y01 = (y0+y1)/2;
        float x12 = (x1+x2)/2;
        float y12 = (y1+y2)/2;
        float x23 = (x2+x3)/2;
        float y23 = (y2+y3)/2;

        float xa = (x01+x12)/2;
        float ya = (y01+y12)/2;
        float xb = (x12+x23)/2;
        float yb = (y12+y23)/2;

        float mx = (xa+xb)/2;
        float my = (ya+yb)/2;

        stbtt_tesselate_cubic(points, x0,y0, x01,y01, xa,ya, mx,my, objspace_flatness_squared, n+1);
        stbtt_tesselate_cubic(points, mx,my, xb,yb, x23,y23, x3,y3, objspace_flatness_squared, n+1);
    }
    else
    {
       points.push_back((short)x3); points.push_back((short)y3);
    }
}
#pragma endregion

} // xpf