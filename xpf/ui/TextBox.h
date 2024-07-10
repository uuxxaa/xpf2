#pragma once
#include <xpf/renderer/common/FormattedText.h>
#include <xpf/ui/UIElement.h>
#include <xpf/core/stringex.h>
#include <xpf/windows/ClipboardService.h>

namespace xpf {

class TextBox : public UIElement
{
protected:
    DECLARE_PROPERTY(TextBox, std::string, Text, std::string(), Invalidates::ParentLayout);
    DECLARE_PROPERTY(TextBox, std::string, FontName, std::string(), Invalidates::ParentLayout);
    DECLARE_PROPERTY(TextBox, int16_t, FontSize, int16_t(15), Invalidates::ParentLayout);
    DECLARE_PROPERTY(TextBox, bool, FontSizeIsInPixels, false, Invalidates::ParentLayout);
    DECLARE_PROPERTY(TextBox, bool, MultiLine, false, Invalidates::ParentLayout);
    ThemedColor m_caret_color_on = {"Caret_Color_On"};
    ThemedColor m_caret_color_off = {"Caret_Color_Off"};

protected:
    struct charinfo
    {
        v2_t pos;
        char32_t ch32;
    };

    std::vector<charinfo> m_chars;
    FormattedText m_formattedText;
    rectf_t m_textRect;
    RenderBatch m_renderCommands;
    float m_newline_width = 0;
    float m_xoffset = 0;
    float m_yoffset = 0;
    float m_cursor_width = 2.0f;
    float m_line_height = 0;

    struct Selection
    {
        int32_t start = -1;
        int32_t end = -1;
        Selection& operator=(int32_t v) { start = end = v; return *this; }
        Selection& operator-=(int32_t v){ start = end = end - v; return *this; }
        Selection& operator+=(int32_t v){ start = end = end + v; return *this; }
        bool operator<(int32_t v) const { return start < v; }
        bool operator<=(int32_t v) const { return start <= v; }
        bool operator>(int32_t v) const { return start > v; }
        bool operator>=(int32_t v) const { return start >= v; }
        bool operator==(Selection other) const { return start == other.start && end == other.end; }
        bool operator!=(Selection other) const { return start != other.start && end != other.end; }
    };
    Selection m_cursor_position;

    struct history_entry
    {
        std::string txt;
        Selection selection;
    };

    std::vector<history_entry> m_history;
    size_t m_historyPosition = 0;

public:
    TextBox(UIElementType type = UIElementType::TextBox)
        : UIElement(type)
    {
        m_acceptsFocus = true;
        m_clippingEnabled = true;

        m_Foreground.SetThemeId("TextBox_Foreground");
        m_Background.SetThemeId("TextBox_Background");
        m_Margin.Set(4);
        m_Padding.Set(4);
    }

    v2_t GetBounds() const { return m_formattedText.GetBounds(); }

    virtual v2_t OnMeasure(v2_t size) override
    {
        Typeface typeface;
        typeface.name = m_FontName;
        typeface.size = m_FontSize;
        if (m_FontSizeIsInPixels)
            typeface.renderOptions = FontRenderOptions::SizeInPixels;

        if (!m_MultiLine)
        {
            auto iter = m_Text.Get().find('\n');
            if (iter != std::string::npos)
            {
                std::string txt = m_Text.Get();
                txt.erase(iter, std::string::npos);
                m_Text.Set(txt);
            }
        }

        m_formattedText.SetText(m_Text.Get());
        m_formattedText.SetFont(typeface);
        m_formattedText.BuildGeometry();
        m_line_height = m_formattedText.GetLineHeight() + m_formattedText.GetAdvanceNewlineY();

        m_chars.clear();
        float xpos = 0;
        float ypos = 0;
        for (const auto& line : m_formattedText.GetLines())
        {
            xpos = 0;
            for (const auto& g : line.glyphs)
            {
                m_chars.push_back({{xpos,ypos}, g.ch});
                xpos += g.advance_x;
                if (g.ch == '\n')
                {
                    m_newline_width = g.advance_x;
                    ypos += m_line_height;
                }
            }
        }
        m_chars.push_back({{xpos,ypos}, 0});

        if (m_cursor_position < 0)
        {
            m_cursor_position = m_chars.size() - 1;
            m_history.push_back({m_Text.Get(), m_cursor_position});
        }
        return m_formattedText.GetBounds();
    }

