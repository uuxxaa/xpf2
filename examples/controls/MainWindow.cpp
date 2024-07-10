#include "MainWindow.h"
#include <xpf/core/Image.h>
#include <xpf/ui/Panel.h>
#include <xpf/ui/SplitPanel.h>
#include <xpf/ui/Border.h>
#include <xpf/ui/Button.h>
#include <xpf/ui/TextBlock.h>
#include <xpf/ui/ThemeEngine.h>
#include <xpf/renderer/common/Font.h>

#include "test_textblock.h"
#include "test_textbox.h"
#include "test_button.h"
#include "test_gridpanel.h"
#include "test_canvas.h"
#include "test_treepanel.h"
#include "test_imagepanel.h"
#include "test_switchbox.h"

using namespace xpf;

bool MainWindow::OnSetup(IRenderer& r)
{
    //                                                light theme               dark theme
    ThemeEngine::AddThemeColor("TestName_Background", xpf::Colors::Transparent, xpf::Colors::Transparent);
    ThemeEngine::AddThemeColor("TestName_Background_Hover", xpf::Colors::Honey, xpf::Colors::BlueSuede);
    ThemeEngine::AddThemeColor("TestName_Foreground", xpf::Colors::XpfBlack, xpf::Colors::XpfWhite);
    ThemeEngine::AddThemeColor("SubTitle_Foreground", xpf::Colors::Gray, xpf::Colors::WhiteSmoke);
    ThemeEngine::AddThemeColor("SubPanel_Background", xpf::Colors::LightGray, xpf::Colors::Gray);
    ThemeEngine::AddThemeColor("TestTextBlock_Foreground", xpf::Colors::Black, xpf::Colors::White);
    ThemeEngine::AddThemeColor("TestTextBlock_Background", xpf::Colors::White, xpf::Colors::Black);

    ThemeEngine::AddThemeFont("TreeNode_Font", {"Arial", 12}, {"Menlo", 12});

    m_root
        .SetX(0).SetY(0)
        .SetWidth(m_options.width)
        .SetHeight(m_options.height)
        .SetPadding(5)
        //.SetPadding(25)
        .GetBackgroundProperty().SetThemeId("Window_Background")
        ;
#if 0
    m_root.
        AddColumns({Panel_Auto, Panel_Pixel(50), Panel_Star(1), PanelLength(2, PanelLength::Star, 100)})
        .AddRows({Panel_Pixel(50), PanelLength(1, PanelLength::Star, 50), Panel_Auto})
        ;

    for (int r = 0; r < 3; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            auto spText = std::make_shared<TextBlock>(); // UIElementType::UIElement);
            spText->
                SetFontSize(20).SetFontName("")
                .
                SetName(std::to_string(r) +"," + std::to_string(c) + ((c==0 && r==0) ? "     hello world!" : ""))
            ;
            spText->Set<Grid_Row>(r);
            spText->Set<Grid_Column>(c);
            spText->SetBorderThickness(10).SetBorder(xpf::Colors::Random());
            spText->SetBackground(xpf::Colors::Random().with_alpha(128));
            // if (c == 0 && r!=0)
            //     spText->SetWidth(160);
            if (r == 1 && c == 1)
                spText->SetHeight(120).SetVerticalAlignment(VerticalAlignment::Center);

            m_root.AddChild(spText);
        }
    }

    m_root.SetOnResize([](UIElement& elem)
    {
        for (const auto& spChild : ((GridPanel*)&elem)->GetChildren())
        {
           ((TextBlock*)spChild.get())->SetText(
               spChild->GetName() + "\n" + 
               xpf::stringex::to_string(spChild->GetActualRect().width, 0) + "," + 
               xpf::stringex::to_string(spChild->GetActualRect().height, 0));
           // spChild->InvalidateLayout();
        }
        //elem.InvalidateLayout();
        return true;
    });

    return true;
