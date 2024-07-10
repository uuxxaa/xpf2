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

#include <xpf/ui/UIElement.h>
#include <xpf/ui/TextBlock.h>
#include <xpf/ui/Button.h>
#include <xpf/ui/Border.h>
#include <xpf/ui/GridPanel.h>
#include <xpf/ui/ImagePanel.h>
#include <xpf/ui/Scrollbar.h>
#include <xpf/ui/SplitPanel.h>
#include <xpf/ui/StackPanel.h>
#include <xpf/ui/ThemeEngine.h>

using namespace xpf;

auto image_button(float x, float y, std::string imageSource, ButtonType btnType)
{
    typedef AttachedProperty<std::shared_ptr<Tween<xpf::Color>>, ConstexprHash("anitimer")> AniTimer;
    AniTimer aniTimer = std::make_shared<Tween<xpf::Color>>();

    auto spImg = std::make_shared<ImagePanel>();
    spImg->SetImageSource(imageSource)
        .SetWidth(60).SetHeight(60)
        .SetIsHitTestVisible(false);

    std::shared_ptr<Button> spBtn = std::make_shared<Button>();
    aniTimer.value->From(xpf::Colors::Transparent)
        .To(xpf::Colors::Yellow, 0.5f)
        .Yoyo(true).Repeat(2)
        .OnTimer([spImg](Tween<xpf::Color>& tw) {
            spImg->SetBackground(tw.Read());
            // spImg->SetBackground_Default(tw.Read());
            // spImg->SetBackground_Hover(tw.Read());
            // spImg->SetBackground_Pressed(tw.Read());
        });

    spBtn->SetContent(spImg).SetButtonType(btnType)
        .SetOnClick([](Button& btn)
        {
            btn.Get<AniTimer>()->get()->Play();
        })
        .SetX(x).SetY(y)
        .SetCornerRadius(8)
        .SetWidth(100).SetHeight(60)
        //..SetHorizontalAlignment(HorizontalAlignment::Stretch)
        .Set(std::move(aniTimer))
        ;

    if (btnType == ButtonType::Toggle)
    {
        auto spMedal = std::make_shared<ImagePanel>();
        spMedal->SetImageSource("alien.png")
            .SetWidth(60).SetHeight(60)
            .SetIsHitTestVisible(false);
        spBtn->SetContent_Pressed(spMedal);
    }
    return spBtn;
}

void test_gridpanel(IRenderer& renderer)
{
    auto init = []() {
        // auto spLeft = image_button(0, 0, "alien.png", ButtonType::Button);
        // spLeft->Set<GridPanel_>(100.0f);

        // auto spTop = image_button(0, 0, "medal.png", ButtonType::Button);
        // spTop->Set<Panel_MinLength>(60.0f);

        // auto spBottom = image_button(0, 0, "medal.png", ButtonType::Button);
        // spBottom->Set<Panel_MinLength>(60.0f);

        auto spPanel = std::make_shared<GridPanel>();
        spPanel->
            AddColumns({Panel_Auto, Panel_Pixel(50), Panel_Star(1), Panel_Star(2)})
            .AddRows({Panel_Pixel(50), Panel_Star(1), Panel_Auto})
            .SetX(10).SetY(30)
            .SetHeight(400).SetWidth(400)
            ;

        for (int r = 0; r < 3; r++)
        {
            for (int c = 0; c < 4; c++)
            {
                auto spCell = std::make_shared<TextBlock>();
                spCell->
                    SetFontSize(20).SetFontName("")
                    .SetName(std::to_string(r) +"," + std::to_string(c) + ((c==0 && r==0) ? "     hello world!" : ""))
                    .SetBackground(xpf::Colors::Random())
                ;
                spCell->Set<Grid_Row>(r);
                spCell->Set<Grid_Column>(c);
                if (c == 0 && r!=0)
                    spCell->SetWidth(160);
                if (r == 2)
                    spCell->SetHeight(120);

                spPanel->AddChild(spCell);
            }
        }

        return spPanel;
    };

    static auto spGrid = init();
    for (const auto& spChild : spGrid->GetChildren())
    {
        ((TextBlock*)spChild.get())->SetText(
            spChild->GetName() + "\n" + 
            xpf::stringex::to_string(spChild->GetActualRect().w, 0) + "," + 
            xpf::stringex::to_string(spChild->GetActualRect().h, 0));
    }

    spGrid->Draw(renderer);
}

