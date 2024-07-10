#pragma once
#include <array>

namespace xpf {

typedef double time_t;
typedef double seconds_t;

class Time
{
protected:
    static inline uint32_t               m_frame_count = 0;
    static inline time_t                 m_update_time = 0;
    static inline float                  m_average_delta_time = 0;
    static inline float                  m_delta_time = 0;
    static inline std::array<float, 512> m_update_times_history;

public:
    Time() = delete;

    static uint32_t GetFrameCount() { return m_frame_count; }
    static time_t GetRealTime();
    static time_t GetTime() { return m_update_time; }
    static time_t GetDeltaTime() { return m_delta_time; }
    static time_t GetAverageDeltaTime() { return m_average_delta_time; }
    static uint16_t GetFps();

protected:
    static void InitializeTime();
    static void UpdateTime();
};

} // xpf