    virtual void OnArrange(v2_t insideSize) override
    {
        m_formattedText.BuildGeometry();
        v2_t bounds = m_formattedText.GetBounds();
        m_textRect = rectf_t{m_insideRect.x, m_insideRect.y, insideSize.x, insideSize.y};
        if (!m_MultiLine)
            m_textRect.y += (m_textRect.h - bounds.y) * .5;

        return UIElement::OnArrange(insideSize);
    }

    virtual void OnUpdateVisuals(IRenderer& renderer) override
    {
        auto r = renderer.CreateCommandBuilder();
        r.DrawText(m_textRect.x, m_textRect.y, m_formattedText, m_Foreground);
        m_renderCommands = r.Build();
    }

    inline bool IsWhitespace(int32_t i) const { auto ch32 = m_chars[i].ch32; return ch32 == '\n' || ch32 == '\t' || ch32 == ' ';}

    virtual void OnDraw(IRenderer& renderer) override
    {
#if TEXTBOX_DEBUG
        {
            float dbgx = m_textRect.right() - 200, dbgy = m_textRect.x; int32_t dbgfs = 10;

            dbgy += dbgfs;
            switch (m_mouse_state)
            {
                case MouseState::Reset: renderer.DrawText("Reset", dbgx, dbgy, "", dbgfs, xpf::Colors::XpfRed); break;
                case MouseState::LookingForClickStart: renderer.DrawText("LookingForClickStart", dbgx, dbgy, "", dbgfs, xpf::Colors::XpfRed); break;
                case MouseState::LookingForClickEnd: renderer.DrawText("LookingForClickEnd", dbgx, dbgy, "", dbgfs, xpf::Colors::XpfRed); break;
                case MouseState::LookingForDoubleClickStart: renderer.DrawText("LookingForDoubleClickStart", dbgx, dbgy, "", dbgfs, xpf::Colors::XpfRed); break;
                case MouseState::LookingForDoubleClickEnd: renderer.DrawText("LookingForDoubleClickEnd", dbgx, dbgy, "", dbgfs, xpf::Colors::XpfRed); break;
            }
        }
#endif

        float mwidth = (m_textRect.w - m_cursor_width);
        if (m_textRect.w >= m_formattedText.GetBounds().x)
        {
            m_xoffset = 0;
        }
        else if (m_chars[m_cursor_position.end].pos.x > m_xoffset + mwidth)
        {
            m_xoffset = std::min(m_chars[m_cursor_position.end].pos.x - mwidth, m_formattedText.GetBounds().x);
        }
        else if (m_chars[m_cursor_position.end].pos.x < m_xoffset)
        {
            m_xoffset = std::max(0.0f, m_chars[m_cursor_position.end].pos.x - m_xoffset);
        }

        if (m_MultiLine)
        {
            float mheight = (m_textRect.h);
            if (m_textRect.h >= m_formattedText.GetBounds().y)
            {
                m_yoffset = 0;
            }
            else if ((m_chars[m_cursor_position.end].pos.y + m_line_height) > (m_yoffset + mheight))
            {
                m_yoffset = std::min(m_chars[m_cursor_position.end].pos.y + m_line_height - mheight, m_formattedText.GetBounds().y - mheight);
            }
            else if (m_chars[m_cursor_position.end].pos.y < m_yoffset)
            {
                m_yoffset = std::max(0.0f, m_chars[m_cursor_position.end].pos.y);
            }

            if (m_yoffset < m_line_height)
                m_yoffset = 0;
        }

        if (IsMouseInside() && InputService::IsMouseButtonDown(xpf::MouseButton::Left))
            Focus();

        // Scroll to where the cursor is visible
        auto scope = renderer.TranslateTransfrom(-m_xoffset, -m_yoffset);

        // Draw selection
        if (m_cursor_position.start != m_cursor_position.end)
        {
            int32_t startpos = std::min(m_cursor_position.start, m_cursor_position.end);
            int32_t endpos = std::max(m_cursor_position.start, m_cursor_position.end);
            v2_t pt;
            for (size_t i = startpos; i <= endpos; i++)
            {
                if (i == startpos)
                    pt = m_chars[i].pos;

                if (i == endpos)
                {
                    rectf_t rect = rectf_t::from_points(
                        pt.x, pt.y,
                        m_chars[i].pos.x,
                        m_chars[i].pos.y + m_formattedText.GetLineHeight()).move(m_textRect.left(), m_textRect.top());
                    renderer.DrawRectangle(rect, xpf::Colors::LightYellow);
                }
                else if (m_chars[i].ch32 == '\n')
                {
                    rectf_t rect = rectf_t::from_points(
                        pt.x, pt.y,
                        m_chars[i].pos.x + m_newline_width,
                        pt.y + m_formattedText.GetLineHeight()).move(m_textRect.left(), m_textRect.top());
                    renderer.DrawRectangle(rect, xpf::Colors::LightYellow);
                    startpos = i + 1;
                }
            }
        }

        renderer.EnqueueCommands(m_renderCommands);

        if (IsInFocus() && m_cursor_position >= -1)
        {
            if (m_cursor_position.start > m_chars.size() - 1)
                m_cursor_position.start = m_chars.size() - 1;
            if (m_cursor_position.end > m_chars.size() - 1)
                m_cursor_position.end = m_chars.size() - 1;
            bool forceDrawCursor = ProcessKeyboard(renderer);
            if (!forceDrawCursor)
                forceDrawCursor = ProcessMouse(renderer);

            static time_t s_next_time = 0; static bool s_cursor_on = false;
            if (forceDrawCursor)
            {
                s_next_time = Time::GetTime() + 1.0;
                s_cursor_on = true;
            }
            else if (Time::GetTime() > s_next_time)
            {
                s_cursor_on = !s_cursor_on;
                s_next_time = Time::GetTime() + (s_cursor_on ? 1.0 : 0.3);
            }

            if (s_cursor_on && m_cursor_position.end < m_chars.size())
            {
                v2_t cursorcoord = m_chars[m_cursor_position.end].pos;
                float xpos = m_textRect.left() + cursorcoord.x + 1;
                float ypos = m_textRect.top() + cursorcoord.y;
                renderer.DrawLine(
                    xpos, ypos,
                    xpos, ypos + m_formattedText.GetLineHeight(), m_cursor_width,
                    s_cursor_on ? m_caret_color_on.Get() : m_caret_color_off.Get());
            }
        }
    }

