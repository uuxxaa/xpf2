#pragma once
#include <xpf/ui/TextBlock.h>
#include <xpf/ui/ItemPanel.h>

namespace xpf {

enum class ButtonType
{
    Button, // default
    Toggle,
};

enum class ButtonState : uint8_t
{
    Default = 0x0,
    Hover   = 0x1,
    Pressed = 0x2,
    Focused = 0x4,
};

ENUM_CLASS_FLAG_OPERATORS(ButtonState);

class Button : public ItemPanel
{
protected:
    static inline xpf::Color s_dark = xpf::Color::from_gray(0x2e);
    static inline xpf::Color s_lessdark = xpf::Color::from_gray(0x4e);
    static inline xpf::Color s_lighter = xpf::Color::from_gray(0x6e);

    DECLARE_PROPERTY(Button, ButtonType, ButtonType, ButtonType::Button, Invalidates::Visuals);
    DECLARE_PROPERTY(Button, bool,       ToggledOn, false, Invalidates::Visuals);

    DECLARE_PROPERTY(Button, std::shared_ptr<UIElement>, Content_Default, nullptr, Invalidates::Visuals);
    DECLARE_PROPERTY(Button, std::shared_ptr<UIElement>, Content_Focus, nullptr, Invalidates::Visuals);
    DECLARE_PROPERTY(Button, std::shared_ptr<UIElement>, Content_Hover, nullptr, Invalidates::Visuals);
    DECLARE_PROPERTY(Button, std::shared_ptr<UIElement>, Content_Pressed, nullptr, Invalidates::Visuals);

    DECLARE_THEMED_PROPERTY(Button, xpf::Color, Foreground_Default, xpf::Colors::XpfWhite, Invalidates::Visuals);
    DECLARE_THEMED_PROPERTY(Button, xpf::Color, Foreground_Focus, xpf::Colors::White, Invalidates::Visuals);
    DECLARE_THEMED_PROPERTY(Button, xpf::Color, Foreground_Hover, xpf::Colors::White, Invalidates::Visuals);
    DECLARE_THEMED_PROPERTY(Button, xpf::Color, Foreground_Pressed, xpf::Colors::White, Invalidates::Visuals);

    DECLARE_THEMED_PROPERTY(Button, xpf::Color, Border_Default, s_lessdark, Invalidates::Visuals);
    DECLARE_THEMED_PROPERTY(Button, xpf::Color, Border_Focus, xpf::Colors::XpfBlack.with_alpha(220), Invalidates::Visuals);
    DECLARE_THEMED_PROPERTY(Button, xpf::Color, Border_Hover, xpf::Colors::XpfBlack.with_alpha(220), Invalidates::Visuals);
    DECLARE_THEMED_PROPERTY(Button, xpf::Color, Border_Pressed, xpf::Colors::XpfBlack.with_alpha(220), Invalidates::Visuals);

    DECLARE_THEMED_PROPERTY(Button, xpf::Color, Background_Default, s_dark, Invalidates::Visuals);
    DECLARE_THEMED_PROPERTY(Button, xpf::Color, Background_Focus, s_lessdark, Invalidates::Visuals);
    DECLARE_THEMED_PROPERTY(Button, xpf::Color, Background_Hover, s_lessdark, Invalidates::Visuals);
    DECLARE_THEMED_PROPERTY(Button, xpf::Color, Background_Pressed, s_lighter, Invalidates::Visuals);

    DECLARE_PROPERTY(Button, std::string, FontName, "Arial", Invalidates::SelfLayout);
    DECLARE_PROPERTY(Button, int16_t, FontSize, 16, Invalidates::ParentLayout);
    DECLARE_PROPERTY(Button, bool, FontSizeIsInPixels, false, Invalidates::ParentLayout);

public:
    Button(UIElementType type = UIElementType::Button)
        : ItemPanel(type)
    {
        m_clippingEnabled = true;
        m_acceptsFocus = true;
        m_visualsInvalidated = true;
        m_HorizontalAlignment = HorizontalAlignment::Center;
        m_VerticalAlignment = VerticalAlignment::Center;
    }

    Button& SetText(std::string text)
    {
        if (m_spChild == nullptr || UIElementType::TextBlock != m_spChild->GetType())
        {
            auto spTextBlock = std::make_shared<TextBlock>();
            spTextBlock->SetVerticalAlignment(VerticalAlignment::Center);
            spTextBlock->SetHorizontalAlignment(HorizontalAlignment::Center);
            spTextBlock->SetTextAlignment(TextAlignment::Center);
            SetContent(spTextBlock);
        }

        ((TextBlock*)m_spChild.get())->SetText(text);

        InvalidateParentLayout();
        return *this;
    }