void test_splitpanel(IRenderer& renderer)
{
    auto init = []() {
        auto spLeft = image_button(0, 0, "alien.png", ButtonType::Button);
        spLeft->Set<Panel_MinLength>(100.0f);

        auto spTop = image_button(0, 0, "medal.png", ButtonType::Button);
        spTop->Set<Panel_MinLength>(60.0f);

        auto spBottom = image_button(0, 0, "medal.png", ButtonType::Button);
        spBottom->Set<Panel_MinLength>(60.0f);

        auto spRight = std::make_shared<SplitPanel>();
        spRight->SetOrientation(xpf::Orientation::Vertical)
            .AddChild(spTop).AddChild(spBottom)
            .Set<Panel_MinLength>(100.0f);
            ;

        auto spobj = std::make_shared<SplitPanel>();
        spobj->SetOrientation(xpf::Orientation::Horizontal)
            .AddChild(spLeft).AddChild(spRight)
            .SetX(10).SetY(30)
            .SetHeight(300).SetWidth(500)
            ;

        return spobj;
    };

    static auto root = init();
    root->Draw(renderer);
}

void test_imgbutton(IRenderer& renderer)
{
    static auto spBtn = image_button(10, 40, "alien.png", ButtonType::Button);
    spBtn->Draw(renderer);

    static auto spBtn2 = image_button(180, 40, "medal.png", ButtonType::Toggle);
    spBtn2->Draw(renderer);
}

void test_image(IRenderer& renderer)
{
    auto init = []() {
        auto spobj = std::make_shared<ImagePanel>();
        spobj->SetImageSource("alien.png")
            .SetX(10).SetY(30)
            .SetBackground(xpf::Colors::Yellow)
            .SetBorder(xpf::Colors::BlueSuede).SetBorderThickness(5)
            .SetHeight(100).SetWidth(100)
            .SetIsHitTestVisible(false);
        return spobj;
    };

    static auto root = init();
    root->Draw(renderer);
}

void test_border(IRenderer& renderer)
{
    auto init = []() {
        auto spIn = std::make_shared<Border>();
        spIn->SetName("inside")
            .SetBackground(xpf::Colors::XpfGray)
            .SetMargin(5)
            .SetBorder(xpf::Colors::Red).SetBorderThickness(1)
            .SetVerticalAlignment(VerticalAlignment::Stretch)
            .SetHeight(50).SetWidth(200)
            .SetIsHitTestVisible(false);

        auto spobj = std::make_shared<Border>();
        spobj
            ->SetChild(spIn)
            .SetName("left")
            .SetX(10).SetY(40)
            .SetBackground(xpf::Colors::Yellow)
            .SetBorder(xpf::Colors::BlueSuede).SetBorderThickness(5)
            .SetHeight(100)
            .SetIsHitTestVisible(false);
        return spobj;
    };

    static auto root = init();
    root->Draw(renderer);
}