    enum class MouseState
    {
        Reset,
        LookingForClickStart, // user clicks down when inside the button
        LookingForClickEnd,   // user releases button
        LookingForDoubleClickStart,
        LookingForDoubleClickEnd,
    };

    MouseState m_mouse_state = MouseState::Reset;
    int32_t m_position_LeftButtonDown = -1;
    int32_t m_position_LeftButtonUp = -1;
    Selection m_double_clicked_word = {-1,-1};
    time_t m_time_LeftButtonDown = 0;
    time_t m_time_LeftButtonUp = 0;
    static inline seconds_t m_double_click_time = 0.5f;

    int32_t FindCursorPosition(v2_t mousePos) const
    {
        int32_t newPosition = m_chars.size() - 1;
        for (int32_t i = 0; i < m_chars.size() - 1; i++)
        {
            const v2_t& chpos = m_chars[i].pos;
            if (mousePos.y < chpos.y + m_formattedText.GetLineHeight())
            {
                const auto& nextEntry = m_chars[i + 1];
                float x = chpos.x + (nextEntry.pos.x - chpos.x) * .5;
                if (mousePos.x < x)
                {
                    newPosition = i;
                    break;
                }
                else if (m_chars[i].ch32 == '\n')
                {
                    newPosition = i;
                    break;
                }
            }
        }

        return newPosition;
    }