    Button& SetContent(const std::shared_ptr<UIElement>& spVisual)
    {
        m_Content_Default = m_spChild = spVisual;
        InvalidateLayout();
        AddVisual(spVisual.get());
        return *this;
    }

    const std::shared_ptr<UIElement>& GetContent() const
    {
        return m_spChild;
    }

    virtual v2_t OnMeasure(v2_t constraint) override
    {
        if (m_spChild != nullptr)
        {
            if (UIElementType::TextBlock == m_spChild->GetType())
            {
                TextBlock* pTextBlock = (TextBlock*)m_spChild.get();
                pTextBlock->SetFontName(GetFontName());
                pTextBlock->SetFontSize(GetFontSize());
                pTextBlock->SetFontSizeIsInPixels(GetFontSizeIsInPixels());
            }

            return m_spChild->Measure(constraint);
        }

        return constraint;
    }

    virtual void OnUpdateVisuals(IRenderer& renderer) override
    {
        if (m_spChild == nullptr)
        {
            if (m_ToggledOn || m_btnstate == ButtonState::Pressed)
                m_spChild = m_Content_Pressed;
            else if (m_btnstate == ButtonState::Focused)
                m_spChild = m_Content_Focus;
            else if (m_btnstate == ButtonState::Hover)
                m_spChild = m_Content_Hover;
            else
                m_spChild = m_Content_Default;
        }
        else if (m_spChild != nullptr && UIElementType::TextBlock == m_spChild->GetType())
        {
            TextBlock* pTextBlock = (TextBlock*)m_spChild.get();
            pTextBlock->SetFontName(GetFontName());
            pTextBlock->SetFontSize(GetFontSize());
            pTextBlock->SetFontSizeIsInPixels(GetFontSizeIsInPixels());
        }

        xpf::Color backgroundColor;
        xpf::Color borderColor;
        xpf::Color foregroundColor;
        if (m_btnstate == ButtonState::Default)
        {
            backgroundColor = m_Background_Default;
            borderColor = m_Border_Default;
            foregroundColor = m_Foreground_Default;
        }
        else
        {
            if (m_btnstate & ButtonState::Pressed)
            {
                backgroundColor = m_Background_Pressed;
                borderColor = m_Border_Pressed;
                foregroundColor = m_Foreground_Pressed;
            }

            if (m_btnstate & ButtonState::Hover)
            {
                backgroundColor = backgroundColor.is_transparent() ? m_Background_Hover : backgroundColor.mix(m_Background_Hover, .5);
                borderColor = borderColor.is_transparent() ? m_Border_Hover : borderColor.mix(m_Border_Hover, .5);
                foregroundColor = foregroundColor.is_transparent() ? m_Foreground_Hover : foregroundColor.mix(m_Foreground_Hover, .5);
            }

            if (m_btnstate & ButtonState::Focused)
            {
                backgroundColor = backgroundColor.is_transparent() ? m_Background_Focus : backgroundColor.mix(m_Background_Focus, .5);
                borderColor = borderColor.is_transparent() ? m_Border_Focus : borderColor.mix(m_Border_Focus, .5);
                foregroundColor = foregroundColor.is_transparent() ? m_Foreground_Focus : foregroundColor.mix(m_Foreground_Focus, .5);
            }
        }

        m_Background.SetDefaultValue(backgroundColor);
        m_Border.SetDefaultValue(borderColor);
        if (m_spChild != nullptr)
            m_spChild->GetForegroundProperty().SetDefaultValue(foregroundColor);

        m_spChild->UpdateVisuals(renderer);

        ItemPanel::OnUpdateVisuals(renderer);
    }

    virtual void OnDraw(IRenderer& renderer) override
    {
        StateManager();
        ItemPanel::OnDraw(renderer);
    }

protected:
    enum class MouseState
    {
        waiting_mouse_is_inside, // user moves into button
        waiting_mouse_is_inside_left_down, // user clicks down when inside the button
        waiting_mouse_is_inside_left_up,   // user releases button
    };

    ButtonState m_btnstate = ButtonState::Default;
    MouseState m_state = MouseState::waiting_mouse_is_inside;

