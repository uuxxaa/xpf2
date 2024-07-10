#include <core/stringex.h>
#include <core/Image.h>
#include <core/Log.h>
#include <core/Tween.h>
#include <math/geometry.h>
#include <renderer/ITexture.h>
#include <renderer/common/RenderBatchBuilder.h>
#include <windows/Application.h>
#include <windows/InputService.h>
#include <windows/Window.h>
#include <iostream>

#include <renderer/common/Font.h>
#include <core/FileSystem.h>
#include <external/earcut.hpp>
#include <external/ttf2mesh/ttf2mesh.h>
#include <stb/stb_truetype.h>

using namespace xpf;

extern "C" {
ttf_outline_t* allocate_ttf_outline(int ncontours, int npoints);
bool ttf_outline_contour_info_majority(const ttf_outline_t *outline, int subglyph_order, int contour, int *nested_to);

static bool compare_contours(const ttf_outline_t *o, int i1, int i2)
{
    if (o->cont[i1].length != o->cont[i2].length) return false;
    for (int i = 0; i < o->cont[i1].length; i++)
    {
        float dx = o->cont[i1].pt[i].x - o->cont[i2].pt[i].x;
        float dy = o->cont[i1].pt[i].y - o->cont[i2].pt[i].y;
        if (fabsf(dx) > xpf::math::Epsilon || fabsf(dy) > xpf::math::Epsilon) return false;
    }
    return true;
}
}

void test_font(IRenderer& renderer) {
    auto init = [](IRenderer& renderer) {
        auto r = renderer.CreateCommandBuilder();

        std::string path = "/Users/umutalev/Library/Fonts/JetBrainsMono-Regular.ttf";
        auto data = FileSystem::LoadFile(path);

        stbtt_fontinfo fontInfo;
        stbtt_InitFont(&fontInfo, data.data(), 0);

        xpf::Color fillColor = xpf::Colors::XpfWhite;
        xpf::Color borderColor = xpf::Colors::XpfWhite.with_alpha(64);
        float lineThickness = 1;
        std::string text = "0"; // B10 The quick fox jumped over the lazy dog.";
        size_t length = text.length();

        float xorg = 10, yorg = 440.5;
        float scale = 1.0f/3;
        auto pxy =  [&](auto xy) {
            return v2_t(floor(xorg + xy.x * scale)+.5, floor(yorg - xy.y * scale)+.5);
        };

        size_t istr = 0;
        int advanceWidth = 0;
        while (istr < length) {
            xorg += advanceWidth * scale;

            const uint32_t ch = stringex::utf8_to_utf32(text, istr);
            stbtt_vertex* pvertices = nullptr;

            int leftSideBearing;
            stbtt_GetCodepointHMetrics(&fontInfo, int32_t(ch), &advanceWidth, &leftSideBearing);

            int num_vertices = stbtt_GetCodepointShape(&fontInfo, int32_t(ch), &pvertices);
            if (num_vertices <= 0)
                continue;

            int contour_count = 0;
            int ttf_outline_vertex_count = 0;
            for (int vindex = 0; vindex < num_vertices; vindex++) {
                const stbtt_vertex& vrtx = pvertices[vindex];
                if (vrtx.type == STBTT_vmove)
                {
                    contour_count++;
                }
                else if (vrtx.type == STBTT_vline)
                {
                    ttf_outline_vertex_count += 2;
                }
                else if (vrtx.type == STBTT_vcurve)
                {
                    ttf_outline_vertex_count += 3;
                }
                else if (vrtx.type == STBTT_vcubic)
                {
                    throw std::exception(); // NYI
                }
            }

            ttf_outline_t* outline = allocate_ttf_outline(contour_count, ttf_outline_vertex_count * 2);

            int16_t x = 0, y = 0;
            int contour_index = 0;
            int npoints = 0;

            auto completeContour = [&]() {
                if (npoints == 0) return;
                outline->cont[contour_index].length = npoints;
                outline->total_points += npoints;
                outline->cont[contour_index].subglyph_id = outline->cont[contour_index].subglyph_id;
                outline->cont[contour_index].subglyph_order = outline->cont[contour_index].subglyph_order;
                if (contour_index != outline->ncontours - 1)
                    outline->cont[contour_index + 1].pt = outline->cont[contour_index].pt + npoints;
                contour_index++;
                npoints = 0;
            };

            for (int vindex = 0; vindex < num_vertices; vindex++) {
                const stbtt_vertex& vrtx = pvertices[vindex];
                const auto& contour = outline->cont[contour_index];
                if (vrtx.type == STBTT_vmove)
                {
                    completeContour();
                }
                else if (vrtx.type == STBTT_vline)
                {
                    contour.pt[npoints++] = {float(x), float(y)};
                    contour.pt[npoints++] = {float(vrtx.x), float(vrtx.y)};
                }
                else if (vrtx.type == STBTT_vcurve)
                {
                    const bool ccw = r.DrawBezierQuadraticTriangle(
                        pxy(v2_t{float(x), float(y)}),
                        pxy(v2_t{float(vrtx.cx), float(vrtx.cy)}),
                        pxy(v2_t{float(vrtx.x), float(vrtx.y)}),
                        fillColor);

                    contour.pt[npoints++] = {float(x), float(y)};
                    if (ccw)
                        contour.pt[npoints++] = {float(vrtx.cx), float(vrtx.cy)};

                    contour.pt[npoints++] = {float(vrtx.x), float(vrtx.y)};
                }
                else if (vrtx.type == STBTT_vcubic)
                {
                    throw std::exception(); // NYI
                }

                x = vrtx.x; y = vrtx.y;
            }

            completeContour();

            std::vector<std::vector<d2_t>> holes;
            std::vector<d2_t> hull;

            for (int i = 0; i < outline->ncontours; i++)
            {
                const auto& contour = outline->cont[i];
                const ttf_point_t* pt = contour.pt;
                int len = contour.length;
                if (len < 3) continue;
                bool duplicated = false;
                for (int j = 0; j < i && !duplicated; j++) {
                    duplicated = compare_contours(outline, i, j);
                    if (duplicated) break;
                }

                if (duplicated) continue;
                int isNestedTo = -1;
                bool is_hole = !ttf_outline_contour_info_majority(outline, contour.subglyph_order, i, &isNestedTo);
                if (is_hole) {
                    std::vector<d2_t>& hole = holes.emplace_back();
                    for (int j = 0; j < contour.length; j++) {
                        hole.push_back({contour.pt[j].x, contour.pt[j].y});
                    }
                } else {
                    for (int j = 0; j < contour.length; j++) {
                        hull.push_back({contour.pt[j].x, contour.pt[j].y});
                    }
                }
            }


            holes.insert(holes.begin(), std::move(hull));

            // Run tessellation
            // Returns array of indices that refer to the vertices of the input polygon.
            // e.g: the index 6 would refer to {25, 75} in this example.
            // Three subsequent indices form a triangle. Output triangles are clockwise.


            r.Flush();

            // int n = 0;
            // std::vector<uint16_t> indices = mapbox::earcut<uint16_t>(holes);
            // for(uint16_t j = 0; j < indices.size(); j+=3) {
            //     const auto& contour = outline->cont[n];
            //     r.DrawTriangle(
            //         pxy(contour.pt[indices[j + 1]]),
            //         pxy(contour.pt[indices[j + 2]]),
            //         pxy(contour.pt[indices[j + 0]]),
            //         fillColor);
            // }

            for (int vindex = 0; vindex < num_vertices; vindex++) {
                const stbtt_vertex& vrtx = pvertices[vindex];
                const auto& contour = outline->cont[contour_index];
                if (vrtx.type == STBTT_vmove)
                {
                }
                else if (vrtx.type == STBTT_vline)
                {
                    r.DrawLine(
                        pxy(v2_t{float(x), float(y)}),
                        pxy(v2_t{float(vrtx.x), float(vrtx.y)}),
                        lineThickness, borderColor,
                        LineOptions::CapRound);
                }
                else if (vrtx.type == STBTT_vcurve)
                {
                    r.DrawBezierQuadratic(
                        pxy(v2_t{float(x), float(y)}),
                        pxy(v2_t{float(vrtx.cx), float(vrtx.cy)}),
                        pxy(v2_t{float(vrtx.x), float(vrtx.y)}),
                        lineThickness, borderColor,
                        LineOptions::CapRound);
                }
                else if (vrtx.type == STBTT_vcubic)
                {
                    throw std::exception(); // NYI
                }

                x = vrtx.x; y = vrtx.y;
            }

            free(outline);
        }

        return r.Build();
    };

    static auto batchedCmds = init(renderer);
    renderer.EnqueueCommands(batchedCmds);
}