void test_border2(IRenderer& renderer)
{
    Thickness thick{
        15.0f + 10.0f * std::sinf(3.0f * Time::GetTime()),
        15.0f + 10.0f * std::sinf(3.0f * Time::GetTime()+xpf::math::PI_QUATER*.6),
        15.0f + 10.0f * std::sinf(3.0f * Time::GetTime()+xpf::math::PI_QUATER*1.2),
        15.0f + 10.0f * std::sinf(3.0f * Time::GetTime()+xpf::math::PI_QUATER*1.4)};

    CornerRadius cr{15.0f + 15.0f * std::sinf(Time::GetTime()+xpf::math::PI_QUATER*1.8)};

    RectangleDescription rect1{
        60, 160,
        HorizontalOrientation::Left,
        VerticalOrientation::Top,
        xpf::Colors::Purple, xpf::Colors::Red, thick, cr};

    RectangleDescription rect2{
        160, 60,
        HorizontalOrientation::Left,
        VerticalOrientation::Top,
        xpf::Colors::Yellow, xpf::Colors::Red, thick, cr};

    renderer.DrawRectangle(0,0,800,480, xpf::Colors::XpfWhite);
    renderer.DrawRectangle(10, 40, rect1);
    renderer.DrawRectangle(100, 40, rect2);

    rect1.cornerRadius = {};
    rect1.borderThickness = {};
    rect1.fillColor = xpf::Colors::LightBlue.with_alpha(128);
    rect1.width -= thick.left_right();
    rect1.height -= thick.top_bottom();

    rect2.cornerRadius = {};
    rect2.borderThickness = {};
    rect2.fillColor = xpf::Colors::LightBlue.with_alpha(128);
    rect2.width -= thick.left_right();
    rect2.height -= thick.top_bottom();

    renderer.DrawRectangle(10 + thick.l, 40 + thick.t, rect1);
    renderer.DrawRectangle(100 + thick.l, 40 + thick.t, rect2);
}

void test_stackpanel(IRenderer& renderer)
{
    auto init = []() {
        auto root = std::make_shared<xpf::StackPanel>();
        root->SetOrientation(xpf::Orientation::Vertical)
            .SetScrollOptions(ScrollOptions::FittedScroll)
            //.set_orientation(xpf::ui::orientation::vertical)
            .SetName("root")
            .SetX(10).SetY(50).SetPadding(5)
            .SetWidth(400).SetHeight(300)
            .SetBackground(xpf::Colors::White)
            .SetHorizontalAlignment(xpf::HorizontalAlignment::Center)
            ;

        // auto sptxt = std::make_shared<TextBlock>();
        // sptxt->SetText("The quick fox jumped over the lazy dog")
        //     .SetFontName("Arial").SetFontSize(40)
        //     .SetTextTrimming(TextTrimming::Ellipsis)
        //     .SetHorizontalAlignment(HorizontalAlignment::Stretch)
        //     // .SetWidth(230)
        //     ;
        // //root->AddChild(std::move(sptxt));

        auto spobj = std::make_shared<Border>();
        spobj//->SetChild(sptxt)
            ->SetName("left")
            .SetBackground(xpf::Colors::Honey)
            .SetBorder(xpf::Colors::BlueSuede).SetBorderThickness(5)
            .SetVerticalAlignment(VerticalAlignment::Stretch)
            .SetHorizontalAlignment(HorizontalAlignment::Stretch)
            // .SetWidth(200)
            // .SetHeight(200)
            .SetIsHitTestVisible(false)
            .Set<Panel_Length>(PanelLength(2, PanelLength::Star))
            ;

        root->AddChild(std::move(spobj));

        auto spobj2 = std::make_shared<Border>();
        spobj2->SetName("middle")
            .SetBackground(xpf::Colors::BlueSuede)
            .SetBorder(xpf::Colors::Honey).SetBorderThickness(5)
            //.SetMargin(5)
            .SetVerticalAlignment(VerticalAlignment::Stretch)
            .SetHorizontalAlignment(HorizontalAlignment::Stretch)
            //.SetWidth(500).SetHeight(390)
            // .SetHeight(400)
            .SetIsHitTestVisible(false)
            .Set<Panel_Length>(Panel_OneStar)
            ;
        root->AddChild(std::move(spobj2));

        auto spBtn = image_button(0,0, "alien.png", ButtonType::Button);
        root->AddChild(std::move(spBtn));

        // spobj = std::make_shared<ui::border2>();
        // spobj->set_name("right")
        //     .set_background_color(colors::Honey)
        //     .set_border_color(colors::BlueSuede).set_border_thickness(5)
        //     .set_margin(5)
        //     .set_width(1000)
        //     //.set_height(1000)
        //     .set_vertical_alignment(ui::vertical_alignment::stretch)
        //     //.set_horizontal_alignment(ui::horizontal_alignment::stretch)
        //     //.enable_debugging_ui(2)
        //     //.set<ui::stackpanel_width>(ui::panel_length(1, ui::panel_length::star))
        //     ;
        // root->add_child(std::move(spobj));

        return root;
    };

    static auto root = init();
    root->Draw(renderer);

    // auto init2 = []() {
    //     xpf::object obj;
    //     obj.set_border_color(xpf::colors::Blue);
    //     obj.set_border_thickness(1);
    //     obj.set_shape_type(xpf::render_shape::rectangle);
    //     obj.set_background_color(xpf::colors::Black);
    //     obj.translate(0, 1);
    //     return obj;
    // };

    // static xpf::object square = init2();
    // square.render();
}

