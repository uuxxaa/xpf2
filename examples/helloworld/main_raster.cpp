#include <core/stringex.h>
#include <core/Image.h>
#include <core/Log.h>
#include <core/Tween.h>
#include <renderer/ITexture.h>
#include <renderer/common/RenderBatchBuilder.h>
#include <windows/Application.h>
#include <windows/InputService.h>
#include <windows/Window.h>
#include <iostream>

#include <renderer/common/Font.h>
#include <core/FileSystem.h>
#include <stb/stb_truetype.h>

using namespace xpf;

// Calculate roots for f(x) = a^2 x + b x + c
static std::pair<float, float> CalculateQuadraticRoots(float a, float b, float c) {
    float rootA = std::numeric_limits<float>::quiet_NaN();
    float rootB = std::numeric_limits<float>::quiet_NaN();

    if (std::abs(a) < xpf::math::Epsilon) {
        // curve is a straight line when a is 0
        if (b != 0) rootA = -c / b; // 
    } else {
        // if discrimant is negative there are no real roots
        float discrimant = b * b - 4 * a * c;
        if (discrimant >= 0) {
            float s = std::sqrtf(discrimant);
            rootA = (-b + s) / (2 * a);
            rootB = (b - s) / (2 * a);
        }
    }

    return {rootA, rootB};
}

template<typename TR>
static bool DrawPointOnBezierCurve(TR& r, float t, v2_t p0, v2_t p1, v2_t p2, float rayY)
{
    bool onBezierCurve = (-xpf::math::Epsilon < t && t < (1.0 + xpf::math::Epsilon));
    if (!onBezierCurve)
        return false;

    float yy = xpf::math::lerp_quadratic(p0.y, p1.y, p2.y, t);
    if (std::abs(yy - rayY) > 0.1)
        return false;

    xpf::Color color = xpf::Colors::DodgerBlue;
    v2_t px = math::lerp_quadratic(p0, p1, p2, {t, t*t, t*t*t});

    r.DrawCircle(px.x, px.y, 6, color);
    return true;
}

static bool HorizontalRayIntersectionTest(
    RenderBatchBuilder& r,
    v2_t p0, v2_t p1, v2_t p2, float rayY) {

    float a = p0.y - 2 * p1.y + p2.y;
    float b = 2 * (p1.y - p0.y);
    float c = p0.y;

    auto [t0, t1] = CalculateQuadraticRoots(a, b, c - rayY);

    if (!std::isnan(t0)) return DrawPointOnBezierCurve(r, t0, p0, p1, p2, rayY);
    if (!std::isnan(t1)) return DrawPointOnBezierCurve(r, t1, p0, p1, p2, rayY);
    return false;
}

// for non-curved lines
static void HorizontalRayIntersectionTest(
    RenderBatchBuilder& r,
    v2_t p0, v2_t p1, float rayY) {

    float t = xpf::math::inverse_lerp(p0.y, p1.y, rayY);
    if (-xpf::math::Epsilon < t && t < (1.0f + xpf::math::Epsilon))
        r.DrawCircle(xpf::math::lerp(p0.x, p1.x, t), rayY, 6, xpf::Colors::Wheat);
}

void bezier_test(IRenderer& r) {
    v2_t p0(305, 81);
    v2_t p1(368, 81);
    v2_t p2(404, 114);

    v2_t m = InputService::GetMousePosition();
    float rayY = m.y;
    r.DrawLine(0,rayY,1000,rayY, 1, xpf::Colors::Black);

    r.DrawBezierQuadratic(p0, p1, p2, 4, xpf::Colors::Gray);

    float a = p0.y - 2 * p1.y + p2.y;
    float b = 2 * (p1.y - p0.y);
    float c = p0.y;

    auto [t0, t1] = CalculateQuadraticRoots(a, b, c);
    DrawPointOnBezierCurve(r, t0, p0, p1, p2, rayY);
    DrawPointOnBezierCurve(r, t1, p0, p1, p2, rayY);

    r.DrawText(
        xpf::stringex::to_string(m.x, 3) + ", " +
        xpf::stringex::to_string(m.y, 3),
        10, 50, "", 20,
        xpf::Colors::LightSalmon);
}

