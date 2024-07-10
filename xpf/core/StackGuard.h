#pragma once
#include <functional>

namespace xpf {

class StackGuard {
private:
    std::function<void()> m_fn;

public:
    StackGuard() : m_fn([](){}) {}
    StackGuard(StackGuard&&) = default;
    StackGuard(const StackGuard&) = delete;
    StackGuard(std::function<void()>&& fn) : m_fn(std::move(fn)) { }

    StackGuard& operator=(StackGuard&& sg) { m_fn = std::move(sg.m_fn); sg.m_fn = [](){}; return *this; }

    void Dismiss() { m_fn = []{} ; };
    ~StackGuard() { m_fn(); };
};

} // xpf