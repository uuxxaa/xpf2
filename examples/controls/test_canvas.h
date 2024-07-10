#pragma once
#include <xpf/ui/StackPanel.h>
#include <xpf/ui/UIElement.h>
#include <xpf/ui/Button.h>
#include <xpf/ui/TextBlock.h>
#include <xpf/ui/SwitchBox.h>
#include <xpf/core/Time.h>
#include <ctime>
#include <nanosvg/nanosvg.h>
#include <nanosvg/nanosvgrast.h>

namespace xpf {

struct DrawText_Test
{
    static auto CreateOptionButton(auto spParent, const std::vector<std::string>& options, auto&& fn)
    {
        auto spBtn = std::make_shared<SwitchBox>();
        spBtn->SetFontName("Arial").SetFontSize(16).SetFontSizeIsInPixels(true).SetItemMargin({8,4,8,4}).SetMargin(10);
        for (const auto& option : options)
            spBtn->AddOption(nullptr, option);
        spBtn->GetSelectedItemProperty().SetOnChanged([fn = std::move(fn)](UIElement*, uint32_t newValue)
        {
            fn(newValue);
        });
        spParent->AddChild(spBtn);
        return spBtn;
    }
/*
            std::string svg = "<?xml version='1.0'?>"
                "<svg xmlns='http://www.w3.org/2000/svg' width='467' height='462' stroke='#000' stroke-width='10'>"
                " <rect x='80' y='60' width='250' height='250' rx='20' fill='#F00'/>"
                " <rect x='140' y='120' width='250' height='250' rx='40' fill='#00F' fill-opacity='.7'/>"
                "</svg>";

            NSVGimage* psvg = nsvgParse(svg.data(), "px", 96);
            static std::vector<byte_t> image;
            image.resize(psvg->width * psvg->height * 4);
            struct NSVGrasterizer* rast = nsvgCreateRasterizer();
            nsvgRasterize(rast, psvg, 0,0,1, image.data(), psvg->width, psvg->height, psvg->width * 4);
            nsvgDeleteRasterizer(rast);

            auto spTexture = ITexture::TextureLoader(std::move(image), psvg->width, psvg->height, PixelFormat::R8G8B8A8, / *mipMaps* / 1);
            nsvgDelete(psvg);
            r.DrawImage(
                10, 10,
                spTexture->GetWidth(), spTexture->GetHeight(),
                spTexture,
                spTexture->GetRegion());
                */
    static auto DrawSvg(IRenderer& renderer)
    {
        std::string svg = "<?xml version='1.0'?>"
            "<svg xmlns='http://www.w3.org/2000/svg' width='467' height='462' stroke='#000' stroke-width='10'>"
            " <rect x='80' y='60' width='250' height='250' rx='20' fill='#F00'/>"
            " <rect x='140' y='120' width='250' height='250' rx='40' fill='#00F' fill-opacity='.7'/>"
            "</svg>";

        auto r = renderer.CreateCommandBuilder();
        NSVGimage* psvg = nsvgParse(svg.data(), "px", 96);

        for (NSVGshape* shape = psvg->shapes; shape != NULL; shape = shape->next) {
            for (NSVGpath* path = shape->paths; path != NULL; path = path->next) {
                for (int i = 0; i < path->npts-1; i += 3) {
                    xpf::Color stroke;
                    switch(shape->stroke.type)
                    {
                        case 1: stroke = xpf::Color(shape->stroke.color); break;
                        case 2: stroke = xpf::Colors::Purple; break;
                    }
                    float* p = &path->pts[i*2];

                    r.DrawBezierCubic(
                        v2_t(p[0],p[1]),
                        v2_t(p[2],p[3]),
                        v2_t(p[4],p[5]),
                        v2_t(p[6],p[7]),
                        shape->strokeWidth,
                        stroke,
                        LineOptions::CapRound | LineOptions::JointRound);

                    xpf::Color fill;
                    switch(shape->fill.type)
                    {
                        case 1: fill = xpf::Color(shape->fill.color); break;
                        case 2: fill = xpf::Colors::Purple; break;
                    }

                    r.DrawBezierCubicQuad(
                        v2_t(p[0],p[1]),
                        v2_t(p[2],p[3]),
                        v2_t(p[4],p[5]),
                        v2_t(p[6],p[7]),
                        fill);
                }
            }
        }
        nsvgDelete(psvg);
        auto cmds = r.Build();

        auto spRoot = std::make_shared<StackPanel>();
        spRoot->SetName("Svg").SetBackground(xpf::Colors::White).SetCornerRadius(8);
        spRoot->SetOnDraw([cmds = std::move(cmds)](UIElement& /*e*/, IRenderer& r)
        {
            r.EnqueueCommands(cmds);
            return true;
        });
        return spRoot;
    }

