#pragma once
#include <xpf/ui/GridPanel.h>
#include <xpf/ui/TextBlock.h>
#include <xpf/ui/ImagePanel.h>
#include <xpf/ui/Border.h>
#include <xpf/ui/Button.h>
#include <xpf/ui/StackPanel.h>
#include <xpf/ui/TreePanel/TreePanel.h>
#include "obj/resources.h"

namespace xpf {

struct TreePanel_Test {

static auto CreateButton(auto spParent, auto text, auto&& fn)
{
    auto spBtn = std::make_shared<Button>();
    spBtn->SetFontSize(13).SetMargin(3).SetPadding(3).SetCornerRadius(4);
    spParent->AddChild(spBtn);
    spBtn->SetText(text);
    spBtn->SetOnClick(std::move(fn));
    return spBtn;
}

static auto TreeControl(IRenderer& r)
{
    auto spRoot = std::make_shared<StackPanel>();
    spRoot->SetName("Tree Control");

    auto spToolbar = std::make_shared<StackPanel>();
    spToolbar->SetOrientation(Orientation::Horizontal)
        .SetHorizontalAlignment(HorizontalAlignment::Stretch)
        .Set<Panel_Length>(Panel_Auto);
    spRoot->AddChild(spToolbar);

    auto spTree = std::make_shared<TreePanel>();
    spTree->SetName("Tree")
        .SetHorizontalAlignment(HorizontalAlignment::Stretch)
        .Set<Panel_Length>(Panel_OneStar);
    spRoot->AddChild(spTree);

    auto spData = std::make_shared<TreeData>();
    spTree->SetData(spData);

    auto folderOpenImg = Image::LoadImage("a.png", resources::folder_open_png());
    auto spOpenTexture = r.CreateTexture(folderOpenImg);
    auto spExpandedIcon = std::make_shared<ImagePanel>();
    spExpandedIcon->SetTexture(spOpenTexture).SetStretch(ImageStretch::Uniform).SetVerticalAlignment(VerticalAlignment::Center).SetHeight(20).SetWidth(30).SetMargin({0,0,10,0});

    auto folderClosedImg = Image::LoadImage("a.png", resources::folder_closed_png());
    auto spClosedTexture = r.CreateTexture(folderClosedImg);
    auto spCollapsedIcon = std::make_shared<ImagePanel>();
    spCollapsedIcon->SetTexture(spClosedTexture).SetStretch(ImageStretch::Uniform).SetVerticalAlignment(VerticalAlignment::Center).SetHeight(20).SetWidth(30).SetMargin({0,0,10,0});

    auto spItemIcon = std::make_shared<TextBlock>();
    spItemIcon->SetText("\uf16a").SetFontName("segoeicons").SetFontSize(16)
        .SetTextVerticalAlignment(TextVerticalAlignment::Center)
        .SetVerticalAlignment(VerticalAlignment::Center)
        .SetHorizontalAlignment(HorizontalAlignment::Center)
        .SetHeight(20).SetWidth(20).SetForeground(xpf::Colors::XpfBlack)
        .SetOnDraw([spItemIcon](UIElement& elem, IRenderer&) mutable
        {
            static Tween<radians_t> tw = [spItemIcon](){
                Tween<radians_t> tw;
                tw.To(math::PI * 2, 1.0f).Repeat(3).Play().OnComplete([&tw, spItemIcon](bool willRepeat) mutable
                {
                    if (!willRepeat)
                        spItemIcon->SetText(" ");
                });
                return tw;
            }();
            float v = tw.Read();
            elem.SetRotationAngle(v);
            return true;
        });

    for (int p = 0; p < 2; p++)
    {
        auto spRootNode = std::make_shared<TreeParentNode>();
        spRootNode->SetExpandedIcon(spExpandedIcon);
        spRootNode->SetIcon(spCollapsedIcon);
        spRootNode->SetText("Root node: " + std::to_string(p));
        spData->AddNode(spRootNode);

        for (int r = 1; r < 3; r++)
        {
            auto spSubRootNode = std::make_shared<TreeParentNode>();
            spSubRootNode->SetExpandedIcon(spExpandedIcon);
            spSubRootNode->SetIcon(spCollapsedIcon);
            spSubRootNode->SetText("Sub node: " + std::to_string(p) + ":" + std::to_string(r));
            spSubRootNode->SetIsExpanded(false);
            spSubRootNode->SetOnExpandingOrCollapsing([&, spItemIcon = spItemIcon](TreeParentNode& parent, bool collapsing)
            {
                if (collapsing)
                    return parent.RemoveAllNodes();

                if (parent.ChildCount() != 0)
                    return;

                for (int i = 0; i < 5; i++)
                {
                    auto spLeafNode = std::make_shared<TreeLeafNode>();
                    spLeafNode->SetText(std::to_string(p) + ":" + std::to_string(r) + ":" + std::to_string(i) + " node");
                    spLeafNode->SetIcon(spItemIcon);
                    parent.AddNode(spLeafNode);
                }
            });

            spRootNode->AddNode(spSubRootNode);
        }
    }

    CreateButton(spToolbar, "Dbl Click to Expand", [](auto& btn)
    {
//        fontName = "Menlo";
    });
    return spTree;
}

};

} // xpf