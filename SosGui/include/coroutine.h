#pragma once

#include "common/config.h"

#include <common/log.h>
#include <coroutine>
#include <exception>
#include <memory>

namespace LIBC_NAMESPACE_DECL
{
    struct already_awaited_exception : std::runtime_error
    {
        explicit already_awaited_exception() : std::runtime_error("This coroutine already await") {}
    };

    inline std::exception_ptr already_awaited() { return std::make_exception_ptr(already_awaited_exception()); }

    class CoroutineTaskPromise;

    struct CoroutineTask
    {
        using handle_type  = std::coroutine_handle<CoroutineTaskPromise>;
        using promise_type = CoroutineTaskPromise;

        CoroutineTask(handle_type handle) : m_handle(handle) {}

        CoroutineTask(const CoroutineTask &other) = delete;

        CoroutineTask &operator=(const CoroutineTask &other) = delete;

        CoroutineTask(CoroutineTask &&other) noexcept
        {
            m_handle       = other.m_handle;
            other.m_handle = nullptr;
        }

        CoroutineTask &operator=(CoroutineTask &&other) = delete;

        struct awaitable
        {
            CoroutineTask *self;
            bool           done = false;

            bool await_ready() const noexcept { return false; }

            auto await_suspend(std::coroutine_handle<> a_handle) const noexcept -> std::coroutine_handle<>;

            void await_resume() {}
        };

        auto operator co_await() -> awaitable;

    private:
        std::coroutine_handle<promise_type> m_handle;
    };

    struct coroutine_handle_deleter
    {
        void operator()(void *p) const { std::coroutine_handle<>::from_address(p).destroy(); }
    };

    class CoroutineTaskPromise
    {
        bool                                            m_started   = false;
        std::unique_ptr<void, coroutine_handle_deleter> m_awaitFrom = nullptr;

        using handle_type = std::coroutine_handle<CoroutineTaskPromise>;
        friend struct CoroutineTask;

    public:
        CoroutineTask get_return_object(const std::source_location &location = std::source_location::current())
        {
            log_debug("CoroutineTaskPromise::get_return_object: {}", location.function_name());
            return CoroutineTask{handle_type::from_promise(*this)};
        }

        struct initial_awaitable
        {
            CoroutineTaskPromise *promise;

            bool await_ready() const noexcept { return false; }

            void await_suspend(std::coroutine_handle<>) {}

            void await_resume() { promise->m_started = true; }
        };

        initial_awaitable initial_suspend() { return initial_awaitable{this}; }

        struct final_awaitable
        {
            CoroutineTaskPromise *promise;

            bool await_ready() const noexcept { return promise->m_awaitFrom == nullptr; }

            auto await_suspend(std::coroutine_handle<CoroutineTaskPromise> a_handle) const noexcept
            {
                std::coroutine_handle<> res = std::noop_coroutine();
                if (promise->m_awaitFrom)
                {
                    res = std::coroutine_handle<CoroutineTaskPromise>::from_address(promise->m_awaitFrom.release());
                }
                a_handle.destroy();
                return res;
            }

            void await_resume() const noexcept {}
        };

        final_awaitable final_suspend() noexcept { return final_awaitable{this}; }

        void return_void() const {}

        void unhandled_exception() { throw std::current_exception(); }
    };

    class CoroutinePromise
    {
    public:
        struct promise_type
        {
            std::unique_ptr<void, coroutine_handle_deleter> m_awaitFrom = nullptr;
            bool                                            done        = false;

            promise_type() = default;

            promise_type(const promise_type &other) = delete;

            promise_type &operator=(const promise_type &other) = delete;

            promise_type(promise_type &&other) = delete;

            promise_type &operator=(const promise_type &&other) = delete;

            CoroutinePromise get_return_object() { return CoroutinePromise(this); }

            std::suspend_never initial_suspend() { return {}; }

            auto final_suspend() noexcept
            {
                struct final_awaitable
                {
                    promise_type *self;

                    bool await_ready() const noexcept { return self->m_awaitFrom == nullptr; }

                    auto await_suspend(const std::coroutine_handle<> h) const noexcept
                    {
                        std::coroutine_handle<> res = std::noop_coroutine();
                        if (self->m_awaitFrom != nullptr)
                        {
                            res = std::coroutine_handle<>::from_address(self->m_awaitFrom.release());
                        }
                        h.destroy();
                        return res;
                    }

                    void await_resume() const noexcept {}
                };

                return final_awaitable{this};
            }

            void return_void() { done = true; }

            void unhandled_exception() { throw std::current_exception(); }

            ~promise_type() { m_awaitFrom.reset(nullptr); }
        };

        CoroutinePromise(promise_type *promise) : m_handle(std::coroutine_handle<promise_type>::from_promise(*promise))
        {
        }

        auto operator co_await()
        {
            struct inner_awaitable
            {
                CoroutinePromise  *self;
                std::exception_ptr exception{nullptr};

                bool await_ready() const noexcept { return false; }

                auto await_suspend(std::coroutine_handle<> a_handle) noexcept
                {
                    auto &promise = self->m_handle.promise();
                    if (promise.done) { return false; }
                    if (exception) { return false; }

                    if (promise.m_awaitFrom != nullptr)
                    {
                        exception = already_awaited();
                        return false;
                    }

                    promise.m_awaitFrom.reset(a_handle.address());
                    return true;
                }

                void await_resume() const
                {
                    if (exception) { std::rethrow_exception(exception); }
                }
            };

            return inner_awaitable{this};
        }

    private:
        std::coroutine_handle<promise_type> m_handle;
    };
}