    static auto ShaderRenderedText()
    {
        auto spRoot = std::make_shared<StackPanel>();
        spRoot->SetName("Clock").SetBackground(xpf::Colors::White).SetCornerRadius(8);

        auto spToolbar = std::make_shared<StackPanel>();
        spToolbar->Set<Panel_Length>(Panel_Auto);
        spToolbar->SetOrientation(Orientation::Horizontal);
        spRoot->AddChild(spToolbar);

        static FontRenderOptions renderOptions = FontRenderOptions::Shaded;
        CreateOptionButton(spToolbar, {"Shaded", "TriShaded"}, [](uint32_t selectedItem)
        {
            if (selectedItem == 0)
                renderOptions = FontRenderOptions::Shaded;
            else
                renderOptions = FontRenderOptions::ShadedByTris;
        })->SetSelectedItem(0);

        static std::string fontName = "Menlo";
        CreateOptionButton(spToolbar, {"Menlo", "JB", "Arial"}, [](uint32_t selectedItem)
        {
            if (selectedItem == 0)
                fontName = "Menlo";
            else if (selectedItem == 1)
                fontName = "/Users/umutalev/Library/Fonts/JetBrainsMono-Regular";
            else if (selectedItem == 2)
                fontName = "Arial Rounded Bold";
        })->SetSelectedItem(0);

        static uint16_t fontSize = 100;
        CreateOptionButton(spToolbar, {"10", "20", "40", "100", "300"}, [](uint32_t selectedItem)
        {
            if (selectedItem == 0)
                fontSize = 10;
            else if (selectedItem == 1)
                fontSize = 20;
            else if (selectedItem == 2)
                fontSize = 40;
            else if (selectedItem == 3)
                fontSize = 100;
            else if (selectedItem == 4)
                fontSize = 300;
        })->SetSelectedItem(3);

        auto spCanvas = std::make_shared<UIElement>(UIElementType::UIElement);
        spCanvas->Set<Panel_Length>(Panel_OneStar);
        spRoot->AddChild(spCanvas);

        spCanvas->SetOnDraw([](UIElement& /*e*/, IRenderer& r)
        {
            ::time_t now = time(0);
            tm* pltm = localtime(&now);

            std::string txt =
                std::to_string(pltm->tm_hour) + ":" +
                //"1:" +
                xpf::stringex::to_string_with_digits(pltm->tm_min, 2) + ":" +
                xpf::stringex::to_string_with_digits(pltm->tm_sec, 2);

            // auto scope = r.TranslateTransfrom(10 - std::cosf(Time::GetTime()), 10 * std::sinf(Time::GetTime()));
            // auto scope2 = r.RotateTransform(std::cosf(Time::GetTime())/20);
            TextDescription desc;
            desc.foreground = xpf::Colors::Black;
            desc.fontName = fontName;
            desc.fontSize = fontSize;
            desc.renderOptions = renderOptions;
            float y = floorf(10 + 10 * sin(Time::GetTime()));
            float x = floorf(10 + 10 * cos(Time::GetTime()));
            // r.DrawText("ajKqç", x, y, desc);
            r.DrawText("çşßğæü 0123456789", x, y, desc); y += fontSize * 1.4;
            r.DrawText("ABCDEFGHIJKLMNOPQRSTUVWXYZ", x, y, desc); y += fontSize * 1.4;
            r.DrawText("abcdefghijklmnopqrstuvwxyz", x, y, desc); y += fontSize * 1.4;
            // r.DrawText("çşßğæü", x, y, desc); y += fontSize * 1.2;
            return true;
        });

        return spRoot;
    }

