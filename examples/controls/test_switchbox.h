#pragma once
#include <xpf/ui/StackPanel.h>
#include <xpf/ui/SwitchBox.h>
#include <xpf/ui/TextBlock.h>

namespace xpf {

struct SwitchBox_Test
{

static auto CreateButton(auto spParent, auto fontName, auto&& fn)
{
    auto spBtn = std::make_shared<Button>();
    spBtn->SetFontSize(13).SetMargin(4).SetPadding(4).SetCornerRadius(4);
    spParent->AddChild(spBtn);
    spBtn->SetText(fontName);
    spBtn->SetName(fontName);
    spBtn->SetOnClick(std::move(fn));
    return spBtn;
}

static auto CreateSizeButton(auto spParent, auto fontSize, auto&& fn)
{
    auto spBtn = std::make_shared<Button>();
    spBtn->SetFontSize(13).SetMargin(4).SetPadding(4).SetCornerRadius(4);
    spBtn->SetText(std::to_string(fontSize));
    spBtn->SetOnClick(std::move(fn));
    spParent->AddChild(spBtn);
    return spBtn;
}

static auto SimpleSwitchBox(IRenderer& renderer)
{
    auto spPanel = std::make_shared<StackPanel>();
    spPanel->SetName("Simple SwitchBox");

    auto spSB1 = std::make_shared<SwitchBox>();
    {
        spSB1->SetFontName("Arial").SetFontSize(16).SetFontSizeIsInPixels(true).SetItemMargin({8,4,8,4}).SetMargin(10);
        spSB1->SetBackground(xpf::Colors::DarkGray);
        spSB1->AddOption(nullptr, "Zeynep Banu", xpf::Colors::Green, xpf::Colors::DodgerBlue, /*isIconOnly:*/ false);
        spSB1->AddOption(nullptr, "Arda", xpf::Colors::Gold, xpf::Colors::BlueSuede, /*isIconOnly:*/ false);
        spSB1->AddOption(nullptr, "Ekin Bora", xpf::Colors::Honey, xpf::Colors::BlueViolet, /*isIconOnly:*/ false);
        spPanel->AddChild(spSB1);
    }
    {
        auto spSB = std::make_shared<SwitchBox>();
        spSB->SetFontName("Arial").SetFontSize(16).SetFontSizeIsInPixels(true).SetItemMargin({8,4,8,4}).SetMargin(10);
        spSB->SetBackground(xpf::Colors::DarkGray);
        spSB->AddOption(nullptr, "Zeynep Banu");
        spSB->AddOption(nullptr, "Arda");
        spSB->AddOption(nullptr, "Ekin Bora");
        spPanel->AddChild(spSB);
    }
    {
        auto spSubPanel = std::make_shared<StackPanel>();
        spSubPanel->SetOrientation(Orientation::Horizontal).SetBackground(xpf::Colors::Honey);
        spPanel->AddChild(spSubPanel);

        auto spSB = std::make_shared<SwitchBox>();
        spSB->SetFontName("Arial").SetFontSize(16).SetFontSizeIsInPixels(true).SetItemMargin({8,4,8,4}).SetMargin(10);
        spSB->SetVerticalAlignment(VerticalAlignment::Center);
        spSB->SetBackground(xpf::Colors::DarkGray);
        spSB->AddOption(nullptr, "Zeynep Banu", xpf::Colors::Green, xpf::Colors::DodgerBlue, /*isIconOnly:*/ false);
        spSB->AddOption(nullptr, "Arda", xpf::Colors::Gold, xpf::Colors::BlueSuede, /*isIconOnly:*/ false);
        spSB->AddOption(nullptr, "Ekin Bora", xpf::Colors::Honey, xpf::Colors::BlueViolet, /*isIconOnly:*/ false);
        spSubPanel->AddChild(spSB);

        auto spTxt = std::make_shared<TextBlock>();
        spTxt->SetFontName("").SetFontSize(20)
            .SetMargin(20)
            // .SetBackground(xpf::Colors::XpfBlack)
            .SetForeground(xpf::Colors::LimeGreen)
            .SetHorizontalAlignment(HorizontalAlignment::Left)
            .SetVerticalAlignment(VerticalAlignment::Center)
            .Set<Panel_Length>(Panel_Auto);
        spSubPanel->AddChild(spTxt);

        spSB->GetSelectedItemProperty().SetOnChanged([spTxt](auto, uint32_t selectedItem)
        {
            spTxt->SetText("Selected Item: " + std::to_string(selectedItem));
        });
    }

    auto spRow = std::make_shared<StackPanel>();
    spRow->SetOrientation(Orientation::Horizontal);
    spPanel->AddChild(spRow);

    auto spSB2 = std::make_shared<SwitchBox>();
    spSB2->SetFontName("Arial").SetFontSize(16).SetFontSizeIsInPixels(true).SetItemMargin(4).SetOrientation(Orientation::Vertical).SetMargin(10);
    spSB2->SetBackground(xpf::Colors::DarkGray);
    spSB2->AddOption(nullptr, "Option1", xpf::Colors::Green, xpf::Colors::DodgerBlue, /*isIconOnly:*/ false);
    spSB2->AddOption(nullptr, "Option2", xpf::Colors::Gold, xpf::Colors::BlueSuede, /*isIconOnly:*/ false);
    spSB2->AddOption(nullptr, "Option3", xpf::Colors::Honey, xpf::Colors::BlueViolet, /*isIconOnly:*/ false);
    spRow->AddChild(spSB2);

    auto spSB3 = std::make_shared<SwitchBox>();
    spSB3->SetFontName("SegoeIcons")
        .SetFontSize(24)
        .SetFontSizeIsInPixels(true)
        .SetItemMargin(10)
        .SetOrientation(Orientation::Vertical)
        .SetMargin(10)
        .SetPadding(3)
        .SetBackground(xpf::Colors::Black)
        //.SetCornerRadius(25)
        ;
    spSB3->AddOption(nullptr, "\ue1d5", xpf::Colors::Green, xpf::Colors::DodgerBlue, /*isIconOnly:*/ false);
    spSB3->AddOption(nullptr, "\ue1d6", xpf::Colors::Gold, xpf::Colors::BlueSuede, /*isIconOnly:*/ false);
    spSB3->AddOption(nullptr, "\ue1d0", xpf::Colors::Honey, xpf::Colors::BlueViolet, /*isIconOnly:*/ false);
    spRow->AddChild(spSB3);

    spPanel->SetOnDraw([spSB1, spSB2, spSB3](UIElement&, IRenderer&)
    {
        spSB1->SetSelectedItem(uint32_t(Time::GetTime()) % 3);
        spSB2->SetSelectedItem(uint32_t(Time::GetTime()) % 3);
        spSB3->SetSelectedItem(uint32_t(Time::GetTime()) % 3);
        return true;
    });

    return spPanel;
}

};

} // xpf