    bool ProcessMouse(IRenderer& renderer)
    {
        bool updateCursor = false;
        v2_t mousePos = GetMousePosition(renderer);
        mousePos.x -= m_textRect.left();
        mousePos.y -= m_textRect.top();

        const bool leftIsDown = InputService::IsMouseButtonDown(xpf::MouseButton::Left);
        const bool isMouseInside = IsMouseInside();

        switch (m_mouse_state)
        {
            case MouseState::Reset:
                m_position_LeftButtonDown = m_position_LeftButtonUp = -1;
                m_time_LeftButtonDown = m_time_LeftButtonUp = 0;
                m_double_clicked_word = {-1,-1};
                m_mouse_state = MouseState::LookingForClickStart;
                break;
            case MouseState::LookingForClickStart:
                if (isMouseInside && leftIsDown)
                {
                    m_cursor_position = FindCursorPosition(mousePos);
                    m_position_LeftButtonDown = m_cursor_position.end;
                    m_time_LeftButtonDown = Time::GetTime();
                    m_time_LeftButtonUp = 0;
                    m_position_LeftButtonUp = -1;
                    m_mouse_state = MouseState::LookingForClickEnd;
                    updateCursor = true;
                }
                break;
            case MouseState::LookingForClickEnd:
                updateCursor = true;
                m_cursor_position.end = FindCursorPosition(mousePos);
                if (!leftIsDown)
                {
                    if (Time::GetTime() - m_time_LeftButtonDown < m_double_click_time && m_cursor_position.end == m_position_LeftButtonDown)
                    {
                        m_mouse_state = MouseState::LookingForDoubleClickStart;
                        m_position_LeftButtonUp = m_cursor_position.end;
                        m_time_LeftButtonUp = Time::GetTime();
                    }
                    else
                    {
                        OnMouseLeftClick();
                        m_mouse_state = MouseState::Reset;
                    }
                }
                break;
            case MouseState::LookingForDoubleClickStart:
                if (Time::GetTime() - m_time_LeftButtonUp > m_double_click_time)
                {
                    m_cursor_position.end = FindCursorPosition(mousePos);
                    OnMouseLeftClick();
                    m_mouse_state  = MouseState::Reset; // convert this to regular click
                }
                else if (leftIsDown)
                {
                    m_double_clicked_word = m_cursor_position = SelectWord(mousePos);
                    m_mouse_state = MouseState::LookingForDoubleClickEnd;
                }

                break;
            case MouseState::LookingForDoubleClickEnd:
                if (leftIsDown)
                {
                    int32_t pos = FindCursorPosition(mousePos);
                    m_cursor_position = m_double_clicked_word;
                    if (pos < m_double_clicked_word.start)
                    {
                        m_cursor_position.start = pos;
                    }
                    else if (pos > m_double_clicked_word.end)
                    {
                        m_cursor_position.end = pos;
                    }
                }
                else
                {
                    m_mouse_state = MouseState::Reset;
                }
                break;
        }

        return updateCursor;
    }

    void OnMouseLeftClick() { }
    void OnMouseLeftDoubleClick_SelectWord()
    {
        int32_t startpos = std::min(m_cursor_position.start, m_cursor_position.end);
        int32_t endpos = std::max(m_cursor_position.start, m_cursor_position.end);
        m_cursor_position = SelectWord(startpos, endpos);
    }

    Selection SelectWord(v2_t mousePos)
    {
        int startPos = FindCursorPosition(mousePos);
        return SelectWord(startPos, startPos);
    }

    Selection SelectWord(int32_t startpos, int32_t endpos)
    {
        bool isws = IsWhitespace(startpos);
        while (startpos > 0 && isws == IsWhitespace(startpos - 1))
        {
            startpos--;
        }

        isws = IsWhitespace(endpos);
        while (isws == IsWhitespace(endpos) && endpos < m_chars.size() - 1)
        {
            endpos++;
        }

        return {startpos, endpos};
    }

