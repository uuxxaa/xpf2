#pragma once
#include <limits>
#include <mutex>
#include <thread>
#include <unordered_set>
#include <math/xpfmath.h>
#include <core/StackGuard.h>
#include <core/Time.h>

namespace xpf {

struct TweenBase {
    virtual void OnTimerTick() = 0;
};

class TweenService {
protected:
    static inline bool m_started = false;
    static inline std::atomic<bool> m_shutdown = false;
    static inline std::unordered_set<TweenBase*> m_tweens;
    static inline std::mutex m_tween_mutex;
    static inline std::thread m_timerThread;

public:
    static void Shutdown() {
        if (!m_started) return;
        m_shutdown = true;
        m_timerThread.join();
    }

    static void Start() {
        m_started = true;
        m_timerThread = std::thread([]()
        {
            constexpr uint32_t c_interval = 32; // 16 milliseconds
            while (!m_shutdown) {
                auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(c_interval);
                {
                    std::lock_guard<std::mutex> guard(m_tween_mutex);
                    for (TweenBase* pt : m_tweens) {
                        pt->OnTimerTick();
                        if (m_shutdown)
                            break;
                    }
                }
                // if (this->OnTimer())
                //     return;
                std::this_thread::sleep_until(x);
            }
        });
    }

    static void RegisterTween(TweenBase* pt) {
        std::lock_guard<std::mutex> guard(m_tween_mutex);
        m_tweens.insert(pt);
    }

    static void UnregisterTween(TweenBase* pt) {
        std::lock_guard<std::mutex> guard(m_tween_mutex);
        m_tweens.erase(pt);
    }
};

template<typename TValue>
class Tween final : TweenBase
{
protected:
    enum class state
    {
        Stopped,
        Playing,
        Paused,
    };

    struct episode
    {
        TValue startValue;
        TValue endValue;
        float duration;
        std::function<float(float t)> easingFunction;
        std::function<void()> onStartFunction;
    };

    state m_state = state::Stopped;
    TValue m_startValue;
    TValue m_lastValue;
    double m_startTime = -1.0f;
    uint32_t m_maxRepeatCount = 1;
    uint32_t m_repeatCount = 0;
    uint32_t m_currentEpisodeIndex = uint32_t(-1);
    std::vector<episode> m_episodes;
    std::function<void(bool)> m_onComplete;
    std::function<void()> m_onStart;
    std::function<void()> m_onPause;
    std::function<void()> m_onStop;
    std::function<void()> m_onUpdate;
    std::function<void(Tween&)> m_onTimer;
    bool m_forward = true;
    bool m_yoyo = false;
    double m_pauseTime = -1.0f;

public:
    Tween() = default;
    Tween(TValue value)
        : m_startValue(value), m_lastValue(value)
    { }

    Tween(Tween &&) = default;
    Tween(const Tween&) = delete;

    ~Tween() {
        TweenService::UnregisterTween(this);
    }

    Tween& operator=(Tween&&) = default;
    Tween& operator=(const Tween&) = delete;

    Tween& From(TValue value)
    {
        m_startValue = value;
        m_lastValue = value;
        return *this;
    }

    bool IsStopped() const { return m_state == state::Stopped; }
    bool IsPlaying() const { return m_state == state::Playing; }
    bool IsPaused() const { return m_state == state::Paused; }

    Tween& Clear()
    {
        m_episodes.clear();
        return *this;
    }

    Tween& To(
        TValue value,
        float duration,
        std::function<float(float t)>&& easingFunction = [](float t) { return t; },
        std::function<void()>&& startFunction = nullptr)
    {
        m_episodes.push_back({
            m_episodes.empty() ? m_startValue : m_episodes.back().endValue,
            std::move(value),
            duration,
            std::move(easingFunction),
            std::move(startFunction)});
        return *this;
    }

    Tween& Delay(float duration, std::function<void()>&& startFunction = nullptr)
    {
        return To(
            TValue(m_episodes.empty() ? m_startValue : m_episodes.back().endValue),
            duration,
            [](float t) { return t; },
            std::move(startFunction));
    }

    Tween& Repeat(uint32_t maxRepeat = 0)
    {
        if (maxRepeat == 0)
            maxRepeat = std::numeric_limits<uint32_t>::max();

        m_maxRepeatCount = maxRepeat;
        return *this;
    }

