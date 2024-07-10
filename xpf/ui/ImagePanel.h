#pragma once
#include <xpf/core/Image.h>
#include <xpf/ui/UIElement.h>

namespace xpf {

// https://learn.microsoft.com/en-us/dotnet/api/system.windows.controls.image.stretch?view=windowsdesktop-8.0
enum class ImageStretch : uint8_t
{
    // The image is not stretched to fill the output area.
    // If the image is larger than the output area, the image is drawn to the output area,
    // clipping what does not fit.
    None,

    // The image is scaled to fit the output area.
    // Because the image height and width are scaled independently,
    // the original aspect ratio of the image might not be preserved.
    // That is, the image might be warped in order to completely fill the output container.
    Fill,

    //The image is scaled so that it fits completely within the output area.
    // The image's aspect ratio is preserved.
    Uniform,

    // The image is scaled so that it completely fills the output area
    // while preserving the image's original aspect ratio.
    UniformToFill
};

class ImagePanel : public UIElement
{
protected:
    RenderBatch m_renderCommands;
    v2_t m_rawImageSize;
    v2_t m_imageSize;

protected:
    DECLARE_PROPERTY(ImagePanel, std::shared_ptr<ITexture>, Texture, nullptr, Invalidates::Visuals);
    DECLARE_PROPERTY(ImagePanel, ImageStretch, Stretch, ImageStretch::None, Invalidates::Visuals);

public:
    ImagePanel() : UIElement(UIElementType::ImagePanel)
    {
        m_clippingEnabled = true;
    }

protected:
    virtual v2_t OnMeasure(v2_t insideSize) override
    {
        m_rawImageSize = v2_t(m_Texture.Get()->GetWidth(), m_Texture.Get()->GetHeight());

        const rectf_t& textureArea = m_Texture.Get()->GetRegion();
        const float txtWidth = textureArea.width * m_rawImageSize.x;
        const float txtHeight = textureArea.height * m_rawImageSize.y;
        float width = insideSize.w;
        float height = insideSize.h;

        switch (m_Stretch)
        {
            case ImageStretch::None: m_imageSize = {txtWidth, txtHeight}; break;
            case ImageStretch::Fill: m_imageSize = insideSize; break;
            case ImageStretch::Uniform:
            {
                if (width > height)
                    width = (height * txtHeight) / txtWidth;
                else
                    height = (width * txtWidth) / txtHeight;

                m_imageSize = {width, height};
                break;
            }
            case ImageStretch::UniformToFill:
            {
                if (height > width)
                    width = (height * m_rawImageSize.x) / m_rawImageSize.y;
                else
                    height = (width * m_rawImageSize.y) / m_rawImageSize.x;

                m_imageSize = {width, height};
                break;
            }
        }

        return insideSize;
    }

    virtual void OnUpdateVisuals(IRenderer& renderer) override
    {
        UIElement::OnUpdateVisuals(renderer);

        rectf_t textCoords = m_Texture.Get()->GetRegion();

        auto r = renderer.CreateCommandBuilder();
        r.DrawImage(
            m_insideRect.x + (m_insideRect.w - m_imageSize.x) * .5f,
            m_insideRect.y + (m_insideRect.h - m_imageSize.y) * .5f,
            m_imageSize.x, m_imageSize.y,
            m_Texture.Get(),
            textCoords);
        m_renderCommands = r.Build();
    }

    virtual void OnArrange(v2_t insideSize) override
    {
        UIElement::OnArrange(insideSize);
    }

    virtual void OnDraw(IRenderer& renderer) override
    {
        renderer.EnqueueCommands(m_renderCommands);
    }
};

} // xpf