#endif
    std::vector<PanelLength> rows;
    rows.push_back(Panel_Pixel(36));
    rows.push_back(Panel_OneStar);
    m_root.AddRows(std::move(rows));

    m_spToolbarPanel = CreateToolBar(r);
    m_spToolbarPanel->Set<Grid_Row>(0);
    m_root.AddChild(m_spToolbarPanel);

    std::shared_ptr<SplitPanel> spSplit = std::make_shared<SplitPanel>();
    spSplit->Set<Grid_Row>(1);
    spSplit->SetOrientation(Orientation::Horizontal);
    m_root.AddChild(spSplit);

    {
        m_spControlsPanel = CreateListOfControls();

        auto spBorder = std::make_shared<Border>();
        spBorder->SetCornerRadius(10)
            .SetBorderThickness(1)
            .SetPadding(4)
            .SetName("Left");
        spBorder->Set<Panel_Length>(Panel_OneStar);
        spBorder->Set<Panel_MinLength>(100);
        spBorder->SetChild(m_spControlsPanel);
        spBorder->GetBackgroundProperty().SetThemeId("Panel_Level1_Background");
        spBorder->GetBorderProperty().SetThemeId("Panel_Level1_Border");

        spSplit->AddChild(spBorder);
    }

    {
        auto spBorder = std::make_shared<Border>();
        spBorder->SetCornerRadius(10)
            .SetBorderThickness(1)
            .SetPadding(5)
            .SetName("Right");
        spBorder->Set<Panel_Length>(Panel_ThreeStars);
        spBorder->Set<Panel_MinLength>(100);
        spBorder->SetChild(m_spDemoPanel);
        spBorder->GetBackgroundProperty().SetThemeId("Panel_Level1_Background");
        spBorder->GetBorderProperty().SetThemeId("Panel_Level1_Border");

        m_spDemoPanel = spBorder;

        spSplit->AddChild(spBorder);
    }

    LoadControls(r);
    return true;
}

std::shared_ptr<UIElement> MainWindow::CreateToolBar(IRenderer& r)
{
    auto spToolbar = std::make_shared<StackPanel>();
    spToolbar->SetOrientation(Orientation::Horizontal);
    spToolbar->SetVerticalAlignment(VerticalAlignment::Center);
    spToolbar->SetHorizontalAlignment(HorizontalAlignment::Right);
//    spToolbar->SetBackground(xpf::Colors::Honey);

    spToolbar->AddChild(CreateResetButton(r));
    spToolbar->AddChild(CreateScreenCaptureButton());
    spToolbar->AddChild(CreateThemeSwitchButton());
    return spToolbar;
}

std::shared_ptr<Button> MainWindow::CreateScreenCaptureButton()
{
    auto spBtn = std::make_shared<Button>();
    spBtn->
        SetFontName("segoeicons").SetFontSizeIsInPixels(true).SetFontSize(16).SetText("\ue722")
        .SetOnClick([this](Button& btn) {
            m_captureScreen = true;
        })
        .SetVerticalAlignment(VerticalAlignment::Center)
        .SetWidth(26)
        .SetHeight(26)
        .SetCornerRadius(4)
        .SetMargin({4,0,0,0})
        ;

    return spBtn;
}

std::shared_ptr<Button> MainWindow::CreateResetButton(IRenderer& r)
{
    auto spBtn = std::make_shared<Button>();
    spBtn->
        SetFontName("Arial").SetFontSizeIsInPixels(true).SetFontSize(16).SetText("Reset")
        .SetOnClick([this, &r](Button& btn) {
            LoadControls(r);
        })
        .SetVerticalAlignment(VerticalAlignment::Center)
        .SetPadding(4)
        .SetHeight(26)
        .SetCornerRadius(4)
        .SetMargin({24,0,0,0})
        ;

    return spBtn;
}

std::shared_ptr<Button> MainWindow::CreateThemeSwitchButton()
{
    static ThemeId id = ThemeEngine::GetCurrentThemeId();
    auto iconPicker = []()
    {
        if (ThemeEngine::GetCurrentThemeId() == ThemeId::Light)
            return ""; // sun
        else
            return ""; // moon
    };
    static bool switching = false;
    static bool theme_loaded = false;

    static Tween<radians_t> tw;
    tw.To(math::PI * 2, .5f, math::cubic_ease_inOut).Repeat(1);

    auto spBtn = std::make_shared<Button>();
    spBtn->
        SetFontName("segoeicons").SetFontSizeIsInPixels(true).SetFontSize(16).SetText(iconPicker()) // sun
        .SetOnClick([](Button& btn) {
            tw.Play();
            switching = true;
            theme_loaded = false;
        })
        .SetVerticalAlignment(VerticalAlignment::Center)
        .SetPadding(4)
        .SetWidth(26)
        .SetHeight(26)
        .SetCornerRadius(CornerRadius::AutoRadius())
        .SetMargin({14,0,0,0})
        .SetOnDraw([this, &iconPicker](UIElement& elem, IRenderer&) mutable
        {
            if (switching)
            {
                if (tw.Read() > math::PI)
                {
                    if (!tw.IsPlaying())
                    {
                        ((Button&)(elem)).GetContent()->GetRotationAngleProperty().Unset();
                        switching = false;
                        return true;
                    }

                    if (!theme_loaded) {
                        theme_loaded = true;
                        ThemeEngine::ToggleTheme();
                        m_root.InvalidateVisuals();
                        ((Button&)(elem)).SetText(iconPicker());
                    }
                }

                ((Button&)(elem)).GetContent()->SetRotationAngle(tw.Read());
            }
            return true;
        })
        ;

    ((TextBlock*)(spBtn->GetContent().get()))->SetTextTrimming(TextTrimming::None);

    return spBtn;
}

