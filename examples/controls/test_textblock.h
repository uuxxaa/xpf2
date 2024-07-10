#pragma once
#include <xpf/ui/GridPanel.h>
#include <xpf/ui/TextBlock.h>

namespace xpf {

struct TextBlock_Test {

static auto CreateTextBlock()
{
    auto spTxt = std::make_shared<TextBlock>();
    spTxt->GetForegroundProperty().SetThemeId("TestTextBlock_Foreground");
    spTxt->GetBackgroundProperty().SetThemeId("TestTextBlock_Background");
    return spTxt;
}

static auto GetTitle(const std::string& title)
{
    auto spTxt = std::make_shared<TextBlock>();
    spTxt->SetText(title).SetFontName("").SetFontSize(20).SetMargin({0,10,0,0});
    spTxt->GetForegroundProperty().SetThemeId("SubTitle_Foreground");
    return spTxt;
}

static std::shared_ptr<UIElement> SimpleTextBlock(IRenderer&)
{
    auto spPanel = std::make_shared<GridPanel>();
    spPanel->AddColumn(Panel_Auto);
    spPanel->AddRow(Panel_Auto);
    spPanel->SetBackground(xpf::Colors::AliceBlue);
    spPanel->SetName("Simple TextBlock");

    auto spTxt = std::make_shared<TextBlock>();
    spPanel->AddChild(spTxt);

    spTxt->SetName("TB");
    spTxt->SetText("Abc").SetFontName("Arial").SetFontSize(16);
    // spTxt->SetTextVerticalAlignment(TextVerticalAlignment::Center);
    spTxt->SetBackground(xpf::Colors::Black);
    spTxt->SetBorder(xpf::Colors::Yellow.with_alpha(196));
//    spTxt->SetPadding(5);
    spTxt->SetBorderThickness(1);
    spTxt->SetMargin(5);
    spTxt->SetCornerRadius(0);

    static std::string s_time;
    spTxt->SetOnDraw([](UIElement& e, IRenderer& r)
    {
        ::time_t now = time(0);
        tm* pltm = localtime(&now);

        s_time =
            std::to_string(pltm->tm_hour) + ":" +
            xpf::stringex::to_string_with_digits(pltm->tm_min, 2) + ":" +
            xpf::stringex::to_string_with_digits(pltm->tm_sec, 2);
        ((TextBlock*)(&e))->SetText(s_time);
        return true;
    });

    spPanel->SetOnDraw([](UIElement& e, IRenderer& r)
    {
        static auto spFont = Font::GetFont({"Arial", 16});
        auto bounds = spFont->Measure(s_time);
        bounds.width = round((bounds.width + 10 + 2) + .49);
        bounds.height = round((bounds.height + 10 + 2) + .49);

        RectangleDescription desc;
        desc.width = bounds.width;
        desc.height = bounds.height;
        desc.borderThickness = 1;
        desc.borderColor = xpf::Colors::Red;
        desc.fillColor = xpf::Colors::Transparent;
        r.DrawRectangle(0,0, desc);

        desc.width = 5;
        desc.height = 5;
        desc.borderThickness = 0;
        desc.fillColor = xpf::Colors::HotPink;
        r.DrawRectangle(0,0, desc);
        r.DrawRectangle(bounds.width-5, bounds.height-5, desc);
        return true;
    });

    return spPanel;
}

static std::shared_ptr<StackPanel> VerticalAlignment_Height(std::optional<float> height = std::nullopt)
{
    auto spRoot = std::make_shared<StackPanel>();
    spRoot->SetName(xpf::stringex("TextBlock VerticalAlignment height:") + (height.has_value() ? xpf::stringex::to_string(height.value(), 0) : "nil"));

    for (VerticalAlignment v = VerticalAlignment::Top; v <= VerticalAlignment::Stretch; v = (VerticalAlignment)(uint32_t(v) + 1))
    {
        spRoot->AddChild(GetTitle("VerticalAlignment: " + xpf::to_string(v)));
        auto spRow = std::make_shared<StackPanel>();
        spRow->GetBackgroundProperty().SetThemeId("SubPanel_Background");
        spRow->SetPadding(4).SetHeight(68).SetCornerRadius(6);
        spRow->SetOrientation(Orientation::Horizontal);
        spRoot->AddChild(spRow);

        for (int i = 8; i <= 20; i+=2)
        {
            auto spCtrl = CreateTextBlock();
            spCtrl->SetText(std::to_string(i) + " Abcgy").SetFontName("Arial").SetFontSize(i).SetMargin(4);
            spCtrl->SetVerticalAlignment(v);
            if (height.has_value())
                spCtrl->SetHeight(height.value());
            spRow->AddChild(spCtrl);
        }
    }

    return spRoot;
}

static std::shared_ptr<StackPanel> TextAlignmentTest()
{
    auto spRoot = std::make_shared<StackPanel>();
    spRoot->SetName(xpf::stringex("TextBlock TextAlignment"));

    for (TextAlignment t = TextAlignment::Center; t <= TextAlignment::Right; t = (TextAlignment)(uint32_t(t) + 1))
    {
        spRoot->AddChild(GetTitle("TextAlignment " + xpf::to_string(t)));
        auto spRow = std::make_shared<StackPanel>();
        spRow->SetPadding(4).SetBackground(xpf::Colors::Gray).SetHeight(60);
        spRow->SetOrientation(Orientation::Horizontal);
        spRoot->AddChild(spRow);

        for (int i = 8; i < 20; i+=2)
        {
            auto spCtrl = std::make_shared<TextBlock>();
            spCtrl->SetText(std::to_string(i) + " Abc").SetFontName("Arial").SetFontSize(i).SetBackground(xpf::Colors::Black).SetMargin(4);
            spCtrl->SetTextAlignment(t).SetWidth(160);
            spRow->AddChild(spCtrl);
        }
    }

    return spRoot;
}

};

} // xpf