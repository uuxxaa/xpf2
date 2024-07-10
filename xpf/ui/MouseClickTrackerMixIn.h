#pragma once

namespace xpf {

template<typename tbase>
class MouseClickTrackerMixIn : public tbase
{
private:
    enum class MouseState
    {
        waiting_mouse_is_inside, // user moves into
        waiting_mouse_is_inside_left_down, // user clicks down when inside the button
        waiting_mouse_is_inside_left_up,   // user releases button
    };

    MouseState m_state = MouseState::waiting_mouse_is_inside;

protected:
    MouseClickTrackerMixIn(UIElementType type) : tbase(type) { }

    virtual void Outside() { }
    virtual void Hover() { }
    virtual void TrackingStarted() { }
    virtual void Tracking() { }
    virtual void TrackingStopped() { }
    virtual void MouseLeftButtonDown() { }
    virtual void MouseLeftButtonUp() { }

    bool TrackMouse(bool isMouseInside)
    {
        if (tbase::IsMouseCaptured()) return false;

        switch (m_state)
        {
            case MouseState::waiting_mouse_is_inside:
                if (isMouseInside)
                {
                    if (InputService::IsMouseButtonUp(xpf::MouseButton::Left))
                        m_state = MouseState::waiting_mouse_is_inside_left_down;

                    Hover();
                }
                else
                {
                    Outside();
                }
                break;
            case MouseState::waiting_mouse_is_inside_left_down:
                if (isMouseInside)
                {
                    if (InputService::IsMouseButtonDown(xpf::MouseButton::Left))
                    {
                        m_state = MouseState::waiting_mouse_is_inside_left_up;
                        TrackingStarted();
                        MouseLeftButtonDown();
                        tbase::CaptureMouse();
                    }
                }
                else
                {
                    m_state = MouseState::waiting_mouse_is_inside;
                }
                break;
            case MouseState::waiting_mouse_is_inside_left_up:
                if (isMouseInside)
                {
                    if (InputService::IsMouseButtonUp(xpf::MouseButton::Left))
                    {
                        tbase::ReleaseMouse();
                        MouseLeftButtonUp();
                        TrackingStopped();
                        m_state = MouseState::waiting_mouse_is_inside;
                    }
                    else
                    {
                        Tracking();
                    }
                }
                else
                {
                    if (InputService::IsMouseButtonUp(xpf::MouseButton::Left))
                    {
                        tbase::ReleaseMouse();
                        MouseLeftButtonUp();
                        TrackingStopped();
                        m_state = MouseState::waiting_mouse_is_inside;
                    }
                }
                break;
        }

        return true;
    }
};

} // xpf 