    bool ProcessKeyboard(IRenderer& renderer)
    {
        bool shiftDown = InputService::IsKeyDown(KeyCode::LeftShift) || InputService::IsKeyDown(KeyCode::RightShift);
        bool altDown = InputService::IsKeyDown(KeyCode::LeftAlt) || InputService::IsKeyDown(KeyCode::RightAlt);
        bool ctrlDown =
        #if defined(PLATFORM_APPLE)
            InputService::IsKeyDown(KeyCode::LeftControl) || InputService::IsKeyDown(KeyCode::RightControl) ||
            InputService::IsKeyDown(KeyCode::LeftSuper) || InputService::IsKeyDown(KeyCode::RightSuper);
        #else
            InputService::IsKeyDown(KeyCode::LeftControl) || InputService::IsKeyDown(KeyCode::RightControl);
        #endif

        const bool c_pressed = s_c.IsKeyPressed();
        const bool x_pressed = s_x.IsKeyPressed();

        // MACOS
        // CTRL+LEFT  => start of line
        // CTRL+RIGHT => end of line
        // ALT+LEFT   => word left
        // ALT+RIGHT  => word right
        // SHIFT      => selection

        //
        //                 |a      a|    b|   c|
        //                 ^        ^     ^    ^
        //                 |        |     |    |
        // cursor pos      0        1     2    3
        //
        if (ctrlDown && s_a.IsKeyPressed()) // CTRL+A - SELECT ALL
        {
            m_cursor_position.start = 0;
            m_cursor_position.end = m_chars.size() - 1;
        }
        else if (ctrlDown && (c_pressed || x_pressed)) // Ctrl+C - copy to clipboard OR Ctrl+X to cut to clipboard
        {
            xpf::stringex str;
            int32_t startpos = std::min(m_cursor_position.start, m_cursor_position.end);
            int32_t endpos = std::max(m_cursor_position.start, m_cursor_position.end);
            for (size_t i = startpos; i < endpos; i++)
                str.append32(m_chars[i].ch32);
            
            if (x_pressed)
            {
                m_cursor_position = ReplaceText(
                    m_cursor_position.end == m_cursor_position.start ? m_cursor_position.start - 1 : m_cursor_position.start,
                    m_cursor_position.end,
                    {});
            }

            ClipboardService::SetString(str);
        }
        else if (ctrlDown && s_v.IsKeyPressed()) // Ctrl+V - paste from clipboard
        {
            std::vector<uint32_t> chars;
            xpf::stringex text = ClipboardService::GetString();
            size_t i = 0;
            size_t length = text.length();
            while (i < length)
            {
                const uint32_t ch32 = stringex::utf8_to_utf32(text, i);
                if (ch32 == 0) [[unlikely]]
                    break;
                chars.push_back(ch32);
            }

            m_cursor_position = ReplaceText(
                m_cursor_position.start,
                m_cursor_position.end,
                chars);
        }
        else if (ctrlDown && s_z.IsKeyPressed()) // UNDO / REDO
        {
            if (shiftDown) // REDO
            {
                if (m_historyPosition + 1 < m_history.size())
                {
                    m_historyPosition++;
                    SetText(m_history[m_historyPosition].txt);
                    m_cursor_position = m_history[m_historyPosition].selection;
                }
            }
            else if (m_historyPosition > 0)
            {
                m_historyPosition--;
                SetText(m_history[m_historyPosition].txt);
                m_cursor_position = m_history[m_historyPosition].selection;
            }
        }
        else if (s_backspace.IsKeyPressed()) // BACKSPACE / DELETE
        {
            m_cursor_position = ReplaceText(
                (m_cursor_position.end == m_cursor_position.start && m_cursor_position.start > 0) ? m_cursor_position.start - 1 : m_cursor_position.start,
                m_cursor_position.end,
                {});
        }
        else if (s_left.IsKeyPressed()) // LEFT
        {
            if (ctrlDown) // LineStart
            {
                Selection newPos;
                newPos.end = 0;
                newPos.start = shiftDown ? m_cursor_position.start : newPos.end;

                for (int32_t i = m_cursor_position.end; i-- > 0;)
                {
                    if (m_chars[i].ch32 == '\n')
                    {
                        if (shiftDown)
                            newPos.end = i + 1;
                        else
                            newPos.start = newPos.end = i + 1;
                        break;
                    }
                }

                m_cursor_position = newPos;
            }
            else if (altDown) // PREV WORD
            {
                Selection newPos;
                newPos.end = 0;
                newPos.start = shiftDown ? m_cursor_position.start : newPos.end;

                bool startingWithWS = IsWhitespace(m_cursor_position.end - 1);
                for (int32_t i = m_cursor_position.end - 1; i-- > 0;)
                {
                    if (IsWhitespace(i) != startingWithWS)
                    {
                        if (shiftDown)
                            newPos.end = i + 1;
                        else
                            newPos.start = newPos.end = i + 1;
                        break;
                    }
                }

                m_cursor_position = newPos;
            }
            else if (m_cursor_position.end > 0)
            {
                if (shiftDown)
                    m_cursor_position.end--;
                else if (m_cursor_position.start != m_cursor_position.end)
                {
                    if (m_cursor_position.end < m_cursor_position.start)
                        m_cursor_position -= 1;
                    else
                        std::swap(m_cursor_position.end, m_cursor_position.start);
                }
                else
                    m_cursor_position -= 1;
            }
            else if (!shiftDown)
            {
                m_cursor_position.start = m_cursor_position.end;
            }
        }
        else if (s_right.IsKeyPressed()) // RIGHT
        {
            if (ctrlDown) // LineEnd
            {
                Selection newPos;
                newPos.end = m_chars.size() - 1;
                newPos.start = shiftDown ? m_cursor_position.start : newPos.end;

                for (int32_t i = m_cursor_position.end; i < m_chars.size(); i++)
                {
                    if (m_chars[i].ch32 == '\n')
                    {
                        if (shiftDown)
                            newPos.end = i;
                        else
                            newPos.start = newPos.end = i;
                        break;
                    }
                }

                m_cursor_position = newPos;
            }
            else if (altDown)
            {
                Selection newPos;
                newPos.end = m_chars.size() - 1;
                newPos.start = shiftDown ? m_cursor_position.start : newPos.end;

                bool startingWithWS = IsWhitespace(m_cursor_position.end);
                for (int32_t i = m_cursor_position.end; i < m_chars.size(); i++)
                {
                    if (IsWhitespace(i) != startingWithWS)
                    {
                        if (shiftDown)
                            newPos.end = i;
                        else
                            newPos.start = newPos.end = i;
                        break;
                    }
                }

                m_cursor_position = newPos;
            }
            else if (m_cursor_position.end < m_chars.size() - 1)
            {
                if (shiftDown)
                    m_cursor_position.end++;
                else if (m_cursor_position.start != m_cursor_position.end)
                {
                    if (m_cursor_position.end > m_cursor_position.start)
                        m_cursor_position = m_cursor_position.end + 1;
                    else
                        std::swap(m_cursor_position.end, m_cursor_position.start);
                }
                else
                    m_cursor_position += 1;
            }
            else if (!shiftDown)
            {
                m_cursor_position.start = m_cursor_position.end;
            }
        }
        else if (s_enter.IsKeyPressed() && m_MultiLine) // ENTER
        {
            if (m_cursor_position >= 0)
                m_cursor_position = ReplaceText(
                    m_cursor_position.start,
                    m_cursor_position.end,
                    {'\n'});
        }
        else if (s_up.IsKeyPressed()) // UP
        {
            bool lookingForPrevNewline = true;
            auto endXY = m_chars[m_cursor_position.end].pos;
            int32_t newEndPos = 0;
            for (int32_t i = m_cursor_position.end; i-- > 0;)
            {
                if (lookingForPrevNewline && m_chars[i].ch32 == '\n')
                {
                    lookingForPrevNewline = false;
                }
                else if (!lookingForPrevNewline && m_chars[i].pos.x <= endXY.x)
                {
                    newEndPos = i;
                    break;
                }
            }

            if (shiftDown)
                m_cursor_position.end = newEndPos;
            else
                m_cursor_position.start = m_cursor_position.end = newEndPos;
        }
        else if (s_down.IsKeyPressed()) // DOWN
        {
            bool lookingForPrevNewline = true;
            auto endXY = m_chars[m_cursor_position.end].pos;
            int32_t newEndPos = m_chars.size() - 1;
            for (int32_t i = m_cursor_position.end; i < m_chars.size() - 1; i++)
            {
                if (lookingForPrevNewline && m_chars[i].ch32 == '\n')
                {
                    lookingForPrevNewline = false;
                }
                else if (!lookingForPrevNewline && m_chars[i].pos.x >= endXY.x)
                {
                    newEndPos = i;
                    break;
                }
            }

            if (shiftDown)
                m_cursor_position.end = newEndPos;
            else
                m_cursor_position.start = m_cursor_position.end = newEndPos;
        }
        else // CHARS TYPED
        {
            const auto& charsPressed = InputService::GetCharsPressed();
            if (!charsPressed.empty())
            {
                m_cursor_position = ReplaceText(
                    m_cursor_position.start,
                    m_cursor_position.end,
                    charsPressed);
            }
            else
            {
                return false;
            }
        }


        if (m_cursor_position.end < 0)
            return true;

        return true;
    }

    int32_t ReplaceText(int32_t start, int32_t end, const std::vector<uint32_t>& ch32)
    {
        int32_t startpos = std::min(start, end);
        int32_t endpos = std::max(start, end);

        xpf::stringex txt;
        for (int32_t i = 0; i < startpos; i++)
            txt.append32(m_chars[i].ch32);

        for (const auto& ch32 : ch32)
            txt.append32(ch32);

        for (int32_t i = endpos; i < int32_t(m_chars.size()) - 1; i++)
            txt.append32(m_chars[i].ch32);

        SetText(txt);

        m_history.erase(m_history.cbegin() + m_historyPosition + 1, m_history.cend());

        Selection newSelection;
        newSelection = startpos + ch32.size();
        m_history.push_back(history_entry{GetText(), newSelection});
        m_historyPosition = m_history.size() - 1;

        return startpos + ch32.size();
    }
};

} // xpf