#define TEST(x) case __LINE__: m_testid =__LINE__; x(renderer); draw_test_name(renderer, #x); break;

class MyWindow : public Window
{
protected:
    int m_testid = -1;
    xpf::time_t m_next_testid_switch = 0;

public:
    MyWindow(WindowOptions&& options) : Window(std::move(options)) { }

    virtual bool OnSetup(IRenderer& r) override
    {
        return true;
    }

    void OnDraw(IRenderer& renderer) override
    {
        if (Time::GetTime() > m_next_testid_switch) {
            m_next_testid_switch = Time::GetTime() + 1;
            m_testid++;
        }

        switch (m_testid) {
            default: m_testid = 0; [[fallthrough]];
            TEST(test_font);
        }
    }

    void draw_test_name(IRenderer& renderer, const xpf::string_viewex name) {
        TextDescription desc;
        desc.fontName = "";
        desc.fontSize = 20;
        desc.foreground = xpf::Colors::Lime;
        desc.background = xpf::Colors::Black.with_alpha(80);
        auto scope = renderer.Transform(m4_t::identity);
        renderer.DrawText(name, 10, 10, desc);
    }
};

class MyApp : public xpf::Application
{
public:
    MyApp() = default;

    virtual bool OnStartup(std::vector<xpf::string_viewex>&& args) override {
        xpf::string_viewex title = "Hello OpenGL World!";
        RendererType renderOption = RendererType::OpenGL;
        if (args.size() == 1)
        {
            if (args[0].equals("metal", string_comparison::ordinal_ignoreCase))
            {
                renderOption = RendererType::Metal;
                title = "Hello Metal World!";
            }
            else if (args[0].equals("directx", string_comparison::ordinal_ignoreCase) ||
                     args[0].equals("dx", string_comparison::ordinal_ignoreCase))
            {
                renderOption = RendererType::DirectX;
                title = "Hello DirectX World!";
            }
            else if (args[0].equals("null", string_comparison::ordinal_ignoreCase))
            {
                renderOption = RendererType::Null;
                title = "Hello NULL World!";
            }
        }

        WindowOptions options;
        options.renderer = renderOption;
        options.title = title;
        options.width = 640;
        options.height = 480;
        options.is_projection_ortho2d = true;
        options.show_stats = 1;
        options.enable_vsync = false;
        options.sample_count = 0;
        options.background_color = xpf::Colors::XpfBlack;

        MyWindow window(std::move(options));
        window.Show();
        return true;
    }
};

int main(int argc, char* argv[])
{
   MyApp app;
   return app.Start(argc, argv);
}