    static inline RenderBatch wallOfTextCache;
    static void WallOftext(IRenderer& rr, std::string_view fontName, uint16_t fontSize, xpf::Color color,  FontRenderOptions renderOptions)
    {
        auto r = rr.CreateCommandBuilder();
        std::string_view loremIpsum = "VA, Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
        float y = 0;
        for (int i = 0; i < 100; i++)
        {
            TextDescription desc;
            desc.fontName = fontName;
            desc.fontSize = fontSize;
            desc.renderOptions = renderOptions;
            desc.foreground = color;
            r.DrawText(loremIpsum, 10, y, desc);
            y += fontSize * 1.5f;
        }
        
        wallOfTextCache = r.Build();
    }

    static auto CreateButton(auto spParent, auto fontName, auto&& fn)
    {
        auto spBtn = std::make_shared<Button>();
        spBtn->SetFontSize(20).SetMargin(10).SetPadding(4).SetCornerRadius(4);
        spParent->AddChild(spBtn);
        spBtn->SetText(fontName);
        spBtn->SetOnClick(std::move(fn));
        return spBtn;
    }

    static auto CreateSizeButton(auto spParent, auto fontSize, auto&& fn)
    {
        auto spBtn = std::make_shared<Button>();
        spBtn->SetFontSize(20).SetMargin(10).SetPadding(4).SetCornerRadius(4);
        spBtn->SetText(std::to_string(fontSize));
        spBtn->SetBorder_Default(xpf::Colors::Transparent);
        spBtn->SetOnClick(std::move(fn));
        spParent->AddChild(spBtn);
        return spBtn;
    }

    static auto ShaderRenderedWallOfText(IRenderer& r)
    {
        static int fontSize = 10;
        static std::string fontName = "Menlo";
        static FontRenderOptions renderOptions = FontRenderOptions::Texture;
        static xpf::Color textColor = xpf::Colors::Black;

        auto spRoot = std::make_shared<StackPanel>();
        spRoot->SetName("Lorem");

        auto spToolbar = std::make_shared<StackPanel>();
        spToolbar->Set<Panel_Length>(Panel_Auto);
        spToolbar->SetOrientation(Orientation::Horizontal);
        spRoot->AddChild(spToolbar);

        auto spCanvas = std::make_shared<UIElement>(UIElementType::UIElement);
        spCanvas->Set<Panel_Length>(Panel_OneStar);
        spRoot->AddChild(spCanvas);

        WallOftext(r, fontName, fontSize, textColor, renderOptions);

        spCanvas->SetOnDraw([](UIElement& /*e*/, IRenderer& r)
        {
            auto scope = r.TranslateTransfrom(floorf(10.0f * sin(Time::GetTime())), 0);
            r.EnqueueCommands(wallOfTextCache);

            return true;
        });

        CreateOptionButton(spToolbar, {"Textured", "Shaded", "TriShaded"}, [&r](uint32_t selectedItem)
        {
            renderOptions = FontRenderOptions::Texture;
            if (selectedItem == 1)
                renderOptions = FontRenderOptions::Shaded;
            else if (selectedItem == 2)
                renderOptions = FontRenderOptions::ShadedByTris;

            WallOftext(r, fontName, fontSize, textColor, renderOptions);
        })->SetSelectedItem(0);

        CreateOptionButton(spToolbar, {"10", "12", "13", "20", "100"}, [&r](uint32_t selectedItem)
        {
            if (selectedItem == 0)
                fontSize = 10;
            if (selectedItem == 1)
                fontSize = 12;
            if (selectedItem == 2)
                fontSize = 13;
            if (selectedItem == 3)
                fontSize = 20;
            if (selectedItem == 4)
                fontSize = 100;

            WallOftext(r, fontName, fontSize, textColor, renderOptions);
        })->SetSelectedItem(0);

        CreateOptionButton(spToolbar, {"", "Menlo", "Arial"}, [&r](uint32_t selectedItem)
        {
            if (selectedItem == 0)
                fontName = "";
            else if (selectedItem == 1)
                fontName = "Menlo";
            else if (selectedItem == 2)
                fontName = "Arial Rounded Bold";

            WallOftext(r, fontName, fontSize, textColor, renderOptions);
        })->SetSelectedItem(1);

        CreateOptionButton(spToolbar, {"Black", "Red", "Green", "Blue", "Yellow"}, [&r](uint32_t selectedItem)
        {
            if (selectedItem == 0)
                textColor = xpf::Colors::Black;
            else if (selectedItem == 1)
                textColor = xpf::Colors::XpfRed;
            else if (selectedItem == 2)
                textColor = xpf::Colors::Green;
            else if (selectedItem == 3)
                textColor = xpf::Colors::Blue;
            else if (selectedItem == 4)
                textColor = xpf::Colors::Yellow;

            WallOftext(r, fontName, fontSize, textColor, renderOptions);
        })->SetSelectedItem(0);
        return spRoot;
    }