void test_button(IRenderer& renderer)
{
    auto initTxtBlock = [](float x, float y)
    {
        TextBlock tb;
        tb.SetX(x).SetY(y);
        tb.SetFontName("").SetFontSize(20);
        return tb;
    };

    auto initScrollbar = [](float x, float y, Orientation orientation, TextBlock& tb) {
        Scrollbar scrl;
        scrl.SetOrientation(orientation)
            .SetX(x).SetY(y).SetHeight(orientation == Orientation::Vertical ? 200 : 20).SetWidth(orientation == Orientation::Vertical ? 20 : 200)
            .SetBackground(xpf::Colors::Teal)
            .SetForeground(xpf::Colors::Red)
            .SetPadding(5)
        ;

        scrl.GetValueProperty().SetOnChanged([&scrl, &tb](UIElement*, float)
        {
            tb.SetText(xpf::stringex::to_string(scrl.GetValue(), 1));
        });
        return scrl;
    };

    auto initImageButton = [](float x, float y, const std::string& imageSource) {
        static Tween<xpf::Color> s_timer;

        auto spImg = std::make_shared<ImagePanel>();
        spImg->SetImageSource(imageSource)
            //.SetBackground(xpf::Colors::Yellow)
            //.SetBorder(xpf::Colors::BlueSuede).SetBorderThickness(5)
            //.SetHeight(100).SetWidth(100)
            // .SetVerticalAlignment(VerticalAlignment::Stretch)
            // .SetHorizontalAlignment(HorizontalAlignment::Center)
            //.SetHeight(200)
            .SetWidth(60)
            .SetIsHitTestVisible(false);

        std::shared_ptr<Button> spBtn = std::make_shared<Button>();
        spBtn->SetContent(spImg)
            .SetOnClick([](Button& btn)
            {
                s_timer.Play();
            })
            .SetWidth(400).SetHeight(60).SetCornerRadius(8)
            ;

        s_timer.From(xpf::Colors::LimeGreen)
            .To(xpf::Colors::Yellow, 0.5f)
            .Yoyo(true).Repeat(2)
            .OnTimer([spBtn](Tween<xpf::Color>& tw) {
                spBtn->SetBackground(tw.Read());
                spBtn->SetBackground_Default(tw.Read());
                spBtn->SetBackground_Hover(tw.Read());
                spBtn->SetBackground_Pressed(tw.Read());
            });

        Border border;
        border.SetChild(std::move(spBtn))
            .SetX(x).SetY(y)
            .SetWidth(220).SetHeight(100)
            .SetBackground(xpf::Colors::XpfWhite)
            .SetCornerRadius(8).SetPadding(5)
        ;

        return border;
    };

    auto initButton = [](float x, float y, const std::string& text) {
        static Tween<float> s_timer;

        std::shared_ptr<Button> spBtn = std::make_shared<Button>();
        spBtn->SetText(text).SetFontName("Arial").SetFontSize(16)
            .SetOnClick([](Button& btn)
            {
                btn.SetText("World!");
                s_timer.Play();
            })
            .SetWidth(400).SetHeight(60).SetCornerRadius(8)
            ;

        s_timer
            .To(0, 3.0f)
            .OnTimer([](Tween<float>& tw){ tw.Read(); })
            .OnComplete([spBtn](bool)
            {
                spBtn->SetText("Hello");
            });

        Border border;
        border.SetChild(std::move(spBtn))
            .SetX(x).SetY(y)
            .SetWidth(220).SetHeight(100)
            .SetBackground(xpf::Colors::XpfWhite)
            .SetCornerRadius(8).SetPadding(5)
        ;

        return border;
    };

    // static auto btn = initButton(10, 40, "Hello!");
    // btn.Draw(renderer);

    static auto btnImg = initImageButton(240, 40, "alien.png");
    btnImg.Draw(renderer);

    // static auto tb = initTxtBlock(30, 200);
    // tb.Draw(renderer);
    // static auto scrlv = initScrollbar(210, 200, Orientation::Vertical, tb);
    // scrlv.Draw(renderer);
// 
    // static auto scrlh = initScrollbar(10, 400, Orientation::Horizontal, tb);
    // scrlh.Draw(renderer);
}

