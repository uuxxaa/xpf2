#include "Time.h"
#include <GLFW/glfw3.h>

namespace xpf {

/*static*/ double Time::GetRealTime() { return glfwGetTime(); }

/*static*/ uint16_t Time::GetFps()
{
    if (m_frame_count < m_update_times_history.size())
        return 0;
    return m_average_delta_time < 0.00001f
        ? 0
        : uint16_t(roundf(1.0f / m_average_delta_time));
}

/*static*/ void Time::InitializeTime()
{
    std::fill(m_update_times_history.begin(), m_update_times_history.end(), 0);
}

/*static*/ void Time::UpdateTime()
{
    const double now = GetRealTime();
    m_delta_time = float(now - m_update_time);
    m_update_time = now;

    const uint32_t updateIndex = m_frame_count % m_update_times_history.size();
    const float oldDeltaTime = m_update_times_history[updateIndex];
    m_update_times_history[updateIndex] = m_delta_time;
    m_average_delta_time += (m_delta_time - oldDeltaTime) / m_update_times_history.size();

    m_frame_count++;
}

} // PCPP