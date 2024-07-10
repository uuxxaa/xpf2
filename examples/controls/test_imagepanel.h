#pragma once
#include <xpf/core/Image.h>
#include <xpf/ui/ImagePanel.h>
#include <xpf/ui/TextBlock.h>
#include <xpf/ui/Border.h>
#include <xpf/ui/Button.h>
#include <xpf/ui/GridPanel.h>
#include "obj/resources.h"

namespace xpf {

struct ImagePanel_Test
{

static auto CreateButton(auto spParent, auto text, auto&& fn)
{
    auto spBtn = std::make_shared<Button>();
    spBtn->SetFontSize(13).SetMargin(3).SetPadding(6).SetCornerRadius(4);
    spParent->AddChild(spBtn);
    spBtn->SetText(text);
    spBtn->SetOnClick(std::move(fn));
    return spBtn;
}

static auto ImageStretchTest(IRenderer& r)
{
    auto img = Image::LoadImage("test.png", resources::test_pattern_01_png());
    auto spTestPattern_NoInterpolation = r.CreateTexture(img);
    auto spTestPattern_Linear = spTestPattern_NoInterpolation->SampledTexture(ITexture::Interpolation::Linear);

    auto spRoot = std::make_shared<GridPanel>();
    spRoot->SetName("Image stretch");
    spRoot->AddRows({PanelLength{36}, Panel_OneStar});

    CreateButton(spRoot, "Toggle Zoom", [spRoot = spRoot, spTestPattern_NoInterpolation = spTestPattern_NoInterpolation, spTestPattern_Linear = spTestPattern_Linear](Button&) {
        static rectf_t rect = {0.25, 0.25, 0.5, 0.5};
        if (spTestPattern_NoInterpolation->GetRegion().x == 0)
        {
            spTestPattern_NoInterpolation->SetRegion({0.25, 0.25, 0.5, 0.5});
            spTestPattern_Linear->SetRegion({0.25, 0.25, 0.5, 0.5});
        }
        else
        {
            spTestPattern_NoInterpolation->SetRegion({0,0,1,1});
            spTestPattern_Linear->SetRegion({0,0,1,1});
        }
        spRoot->InvalidateVisuals();
    });

    auto spPanel = std::make_shared<GridPanel>();
    spRoot->AddChild(spPanel);
    spPanel->Set<Grid_Row>(1);
    spPanel->AddRows({PanelLength{36}, Panel_OneStar, Panel_OneStar});

    int column = 0;
    for (ImageStretch stretch : {
        ImageStretch::None,
        ImageStretch::Uniform,
        ImageStretch::UniformToFill,
        ImageStretch::Fill
        })
    {
        spPanel->AddColumn(Panel_OneStar);

        std::shared_ptr<TextBlock> spTxt = std::make_shared<TextBlock>();
        spTxt->SetText(
            stretch == ImageStretch::None ? "No stretch" :
            (stretch == ImageStretch::Fill ? "Fill" :
            (stretch == ImageStretch::Uniform ? "Uniform" : "UniformToFill")))
            .SetFontSize(20)
            .SetForeground(xpf::Colors::Black)
            .Set<Grid_Column>(column)
            ;
        spPanel->AddChild(spTxt);

        {
            std::shared_ptr<ImagePanel> spImg = std::make_shared<ImagePanel>();
            spImg->SetName(std::to_string(column));
            spImg->SetTexture(spTestPattern_NoInterpolation).SetStretch(stretch)
                //.SetWidth(100).SetHeight(100)
                .SetPadding(10)
                .SetBackground(xpf::Colors::Random())
                .Set<Grid_Column>(column)
                .Set<Grid_Row>(1);
            spPanel->AddChild(spImg);
        }

        {
            std::shared_ptr<ImagePanel> spImg = std::make_shared<ImagePanel>();
            spImg->SetTexture(spTestPattern_Linear).SetStretch(stretch)
                //.SetWidth(100).SetHeight(100)
                .SetPadding(10)
                .SetBackground(xpf::Colors::Random())
                .Set<Grid_Column>(column)
                .Set<Grid_Row>(2);
            spPanel->AddChild(spImg);
        }

        column++;
    }

    return spRoot;
}

};

}