void test_textblock(IRenderer& renderer)
{
    auto initTextBlock = [](float x, float y, const std::string& text, TextAlignment horzAlignment, TextVerticalAlignment vertAlignment) {
        TextBlock tb;
        tb.SetX(x).SetY(y).SetWidth(140).SetHeight(60)
            .SetMargin(5)
            .SetBorderThickness(5)
            .SetCornerRadius(6)
            .SetPadding(0)
            .SetBackground(xpf::Color(0, 0xff)).SetBorder(xpf::Colors::Brown)
            .SetForeground(xpf::Colors::White);
        tb.SetText(text).SetFontName("Arial").SetFontSize(15)
            .SetTextAlignment(horzAlignment)
            .SetTextVerticalAlignment(vertAlignment)
            ;
        return tb;
    };

    float x = 10;
    for (const auto& ha : std::vector<std::pair<std::string, TextAlignment>>({
        {"left", TextAlignment::Left},
        {"center", TextAlignment::Center},
        {"right", TextAlignment::Right}
        }))
    {
        float y = 40;
        for (const auto& va : std::vector<std::pair<std::string, TextVerticalAlignment>>({
            {"top", TextVerticalAlignment::Top},
            {"center", TextVerticalAlignment::Center},
            {"baseline", TextVerticalAlignment::CenterBaseline},
            {"bottom", TextVerticalAlignment::Bottom}
            }))
        {
            TextBlock txtBlock = initTextBlock(10 + x, 80 + y, ha.first + " " + va.first, ha.second, va.second);
            txtBlock.Draw(renderer);
            y +=70;
        }
        x += 160;
    }
}

void test_glyph(IRenderer& renderer) {
    static Tween<float> m_animatePosition = 500.0f;
    auto init = [](IRenderer& renderer){
        m_animatePosition
            .To(-500, 3.0f, math::cubic_ease_inOut)
            .Yoyo(true)
            .Repeat()
            .Play();

        auto r = renderer.CreateCommandBuilder();
        r.DrawText(
            "S: 10 The quick fox jumped over the lazy dog.", 
            10, 200,
            "ld",
            200,
            xpf::Colors::XpfWhite);
        return r.Build();
    };

    static auto batchedCmds = init(renderer);
    // renderer.Transform(m4_t::translation_matrix(floor(m_animatePosition.Read()), 0));
    renderer.EnqueueCommands(batchedCmds);
}