static bool IsValidIntersection(float t, v2_t rayPt, v2_t p0, v2_t p1, v2_t p2) {
    bool onBezierCurve = (t >= 0 && t < 1.0);
    if (!onBezierCurve)
        return false;

//    float yy = xpf::math::lerp_quadratic(p0.y, p1.y, p2.y, t);
    v2_t px = math::lerp_quadratic(p0, p1, p2, {t, t*t, t*t*t});

    if (std::abs(px.y - rayPt.y) > 0.01 || xpf::math::is_less_than(rayPt.x, px.x))
        return false;

    return true;
}

static int32_t CountHorizontalIntersections(v2_t rayOrigin, v2_t p0, v2_t p1, v2_t p2) {
    if (std::abs(p1.y - p2.y) <.1 && std::abs(p0.y - p2.y) < .1)
        return 0;

    float a = p0.y - 2 * p1.y + p2.y;
    float b = 2 * (p1.y - p0.y);
    float c = p0.y;

    auto [t0, t1] = CalculateQuadraticRoots(a, b, c - rayOrigin.y);

    int32_t num = 0;
    if (!std::isnan(t0) && IsValidIntersection(t0, rayOrigin, p0, p1, p2)) num++;
    if (!std::isnan(t1) && IsValidIntersection(t1, rayOrigin, p0, p1, p2)) num++;
    return num;
}

static int32_t CountHorizontalIntersections(v2_t rayOrigin, v2_t p0, v2_t p1) {
    float t = xpf::math::inverse_lerp(p0.y, p1.y, rayOrigin.y);
    if (-xpf::math::Epsilon < t && t < (1.0f + xpf::math::Epsilon))
            return 1;
    t = xpf::math::inverse_lerp(p1.y, p0.y, rayOrigin.y);
    if (-xpf::math::Epsilon < t && t < (1.0f + xpf::math::Epsilon))
        return 1;
    return 0;
}

bool IsPointInsizeGlyph(v2_t point, stbtt_vertex* pvertices, int count) {
    int numIntersections = 0;
    int16_t x = 0, y = 0;

    for (int i = 0; i < count; i++) {
        const stbtt_vertex& vrtx = pvertices[i];
        if (vrtx.type == STBTT_vmove)
        {
        }
        else if (vrtx.type == STBTT_vline)
        {
            v2_t p0(x,y);
            v2_t p1(vrtx.x, vrtx.y);
            if (y != vrtx.y)
            {
                numIntersections += CountHorizontalIntersections(
                    point,
                    p0,
                    v2_t::mid_point(p0, p1),
                    p1);
            }
        }
        else if (vrtx.type == STBTT_vcurve)
        {
            v2_t p0(x,y);
            v2_t p1(vrtx.cx, vrtx.cy);
            v2_t p2(vrtx.x, vrtx.y);
            int count0 = CountHorizontalIntersections(
                point,
                p0,
                p1,
                p2);

            int count1 = CountHorizontalIntersections(
                point,
                p2,
                p1,
                p0);

            if (count0 > count1)
                numIntersections += count0;
            else
                numIntersections += count1;
        }

        x = vrtx.x;
        y = vrtx.y;
    }
    
    bool isOdd = (numIntersections & 1) != 0;
    if (isOdd)
        return true;
    return false;

//    return (numIntersections & 1) != 0; // odd number of intersections (inside)
}

v2_t mouse_pos() {
    return {20, 50};
    // return InputService::GetMousePosition();
}