    static auto ShaderRenderedText2()
    {
        static int fontSize = 20;
        static std::string fontName = "Menlo";

        auto spRoot = std::make_shared<StackPanel>();
        spRoot->SetName("Text Canvas");

        auto spToolbar = std::make_shared<StackPanel>();
        spToolbar->Set<Panel_Length>(Panel_Auto);
        spToolbar->SetOrientation(Orientation::Horizontal);
        spRoot->AddChild(spToolbar);

        auto spCanvas = std::make_shared<UIElement>(UIElementType::UIElement);
        spCanvas->Set<Panel_Length>(Panel_OneStar);
        spRoot->AddChild(spCanvas);

        spCanvas->SetOnDraw([](UIElement& /*e*/, IRenderer& r)
        {
            // std::string txt = "TTTTTTTTTT";
            std::string txt = " 12 AbcOOOOOOO";

            r.DrawRectangle(0,0, 400, 400, xpf::Colors::White);

            float y = 0;
            r.DrawText("textured:", 10, y, "", 20, xpf::Colors::Tomato);

            y += 20;
            r.DrawText(std::to_string(fontSize) + txt, 10, y, fontName, fontSize, xpf::Colors::Blue);

            // y += 10 + std::abs(fontSize * 1.5f);
            // {
            //     TextDescription desc;
            //     desc.fontName = fontName;
            //     desc.fontSize = fontSize;
            //     desc.background = xpf::Colors::AliceBlue;
            //     desc.foreground = xpf::Colors::Black;
            //     desc.fontIndex = 0;
            //     r.DrawText("12", 10, y, desc);
            // }

            y += 10 + std::abs(fontSize * 1.5f);
            r.DrawText("shaded:", 10, y, "", 20, xpf::Colors::Tomato);

            y += 20;
            // auto scope = r.TranslateTransfrom(10 - std::cosf(Time::GetTime()), 10 * std::sinf(Time::GetTime()));
            // auto scope2 = r.RotateTransform(std::cosf(Time::GetTime())/20);
            TextDescription desc;
            desc.fontName = fontName;
            desc.fontSize = fontSize;
            desc.renderOptions = FontRenderOptions::Shaded;
            desc.foreground = xpf::Colors::XpfRed;
            r.DrawText(std::to_string(fontSize) + txt, 10, y, desc);

            return true;
        });

        CreateButton(spToolbar, "Menlo", [](auto& btn)
        {
            fontName = "Menlo";
        });

        CreateButton(spToolbar, "Arial", [](auto& btn)
        {
            fontName = "Arial Rounded Bold";
        });

        CreateSizeButton(spToolbar, 12, [](auto& btn)
        {
            fontSize = 12;
        });

        CreateSizeButton(spToolbar, 13, [](auto& btn)
        {
            fontSize = 13;
        });

        CreateSizeButton(spToolbar, 20, [](auto& btn)
        {
            fontSize = 20;
        });

        CreateSizeButton(spToolbar, 100, [](auto& btn)
        {
            fontSize = 100;
        });

        CreateSizeButton(spToolbar, 200, [](auto& btn)
        {
            fontSize = 200;
        });

        return spRoot;
    }

    static auto RoundedRectangle()
    {
        auto spRoot = std::make_shared<StackPanel>();
        spRoot->SetName("Rounded Rect")
            .SetBackground(xpf::Colors::XpfWhite);

        auto spCanvas = std::make_shared<UIElement>(UIElementType::UIElement);
        spCanvas->Set<Panel_Length>(Panel_OneStar);
        spRoot->AddChild(spCanvas);

        spCanvas->SetOnDraw([](UIElement& /*e*/, IRenderer& r)
        {
            RectangleDescription desc;
            desc.borderColor = xpf::Colors::Tomato;
            desc.borderThickness = 5;
            desc.cornerRadius = 10;
            desc.fillColor = xpf::Colors::Honey;
            desc.width = 160;
            desc.height = 60;
            r.DrawRectangle(10, 10, desc);
            return true;
        });

        return spRoot;
    }
};

} // xpf