typedef AttachedProperty<std::shared_ptr<UIElement>, ConstexprHash("test_panel")> TestPanelProperty;

std::shared_ptr<UIElement> MainWindow::GetTestListItem(const std::shared_ptr<UIElement>& spTest)
{
    auto spBorder = std::make_shared<Border>();
    spBorder->SetPadding({5,2,5,2})
        .SetHorizontalAlignment(HorizontalAlignment::Stretch)
        .SetVerticalAlignment(VerticalAlignment::Top)
        .SetOnMouse([this](UIElement& elem, bool isInside)
        {
            elem.SetBackground(ThemeEngine::GetColor(isInside ? "TestName_Background_Hover" : "TestName_Background"));
            if (isInside && InputService::IsMouseButtonPressed(MouseButton::Left))
            {
                m_spDemoPanel->SetChild(*elem.Get<TestPanelProperty>());
                m_spDemoPanel->Focus();
            }
        })
        .SetHeight(30)
        .Set<TestPanelProperty>(spTest)
        ;

    auto spText = std::make_shared<TextBlock>();
    spText->SetIsHitTestVisible(false);
    spText->SetText(spTest->GetName())
        .SetFontName("")
        .SetFontSize(20)
        .SetTextAlignment(TextAlignment::Left)
        .SetVerticalAlignment(VerticalAlignment::Center)
        .GetForegroundProperty().SetThemeId("TestName_Foreground");
    spBorder->SetChild(spText);

    return spBorder;
}

std::shared_ptr<StackPanel> MainWindow::CreateListOfControls()
{
    auto spPanel = std::make_shared<StackPanel>();
    spPanel->SetName("ControlList");
    spPanel->SetOrientation(Orientation::Vertical);
    spPanel->SetScrollOptions(ScrollOptions::ScrollOvershoot);
    spPanel->SetBackground(xpf::Colors::LightBlue);
    return spPanel;
}

void MainWindow::LoadControls(IRenderer& r)
{
    auto spPanel = m_spControlsPanel;
    spPanel->RemoveAll();
    spPanel->AddChild(GetTestListItem(TextBlock_Test::SimpleTextBlock(r)));
    spPanel->AddChild(GetTestListItem(SwitchBox_Test::SimpleSwitchBox(r)));
    spPanel->AddChild(GetTestListItem(TextBox_Test::SimpleTextBox(r)));

    spPanel->AddChild(GetTestListItem(ImagePanel_Test::ImageStretchTest(r)));

    spPanel->AddChild(GetTestListItem(DrawText_Test::ShaderRenderedText()));
    spPanel->AddChild(GetTestListItem(DrawText_Test::RoundedRectangle()));

    spPanel->AddChild(GetTestListItem(TextBlock_Test::VerticalAlignment_Height()));
    spPanel->AddChild(GetTestListItem(TextBlock_Test::VerticalAlignment_Height(60)));
    spPanel->AddChild(GetTestListItem(TextBlock_Test::TextAlignmentTest()));

    spPanel->AddChild(GetTestListItem(Button_Test::VerticalAlignment_Height()));
    spPanel->AddChild(GetTestListItem(Button_Test::VerticalAlignment_Height(30)));
    spPanel->AddChild(GetTestListItem(Button_Test::Rectangles()));

    spPanel->AddChild(GetTestListItem(GridPanel_Test::SimpleGrid()));
    spPanel->AddChild(GetTestListItem(TreePanel_Test::TreeControl(r)));
    spPanel->AddChild(GetTestListItem(DrawText_Test::ShaderRenderedText2()));
    spPanel->AddChild(GetTestListItem(DrawText_Test::ShaderRenderedWallOfText(r)));
    spPanel->AddChild(GetTestListItem(DrawText_Test::DrawSvg(r)));

    m_spDemoPanel->SetChild(nullptr);
}

void MainWindow::OnDraw(IRenderer& r)
{
    if (m_captureScreen)
    {
        r.CaptureScreen(1, [&](Image&& img) { OnScreenCapture(r, std::move(img)); });
        m_captureScreen = false;
    }

    m_root.Draw(r);
}

void MainWindow::OnResize(int32_t width, int32_t height)
{
    m_root.SetWidth(width).SetHeight(height);
}

void MainWindow::OnScreenCapture(IRenderer& r, Image&& image)
{
    image.SaveImage("abc.png");
}