    Tween& Yoyo(bool yoyo = true) { m_yoyo = yoyo; return *this; }

    Tween& OnStart(std::function<void()>&& fn) { m_onStart = std::move(fn); return *this; }
    Tween& OnPause(std::function<void()>&& fn) { m_onPause = std::move(fn); return *this; }
    Tween& OnStop(std::function<void()>&& fn) { m_onStop = std::move(fn); return *this; }
    Tween& OnUpdate(std::function<void()>&& fn) { m_onUpdate = std::move(fn); return *this; }
    Tween& OnComplete(std::function<void(bool)>&& fn) { m_onComplete = std::move(fn); return *this; }
    Tween& OnTimer(std::function<void(Tween&)>&& fn) {
        m_onTimer = std::move(fn);
        return *this;
     }

    Tween& Pause()
    {
        if (m_state == state::Playing)
        {
            m_state = state::Paused;
            m_pauseTime = Time::GetTime();
        }
        return *this;
    }

    Tween& Stop()
    {
        m_state = state::Stopped;
        m_startTime = -1.0f;
        m_repeatCount = 0;
        m_currentEpisodeIndex = uint32_t(-1);
        return *this;
    }

    Tween& Play(bool forward = true)
    {
        if (m_onTimer != nullptr)
            TweenService::RegisterTween(this);
        if (m_state == state::Paused)
        {
            m_state = state::Playing;
            m_startTime += Time::GetTime() - m_pauseTime;
            m_pauseTime = -1.0f;
            m_forward = forward;
        }
        else if (m_state == state::Stopped)
        {
            m_startTime = -1.0f;
            m_repeatCount = 0;
            m_currentEpisodeIndex = uint32_t(-1);
            m_forward = forward;
            m_state = state::Playing;
        }
        return *this;
    }

    const TValue& Read()
    {
        if (m_episodes.empty() || m_repeatCount >= m_maxRepeatCount)
            m_state = state::Stopped;

        if (m_state == state::Stopped)
            return m_lastValue;

        const double time = m_state == state::Paused
            ? m_pauseTime
            : Time::GetTime();

        if (m_startTime < 0.0f)
        {
            m_startTime = time;
            if (m_repeatCount == 0 && m_onStart != nullptr)
                m_onStart();
        }

        if (m_forward)
        {
            double durations = m_startTime;
            uint32_t i = 0;
            for (const auto& episode : m_episodes)
            {
                if (ProcessEpisode(time, durations, i, episode))
                    return m_lastValue;
                i++;
            }

            m_lastValue = m_episodes.back().endValue;
        }
        else // backward
        {
            double durations = m_startTime;
            for (size_t i = m_episodes.size(); i-- > 0;)
            {
                if (ProcessEpisode(time, durations, i, m_episodes[i]))
                    return m_lastValue;
            }

            m_lastValue = m_episodes.front().startValue;
        }

        m_repeatCount++;
        m_startTime = -1.0f;

        if (m_yoyo)
            m_forward = !m_forward;

        if (m_onComplete != nullptr)
            m_onComplete(/*willRepeat:*/ m_repeatCount < m_maxRepeatCount);

        return m_lastValue;
    }

protected:
    virtual void OnTimerTick() override {
        if (m_state != state::Stopped)
            m_onTimer(*this);
    }

    bool ProcessEpisode(float time, double& durations, uint32_t episodeIndex, const episode& episode)
    {
        double episodeStartTime = durations;
        durations += episode.duration;
        if (time < durations)
        {
            if (m_currentEpisodeIndex != episodeIndex)
            {
                m_currentEpisodeIndex = episodeIndex;
                if (episode.onStartFunction != nullptr)
                    episode.onStartFunction();
            }

            ReportState();

            const float t = (time - episodeStartTime) / episode.duration;
            if (m_forward)
                m_lastValue = math_trait<TValue>::lerp(episode.startValue, episode.endValue, episode.easingFunction(t));
            else
                m_lastValue = math_trait<TValue>::lerp(episode.endValue, episode.startValue, episode.easingFunction(t));
            return true;
        }

        return false;
    }

    void ReportState()
    {
        if (m_state == state::Paused && m_onStop != nullptr)
            m_onStop();
        else if (m_state == state::Paused && m_onPause != nullptr)
            m_onPause();
        else if (m_onUpdate != nullptr)
            m_onUpdate();
    }
};

} // xpf