void test_lines(IRenderer& r) {
    {
        float x = 10, y = 50;
        for (float w = 1; w < 8; w += 1) {
            float x0 = x + w * 20, y0 = y;
            r.DrawLine(x0, y0, x0 - 20, y0 + 100, w, xpf::Colors::Black);
        }

        x = 200;
        y = 200;
        float radius = 50;
        std::vector<PolyLineVertex> points;
        for (radians_t a = 0; a <= math::TAU+.001; a += math::PI_QUATER / 8) {
            xpf::Color color = xpf::math::lerp(xpf::Colors::Red, xpf::Colors::Blue, a / math::TAU);
            points.push_back({{x + cos(a) * radius, y + sin(a) * radius}, color, 5});
        }
        r.DrawLine(points);
    }

    xpf::Color lineColor = xpf::Colors::BlueSuede;
    float strokeWidth = 20;

    float y = 300.0f;
    float x = 20.0f;
    y +=50.f; r.DrawLine(x,y, x+150, y, 20, xpf::Colors::BlueSuede, LineOptions::CapButt);   r.DrawLine(x,y, x+150, y, 1.f, xpf::Colors::Red);
    y +=50.f; r.DrawLine(x,y, x+150, y, 20, xpf::Colors::BlueSuede, LineOptions::CapRound);  r.DrawLine(x,y, x+150, y, 1.f, xpf::Colors::Red);
    y +=50.f; r.DrawLine(x,y, x+150, y, 20, xpf::Colors::BlueSuede, LineOptions::CapSquare); r.DrawLine(x,y, x+150, y, 1.f, xpf::Colors::Red);
    xpf::Color red(20,200,128);
    xpf::Color blue(15,20,230,128);
    xpf::Color green(15,220,20,128);

    float off = -60.0f;
    off += 30; r.DrawLine({
        {{-off + 100,  off +  50}, red, 20},
        {{-off + 250, off +  50}, blue, 20},
        {{-off + 250, off + 200}, green, 20}},
        LineOptions::JointBevel | LineOptions::CapRound);
    off += 30; r.DrawLine({
        {{-off + 100,  off +  50}, red, 20},
        {{-off + 250, off +  50}, blue, 20},
        {{-off + 250, off + 200}, green, 20}},
        LineOptions::JointRound | LineOptions::CapRound);
    off += 30; r.DrawLine({
        {{-off + 100,  off +  50}, red, 20},
        {{-off + 250, off +  50}, blue, 20},
        {{-off + 250, off + 200}, green, 20}},
        LineOptions::JointMiter | LineOptions::CapRound);
    off += 30; r.DrawLine({
        {{-off + 100,  off +  50}, red, 20},
        {{-off + 250, off +  50}, blue, 20},
        {{-off + 250, off + 200}, green, 20}},
        LineOptions::JointRound | LineOptions::Closed);

    //r.DrawSimpleLine({{100, 300}, {200, 400}, {300, 420}, {300, 350}});
    r.DrawLine({{100, 300}, {200, 400}, {300, 420}, {250, 420}});

    v2_t p1(300, 400);
    v2_t p2(280, 200);
    v2_t p3(500, 220);
    v2_t p4(600, 380);

    r.DrawBezierQuadratic({p1, Colors::Yellow, 10}, {p2, math::midpoint(Colors::Red, Colors::Blue), 2}, {p3, Colors::Blue, 10}, LineOptions::CapRound);
    r.DrawBezierCubic({p1, Colors::Red.with_alpha(40), 10}, {p2, Colors::Red, 2}, {p3, Colors::Blue, 100}, {p4, Colors::Blue, 10}, LineOptions::CapRound);

    r.DrawBezierCubic(
        {{10,100}, Colors::Honey, 10},
        {{100,10}, Colors::Honey, 10},
        {{150,150}, Colors::Honey, 10},
        {{200,100}, Colors::Honey, 10}, LineOptions::CapRound);

    r.DrawBezierCubic(
        {{10,50}, Colors::Yellow, 5},
        {{30,500}, Colors::Yellow, 5},
        {{50,100}, Colors::Yellow, 5},
        {{70,200}, Colors::Yellow, 5});

    float width = 640;
    float height = 480;
    float lowerMargin = 0.18;
    float upperMargin = 1.0 - lowerMargin;
    float xDelta = (upperMargin - lowerMargin) / 8;

    float px = lowerMargin * width;
    float py = lowerMargin * height;
//     r.SetStrokeWidth(5.0);
//     r.SetStroke(Colors::Honey);
//     r.DrawSpline({
//         {px,py},
//         {px + 1 * xDelta * width, upperMargin * height},
//         {px + 2 * xDelta * width, lowerMargin * height},
//         {px + 3 * xDelta * width, upperMargin * height},
//         {px + 4 * xDelta * width, lowerMargin * height},
//         {px + 5 * xDelta * width, upperMargin * height},
//         {px + 6 * xDelta * width, lowerMargin * height},
//         {px + 7 * xDelta * width, upperMargin * height},
//         {px + 8 * xDelta * width, lowerMargin * height},
//         });

//    r.SetStrokeWidth(1.0);
//    r.SetStroke(Colors::Red);
//    r.DrawLine({p1, p2, p3, p4});
}

