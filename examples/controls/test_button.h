#pragma once
#include <xpf/ui/GridPanel.h>
#include <xpf/ui/TextBlock.h>
#include <xpf/ui/Button.h>
#include <xpf/ui/StackPanel.h>

namespace xpf {

struct Button_Test {

static auto CreateButton()
{
    auto spTxt = std::make_shared<Button>();
    return spTxt;
}

static auto GetTitle(const std::string& title)
{
    auto spTxt = std::make_shared<TextBlock>();
    spTxt->SetText(title).SetFontName("").SetFontSize(20).SetMargin({0,10,0,0});
    spTxt->GetForegroundProperty().SetThemeId("SubTitle_Foreground");
    return spTxt;
}

static std::shared_ptr<StackPanel> Rectangles()
{
    auto spRoot = std::make_shared<StackPanel>();
    spRoot->SetName("Button rectangles");

    spRoot->AddChild(GetTitle("Basic:"));

    auto spRow = std::make_shared<StackPanel>();
    spRow->GetBackgroundProperty().SetThemeId("SubPanel_Background");
    spRow->SetPadding(4).SetHeight(100).SetCornerRadius(6);
    spRow->SetOrientation(Orientation::Horizontal);
    spRoot->AddChild(spRow);

    auto spCtrl = CreateButton();
    spCtrl->SetText("Testxyj").SetFontName("Arial").SetFontSize(20);
    spCtrl->SetMargin(5).SetBorderThickness(5).SetPadding(5);
    spCtrl->GetContent()->SetBackground(xpf::Colors::Chocolate);
    spCtrl->SetShowDebugView(true);

    spRow->AddChild(spCtrl);
    return spRoot;
}

static std::shared_ptr<StackPanel> VerticalAlignment_Height(std::optional<float> height = std::nullopt)
{
    auto spRoot = std::make_shared<StackPanel>();
    spRoot->SetName(xpf::stringex("Button VerticalAlignment height:") + (height.has_value() ? xpf::stringex::to_string(height.value(), 0) : "nil"));

    for (VerticalAlignment v = VerticalAlignment::Top; v <= VerticalAlignment::Stretch; v = (VerticalAlignment)(uint32_t(v) + 1))
    {
        spRoot->AddChild(GetTitle("VerticalAlignment: " + xpf::to_string(v)));
        auto spRow = std::make_shared<StackPanel>();
        spRow->GetBackgroundProperty().SetThemeId("SubPanel_Background");
        spRow->SetPadding(4).SetHeight(80).SetCornerRadius(6);
        spRow->SetOrientation(Orientation::Horizontal);
        spRoot->AddChild(spRow);

        for (int i = 8; i <= 20; i+=2)
        {
            auto spCtrl = CreateButton();
            spCtrl->SetText(std::to_string(i) + " Abcgy").SetFontName("Arial").SetFontSize(i).SetMargin(4);
            spCtrl->SetVerticalAlignment(v);
            spCtrl->SetWidth(160);//.SetBorderThickness(2);
            if (height.has_value())
                spCtrl->SetHeight(height.value());

            // spCtrl->GetContent()->SetBackground(xpf::Colors::Chocolate);

            spRow->AddChild(spCtrl);
        }
    }

    return spRoot;
}

};

} // xpf