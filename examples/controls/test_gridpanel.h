#pragma once
#include <xpf/ui/GridPanel.h>
#include <xpf/ui/TextBlock.h>
#include <xpf/ui/Border.h>
#include <xpf/ui/Button.h>
#include <xpf/ui/StackPanel.h>

namespace xpf {

struct GridPanel_Test
{
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

static auto SimpleGrid(GridPanel* spPanel)
{
    spPanel->
        AddColumns({Panel_Auto, Panel_Pixel(50), Panel_Star(1), PanelLength(2, PanelLength::Star, 100)})
        .AddRows({Panel_Auto, PanelLength(1, PanelLength::Star, 50), Panel_Auto})
        ;

    for (int r = 0; r < 3; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            auto spText = std::make_shared<TextBlock>(); // UIElementType::UIElement);
            spText->
                SetFontSize(20).SetFontName("")
                .SetName(std::to_string(r) +"," + std::to_string(c) + ((c==0 && r==0) ? "     hello world!" : ""))
            ;
            //spText->SetName(r==0 && c==0 ? "TB" : "");
            spText->Set<Grid_Row>(r);
            spText->Set<Grid_Column>(c);
            spText->SetBorderThickness(10).SetBorder(xpf::Colors::Random());
            spText->SetHorizontalAlignment(HorizontalAlignment::Stretch);
            spText->SetVerticalAlignment(VerticalAlignment::Stretch);
            spText->SetBackground(xpf::Colors::Random().with_alpha(128));
            // if (c == 0 && r!=0)
            //     spText->SetWidth(160);
            if (r == 1 && c == 1)
                spText->SetHeight(120).SetVerticalAlignment(VerticalAlignment::Center);

            spPanel->AddChild(spText);
        }
    }

    spPanel->SetOnDraw([](UIElement& elem, IRenderer& r)
    {
        for (const auto& spChild : ((GridPanel*)&elem)->GetChildren())
        {
           ((TextBlock*)spChild.get())->SetText(
               spChild->GetName() + "\n" + 
               xpf::stringex::to_string(spChild->GetActualRect().width, 0) + "," + 
               xpf::stringex::to_string(spChild->GetActualRect().height, 0));
               
           // spChild->InvalidateLayout();
        }
        // elem.InvalidateLayout();
        return true;
    });

    // spRoot->AddChild(spPanel);

    return true;
}

static auto SimpleGrid()
{
    // auto spRoot = std::make_shared<StackPanel>();
    // spRoot->SetName("Simple Grid");

    // auto spLeft = image_button(0, 0, "alien.png", ButtonType::Button);
    // spLeft->Set<GridPanel_>(100.0f);

    // auto spTop = image_button(0, 0, "medal.png", ButtonType::Button);
    // spTop->Set<Panel_MinLength>(60.0f);

    // auto spBottom = image_button(0, 0, "medal.png", ButtonType::Button);
    // spBottom->Set<Panel_MinLength>(60.0f);

    auto spPanel = std::make_shared<GridPanel>();
    spPanel->SetName("Simple Grid");
    SimpleGrid(spPanel.get());
    // spPanel->SetCornerRadius(4);
    // spPanel->SetBackground(xpf::Colors::Honey);
    // spPanel->SetBorder(xpf::Colors::BlueSuede);
    // spPanel->SetBorderThickness(5);
    return spPanel;
}

};

} // xpf