void test_empty(IRenderer& r) {
}

void test_screen(IRenderer& renderer) {
    static std::shared_ptr<ITexture> m_spTexture1;
    static std::shared_ptr<ITexture> m_spTexture2;
    static Tween<float> m_animatePosition = 10.0f;

    auto init = [](IRenderer& renderer){

        m_spTexture1 = renderer.CreateTexture("alien.png");
        m_spTexture2 = renderer.CreateTexture("medal.png");
        m_animatePosition
            .To(150, 2.0f, math::cubic_ease_inOut)
            .Yoyo(true)
            .Repeat(5)
            .Play();

        auto r = renderer.CreateCommandBuilder();
        r.DrawRectangle(100, 220, {
                    100, 60,
                    HorizontalOrientation::Left,
                    VerticalOrientation::Top,
                    /*fillColor:*/ xpf::Colors::Honey,
                    /*borderColor:*/ xpf::Colors::Violet,
                    /*borderThickness:*/{1, 5, 10, 20}, /*cornerRadius*/{20}});
        r.DrawRectangle(100, 300, {
                    60, 100,
                    HorizontalOrientation::Left,
                    VerticalOrientation::Top,
                    /*fillColor:*/ xpf::Colors::Honey,
                    /*borderColor:*/ xpf::Colors::Violet,
                    /*borderThickness:*/{0}, /*cornerRadius*/{10},
                    m_spTexture1});
        r.DrawCircle(300, 50, 30, xpf::Colors::Brown);
        r.DrawCircle(360, 50,{
            30, 5,
            HorizontalOrientation::Left,
            VerticalOrientation::Top,
            /*fillColor:*/ xpf::Colors::Honey,
            /*borderColor:*/ xpf::Colors::Violet,
            /*spTexture*/ nullptr});
        r.DrawImage(200, 20, 128, 128, m_spTexture2);
        r.DrawImage(20, 20, 128, 128, m_spTexture1);
        r.DrawRectangle(10, 10, 64, 64, xpf::Colors::Honey);
        r.DrawRectangle(158, 158, 50, 50, xpf::Colors::Green);
        r.RunAction([&renderer](){
            auto r2 = renderer.CreateCommandBuilder();
            r2.DrawText("Can you read me?", 0, 0, "", 10, xpf::Colors::Random());
            return r2.Build();
        });
        r.DrawText("Hello World!!!!", 200, 200, "", 20, xpf::Colors::XpfRed);

        for (float w = 1; w < 16; w += 1) {
            float x0 = 200 + w * 20, y0 = 300;
            r.DrawLine(x0, y0, x0 - 20, y0 + 100, w, xpf::Colors::Black);
        }
        return r.Build();
    };
    static auto batchedCmds = init(renderer);

    auto scope = renderer.Transform(m4_t::translation_matrix(floor(m_animatePosition.Read()), 0));
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
            m_next_testid_switch = Time::GetTime() + 2;
            m_testid++;
        }

        switch (m_testid) {
            default: m_testid = 0; [[fallthrough]];
            TEST(test_glyph)
            TEST(test_border2)
            TEST(test_button)
            TEST(test_stackpanel)
            TEST(test_gridpanel)
            TEST(test_splitpanel)
            TEST(test_imgbutton)
            TEST(test_image)
            TEST(test_textblock)
            TEST(test_lines);
            TEST(test_screen);
            // TEST(test_empty);
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

        ThemeEngine::Initialize();

        WindowOptions options;
        options.renderer = renderOption;
        options.title = title;
        options.width = 800;
        options.height = 480;
        options.is_projection_ortho2d = true;
        options.show_stats = 1;
        //options.metal_transactional_rendering = false;
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