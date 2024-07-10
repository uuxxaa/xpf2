#pragma once
#include <xpf/ui/GridPanel.h>
#include <xpf/ui/TextBox.h>

namespace xpf {

struct TextBox_Test
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

static auto SimpleTextBox(IRenderer& renderer)
{
    auto spPanel = std::make_shared<StackPanel>();
    spPanel->SetName("Simple TextBox");

    auto spToolbar = std::make_shared<StackPanel>();
    spToolbar->SetName("TB");
    spToolbar->SetOrientation(Orientation::Horizontal).SetVerticalAlignment(VerticalAlignment::Top).SetBackground(xpf::Colors::Teal);
    spPanel->AddChild(spToolbar);

    auto spTxt1 = std::make_shared<TextBox>();
    spTxt1->SetText("Abc").SetFontName("Arial").SetFontSize(20).SetCornerRadius(4).SetVerticalAlignment(VerticalAlignment::Top);
    spPanel->AddChild(spTxt1);

    auto spTxt2 = std::make_shared<TextBox>();
    spTxt2->SetText("Abc").SetFontName("Arial").SetFontSize(20).SetMultiLine(true).SetCornerRadius(4).SetVerticalAlignment(VerticalAlignment::Top);
    spTxt2->SetHeight(200);
    spPanel->AddChild(spTxt2);

    CreateButton(spToolbar, "Arial", [spPanel = spPanel, spTxt1 = spTxt1, spTxt2 = spTxt2](auto& btn)
    {
        spTxt1->SetFontName(btn.GetName());
        spTxt2->SetFontName(btn.GetName());
    });

    CreateButton(spToolbar, "Menlo", [spTxt1 = spTxt1, spTxt2 = spTxt2](auto& btn)
    {
        spTxt1->SetFontName(btn.GetName());
        spTxt2->SetFontName(btn.GetName());
    });

    CreateButton(spToolbar, "Default", [spTxt1 = spTxt1, spTxt2 = spTxt2](auto& btn)
    {
        spTxt1->SetFontName(btn.GetName());
        spTxt2->SetFontName(btn.GetName());
    });

    CreateSizeButton(spToolbar, 10, [spTxt1 = spTxt1, spTxt2 = spTxt2](auto& btn)
    {
        spTxt1->SetFontSize(10);
        spTxt2->SetFontSize(10);
    });

    CreateSizeButton(spToolbar, 13, [spTxt1 = spTxt1, spTxt2 = spTxt2](auto& btn)
    {
        spTxt1->SetFontSize(13);
        spTxt2->SetFontSize(13);
    });

    CreateSizeButton(spToolbar, 16, [spTxt1 = spTxt1, spTxt2 = spTxt2](auto& btn)
    {
        spTxt1->SetFontSize(16);
        spTxt2->SetFontSize(16);
    });

    CreateSizeButton(spToolbar, 20, [spTxt1 = spTxt1, spTxt2 = spTxt2](auto& btn)
    {
        spTxt1->SetFontSize(20);
        spTxt2->SetFontSize(20);
    });

    return spPanel;
}

};

} // xpf