    static inline KeyboardState m_spaceKeyState = KeyboardState::WaitingForKeyDown;
    void StateManager()
    {
        ButtonState newState = ButtonState::Default;
        if (IsInFocus())
        {
            newState = ButtonState::Focused;

            bool spaceKeyPressed = false;
            switch (m_spaceKeyState)
            {
                case KeyboardState::WaitingForKeyDown:
                    if (InputService::IsKeyDown(KeyCode::Space))
                    {
                        m_spaceKeyState = KeyboardState::WaitingForKeyUp;
                        newState |= ButtonState::Pressed;
                        break;
                    }
                    break;
                case KeyboardState::WaitingForKeyUp:
                    if (InputService::IsKeyUp(KeyCode::Space))
                    {
                        m_spaceKeyState = KeyboardState::WaitingForKeyDown;
                        switch (m_ButtonType)
                        {
                            case ButtonType::Button:
                                OnClick();
                                break;
                            case ButtonType::Toggle:
                                newState |= ButtonState::Pressed;
                                m_ToggledOn.Set(!m_ToggledOn);
                                OnToggle(m_ToggledOn);
                                break;
                        }
                    }
                    break;
            }
        }

        switch (m_state)
        {
            case MouseState::waiting_mouse_is_inside:
                if (IsMouseInside())
                {
                    if (InputService::IsMouseButtonUp(xpf::MouseButton::Left))
                        m_state = MouseState::waiting_mouse_is_inside_left_down;

                    newState |= ButtonState::Hover;
                }
                break;
            case MouseState::waiting_mouse_is_inside_left_down:
                if (IsMouseInside())
                {
                    if (InputService::IsMouseButtonDown(xpf::MouseButton::Left))
                    {
                        m_state = MouseState::waiting_mouse_is_inside_left_up;
                        newState |= ButtonState::Pressed;
                        Focus();
                        InvalidateVisuals();
                        OnMouseDown();
                    }
                    else
                    {
                        newState |= ButtonState::Hover;
                        OnHover();
                    }
                }
                else
                {
                    m_state = MouseState::waiting_mouse_is_inside;
                }
                break;
            case MouseState::waiting_mouse_is_inside_left_up:
                if (IsMouseInside())
                {
                    if (InputService::IsMouseButtonUp(xpf::MouseButton::Left))
                    {
                        m_state = MouseState::waiting_mouse_is_inside;
                        switch (m_ButtonType)
                        {
                            case ButtonType::Button:
                                OnMouseUp();
                                OnClick();
                                break;
                            case ButtonType::Toggle:
                                newState |= ButtonState::Pressed;
                                OnMouseUp();
                                m_ToggledOn.Set(!m_ToggledOn);
                                OnToggle(m_ToggledOn);
                                break;
                        }
                    }
                    else
                    {
                        newState |= ButtonState::Pressed;
                        OnHover();
                    }
                }
                else
                {
                    if (InputService::IsMouseButtonUp(xpf::MouseButton::Left))
                    {
                        OnMouseUp();
                        m_state = MouseState::waiting_mouse_is_inside;
                    }
                }
                break;
        }

        if (m_btnstate != newState)
        {
            m_btnstate = newState;
            InvalidateVisuals();
        }
    }

#pragma region events
protected:
    std::function<void(Button&)> m_on_click = [](Button&) { };
    std::function<void(Button&)> m_on_hover = [](Button&) { };
    std::function<void(Button&)> m_on_mouse_down = [](Button&) { };
    std::function<void(Button&)> m_on_mouse_up = [](Button&) { };
    std::function<void(Button&, bool)> m_on_toggle = [](Button&, bool) { };

protected:
    virtual void OnClick() { m_on_click(*this); }
    virtual void OnHover() { m_on_hover(*this); }
    virtual void OnMouseDown() { m_on_mouse_down(*this); }
    virtual void OnMouseUp() { m_on_mouse_up(*this); }
    virtual void OnToggle(bool toggled_on) { m_on_toggle(*this, toggled_on); }

public:
    Button& SetOnClick(std::function<void(Button&)>&& fn) { m_on_click = std::move(fn); return *this; }
    Button& SetOnHover(std::function<void(Button&)>&& fn) { m_on_hover = std::move(fn); return *this; }
    Button& SetOnMouseDown(std::function<void(Button&)>&& fn) { m_on_mouse_down = std::move(fn); return *this; }
    Button& SetOnMouseUp(std::function<void(Button&)>&& fn) { m_on_mouse_up = std::move(fn); return *this; }
    Button& SetOnToggle(std::function<void(Button&, bool)>&& fn) { m_on_toggle = std::move(fn); return *this; }
#pragma endregion
};

} // xpf