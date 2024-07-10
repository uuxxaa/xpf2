#pragma once
#include <atomic>
#include <functional>
#include <mutex>
#include <stack>
#include <thread>
#include <core/Event.h>
#include <core/Log.h>

namespace xpf {

class TaskQueue {
protected:
    struct task {
        std::function<void()> taskFn;
        std::function<void(const std::exception*)> onCompleteFn; // null exception: succeeded
    };

    std::atomic<bool> m_shutdown = false;

    xpf::AutoResetEvent m_worker_event;
    std::mutex m_worker_mutex;
    std::thread m_worker_thread;
    std::stack<task> m_worker_tasks;
    std::atomic<uint32_t> m_worker_taskcount = 0;

    std::mutex m_ui_mutex;
    std::stack<std::function<void()>> m_ui_tasks;
    std::atomic<uint32_t> m_ui_taskcount = 0;

    std::function<void(const std::exception&)> m_on_exception = [](const std::exception&){};

public:
    TaskQueue() = default;
    TaskQueue(TaskQueue&&) = delete;
    TaskQueue(const TaskQueue&) = delete;

    void Start() {
        m_worker_thread = std::thread([this]() {
            OnStart();
        });
    }

    void OnException(std::function<void(const std::exception&)>&& fn) {
        m_on_exception = std::move(fn);
    }

    void Stop() {
        m_shutdown = true;
        m_worker_event.Fire();
        m_worker_thread.join();
    }

    void EnqueueWork(std::function<void()>&& fn) {
        if (m_shutdown) return;

        std::unique_lock<std::mutex> lock(m_worker_mutex);
        m_worker_tasks.push({fn, [](auto){}});
        m_worker_taskcount++;
        m_worker_event.Fire();
    }

    void DispatchToUI(std::function<void()>&& fn) {
        if (m_shutdown) return;

        std::unique_lock<std::mutex> lock(m_ui_mutex);
        m_ui_tasks.push(std::move(fn));
        m_ui_taskcount++;
    }

    void RunUITasks() {
        while (!m_shutdown && m_ui_taskcount != 0) {
            std::unique_lock<std::mutex> lock(m_ui_mutex);
            const auto currentTask = std::move(m_ui_tasks.top());
            m_ui_tasks.pop();
            m_ui_taskcount--;
            try {
                xpf::Log::info("running ui task");
                currentTask();
            } catch (const std::exception& e) {
                m_on_exception(e);
            }
        }
    }

protected:
    void OnStart() {
        xpf::Log::info("task queue starting up");
        while (!m_shutdown) {
            m_worker_event.WaitInfinite();

            while (!m_shutdown && m_worker_taskcount != 0) {
                const auto currentTask = std::move(m_worker_tasks.top());
                m_worker_tasks.pop();
                m_worker_taskcount--;

                bool completeFiredException = false;
                try {
                    xpf::Log::info("running worker task");
                    currentTask.taskFn();

                    if (!m_shutdown && currentTask.onCompleteFn != nullptr) {
                        completeFiredException = true;
                        currentTask.onCompleteFn(nullptr);
                    }
                } catch (const std::exception& e) {
                    if (!m_shutdown) {
                        try {
                            if (!completeFiredException && currentTask.onCompleteFn != nullptr) {
                                currentTask.onCompleteFn(&e);
                            } else {
                                m_on_exception(e);
                            }
                        } catch (const std::exception& e) {
                            m_on_exception(e);
                        }
                    }
                }
            }
        }
        xpf::Log::info("task queue shutting down");
    }
};

} // xpf