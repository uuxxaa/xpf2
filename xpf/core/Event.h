#pragma once
#include <condition_variable>
#include <mutex>

namespace xpf {

class EventBase {
protected:
    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_triggered = false;
    bool m_manualReset = false;

public:
    static const uint32_t Infinite = 0xFFFFFFFF; // Infinite timeout

public:
    EventBase(bool manualReset, bool triggered) noexcept;
    EventBase(const EventBase&) = delete;
    EventBase(EventBase&&) = delete;
    EventBase& operator=(const EventBase&) = delete;
    EventBase& operator=(EventBase&&) = delete;

    void Fire() noexcept;
    void Reset() noexcept;
    bool WaitInfinite() noexcept;
    bool Wait(uint32_t msecTimeOut) noexcept;
};

class AutoResetEvent : public EventBase {
public:
    AutoResetEvent()
        : EventBase(/*manualReset=*/false, /*triggered=*/false)
    {}
};

} // xpf
