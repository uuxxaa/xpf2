#include <chrono>
#include <core/Event.h>

namespace xpf {

EventBase::EventBase(bool manualReset, bool triggered) noexcept
    : m_triggered(triggered)
    , m_manualReset(manualReset)
{}

void EventBase::Fire() noexcept
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_triggered = true;

    // Manual unlocking is done before notifying, to avoid waking up
    // the waiting thread only to block again.
    m_cv.notify_all();
}

void EventBase::Reset() noexcept
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_triggered = false;
}

bool EventBase::WaitInfinite() noexcept
{
    return Wait(EventBase::Infinite);
}

bool EventBase::Wait(uint32_t msecTimeOut) noexcept
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto predicateFn = [this]() noexcept
    {
        if (m_triggered) {
            if (!m_manualReset)
                m_triggered = false;
            return true;
        }

        return false;
    };

    if (msecTimeOut == EventBase::Infinite)
    {
        m_cv.wait(lock, std::move(predicateFn));
        return true;
    }

    return m_cv.wait_for(lock, std::chrono::milliseconds(msecTimeOut), std::move(predicateFn));
}

} // xpf