void test_font(IRenderer& renderer) {
    bezier_test(renderer);
    
    float lineThickness = 0;
    xpf::Color lineColor = xpf::Colors::XpfWhite;

    auto init = [&lineThickness, &lineColor](IRenderer& renderer){
        auto r = renderer.CreateCommandBuilder();

        r.RunAction([&renderer]()
        {
            auto b = renderer.CreateCommandBuilder();
            v2_t m = InputService::GetMousePosition();
            b.DrawLine(0, m.y, 1000, m.y, 1, xpf::Colors::Black);
            return b.Build();
        });

        // std::string path = "ld.otf";
        std::string path = "/Users/umutalev/Library/Fonts/JetBrainsMono-Regular.ttf";
        // std::string path = "/System/Library/Fonts/Menlo.ttc";
        // std::string path = "/System/Library/Fonts/Supplemental/Arial.ttf";
        auto data = FileSystem::LoadFile(path);

        stbtt_fontinfo fontInfo;
        stbtt_InitFont(&fontInfo, data.data(), 0);
        std::string text = "B"; // 10 The quick fox jumped over the lazy dog.";
        size_t length = text.length();
        float xorg = 20;
        float yorg = 400;

        float scale = 1.0f/20;
        auto px = [&](int16_t x) {
            return xorg + float(x) * scale;
        };
        auto py = [&](int16_t y) {
            return yorg - float(y) * scale;
        };

        int ascent, descent, lineGap;
        stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);
        int fontHeight = ascent - descent;

        char prevch = 0;
        size_t ix = 0;
        while (ix < length) {
            const uint32_t ch = stringex::utf8_to_utf32(text, ix);
            if (ch == 0) [[unlikely]]
                break;

            stbtt_vertex *pvertices = nullptr;
            int count = stbtt_GetCodepointShape(&fontInfo, int32_t(ch), &pvertices);
            if (count <= 0)
                continue;

            int advanceWidth, leftSideBearing;
            stbtt_GetCodepointHMetrics(&fontInfo, int32_t(ch), &advanceWidth, &leftSideBearing);

            for (float yi = 0; yi < fontHeight; yi+= fontHeight/100) {
                for (float xi = 0; xi < advanceWidth; xi+=advanceWidth/100) {
                    v2_t point(xi, yi);
                    if (IsPointInsizeGlyph(point, pvertices, count))
                        r.DrawCircle(floor(px(xi)), floor(py(yi)), 1, xpf::Colors::LimeGreen);
                }
            }
            prevch = ch;
            int16_t x = 0, y = 0; bool hole = false;
            for (int i = 0; i < count; i++) {
                const stbtt_vertex& vrtx = pvertices[i];

                bool ccw = false;
                if (vrtx.type == STBTT_vmove)
                {
                    printf("m: %d, %d\n", vrtx.x, vrtx.y);
                }
                else if (vrtx.type == STBTT_vline)
                {
                    printf("l: %d, %d ", x, y);
                    printf("-  %d, %d\n", vrtx.x, vrtx.y);

                    v2_t p0(px(x),py(y));
                    v2_t p1(px(vrtx.x), py(vrtx.y));
                    r.DrawLine(
                        p0.x, p0.y,
                        p1.x, p1.y,
                        lineThickness, xpf::Colors::White,
                        LineOptions::CapRound);

                    r.RunAction([p0, p1, &renderer]()
                    {
                        auto b = renderer.CreateCommandBuilder();
                        HorizontalRayIntersectionTest(b, p0, p1, mouse_pos().y);
                        return b.Build();
                    });
                }
                else if (vrtx.type == STBTT_vcurve)
                {
                    printf("q: %d, %d ", x, y);
                    printf("-  %d, %d ", vrtx.cx, vrtx.cy);
                    printf("-  %d, %d\n", vrtx.x, vrtx.y);

                    v2_t p0(px(x),py(y));
                    v2_t p1(px(vrtx.cx), py(vrtx.cy));
                    v2_t p2(px(vrtx.x), py(vrtx.y));

                    if (i != 1000)
                    {
                        r.DrawBezierQuadratic(
                            {p0.x, p0.y},
                            {p1.x, p1.y},
                            {p2.x, p2.y},
                            lineThickness, xpf::Colors::Random(),
                            LineOptions::CapRound);

                        r.RunAction([p0, p1, p2, &renderer]()
                        {
                            auto b = renderer.CreateCommandBuilder();
                            if (!HorizontalRayIntersectionTest(b, p0, p1, p2, mouse_pos().y))
                                HorizontalRayIntersectionTest(b, p2, p1, p0, mouse_pos().y);
                            return b.Build();
                        });
                    }
                }
                else if (vrtx.type == STBTT_vcubic)
                {
                    printf("c: %d, %d ", x, y);
                    printf("-  %d, %d ", vrtx.cx, vrtx.cy);
                    printf("-  %d, %d ", vrtx.cx1, vrtx.cy1);
                    printf("-  %d, %d\n", vrtx.x, vrtx.y);

                    r.DrawBezierCubic(
                        {px(x), py(y)},
                        {px(vrtx.cx), py(vrtx.cy)},
                        {px(vrtx.cx1), py(vrtx.cy1)},
                        {px(vrtx.x), py(vrtx.y)},
                        lineThickness, lineColor,
                        LineOptions::CapRound);
                }
                else
                {
                    throw std::exception();
                }

                x = vrtx.x; y = vrtx.y;
            }

            free(pvertices);
            xorg += advanceWidth * scale;
        }
        return r.Build();
    };

    static auto batchedCmds = init(renderer);
    renderer.EnqueueCommands(batchedCmds);
}

/////////////
/////////////
/////////////
/////////////
/////////////
/////////////
/////////////
/////////////

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