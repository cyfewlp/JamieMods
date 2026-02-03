//
// Created by jamie on 2026/1/17.
//

#pragma once

#include <chrono>

namespace Ime
{
class DebounceTimer
{
    using Clock = std::chrono::steady_clock;

public:
    explicit DebounceTimer(std::chrono::milliseconds delay) : m_Delay(delay) {}

    void Poke()
    {
        m_LastPokeTime = Clock::now();
        m_IsWaiting    = true;
        m_Triggered    = false;
    }

    bool Check()
    {
        if (!m_IsWaiting) return false;

        auto now = Clock::now();
        if (now - m_LastPokeTime >= m_Delay)
        {
            m_IsWaiting = false;
            m_Triggered = true;
            return true;
        }
        return false;
    }

    bool IsWaiting() const
    {
        return m_IsWaiting;
    }

    void Reset()
    {
        m_IsWaiting = false;
        m_Triggered = false;
    }

private:
    std::chrono::milliseconds m_Delay;
    Clock::time_point         m_LastPokeTime;
    bool                      m_IsWaiting = false;
    bool                      m_Triggered = false;
};
} // namespace Ime
