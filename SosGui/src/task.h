//
// Created by jamie on 2025/4/23.
//

#pragma once

#include <coroutine>
#include <exception>

namespace SosGui
{
class TaskPromiseBase
{
    struct final_awaitable
    {
        bool await_ready() const noexcept { return false; }

        template <typename PROMISE>
        std::coroutine_handle<> await_suspend(std::coroutine_handle<PROMISE> coro) noexcept
        {
            return coro.promise().m_continuation;
        }

        void await_resume() noexcept {}
    };

public:
    TaskPromiseBase() noexcept {}

    auto initial_suspend() noexcept { return std::suspend_always{}; }

    auto final_suspend() noexcept { return final_awaitable{}; }

    void set_continuation(std::coroutine_handle<> continuation) noexcept { m_continuation = continuation; }

private:
    std::coroutine_handle<> m_continuation;
};

class Task;

class TaskPromise final : public TaskPromiseBase
{
public:
    TaskPromise() noexcept {}

    ~TaskPromise() noexcept {}

    Task get_return_object(const std::source_location &location = std::source_location::current()) noexcept;

    void unhandled_exception() noexcept { m_exception = std::current_exception(); }

    void return_void() noexcept {}

private:
    std::exception_ptr m_exception;
};

class Task
{
public:
    using handle_type  = std::coroutine_handle<TaskPromise>;
    using promise_type = TaskPromise;
    bool m_awaited     = false;

private:
    struct awaitable
    {
        std::coroutine_handle<promise_type> m_coroutine;

        bool await_ready() const noexcept { return !m_coroutine || m_coroutine.done(); }

        auto await_suspend(std::coroutine_handle<> a_handle) const noexcept -> std::coroutine_handle<>
        {
            m_coroutine.promise().set_continuation(a_handle);
            return m_coroutine;
        }

        void await_resume() {}
    };

public:
    explicit Task() noexcept : m_handle(nullptr) {}

    explicit Task(handle_type handle) noexcept : m_handle(handle) {}

    Task(const Task &other)            = delete;
    Task &operator=(const Task &other) = delete;

    Task(Task &&other) noexcept : m_awaited(other.m_awaited), m_handle(other.m_handle)
    {
        other.m_handle  = nullptr;
        other.m_awaited = false;
    }

    ~Task()
    {
        logger::debug("{}", __FUNCTION__);
        if (m_handle)
        {
            m_handle.destroy();
        }
    }

    Task &operator=(Task &&other) = delete;

    auto operator co_await() const -> awaitable { return awaitable{m_handle}; }

private:
    std::coroutine_handle<promise_type> m_handle;
};

inline Task TaskPromise::get_return_object(const std::source_location &location) noexcept
{
    logger::debug("CoroutineTaskPromise::get_return_object: {}", location.function_name());
    return Task{std::coroutine_handle<TaskPromise>::from_promise(*this)};
}

struct EagerTask
{
    struct promise_type
    {
        EagerTask get_return_object() { return {}; }

        std::suspend_never initial_suspend() { return {}; }

        std::suspend_never final_suspend() noexcept { return {}; }

        void return_void() {}

        void unhandled_exception() {}
    };
};

template <typename Task>
EagerTask operator+(Task &&task)
{
    logger::debug("start {}", __FUNCTION__);
    co_await task();
    logger::debug("end {}", __FUNCTION__);
}
} // namespace SosGui
