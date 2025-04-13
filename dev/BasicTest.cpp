#include <algorithm>
#include <coroutine>
#include <iostream>
#include <optional>
#include <spdlog/spdlog.h>

#include <coroutine>
#include <iostream>
#include <stdexcept>
#include <thread>

auto switch_to_new_thread(std::jthread &out)
{
    struct awaitable
    {
        std::jthread *p_out;

        bool await_ready()
        {
            std::cout << "await_ready " << '\n';
            return false;
        }

        void await_suspend(std::coroutine_handle<> h)
        {
            std::cout << "await_suspend " << '\n';
            std::jthread &out = *p_out;
            if (out.joinable())
                throw std::runtime_error("Output jthread parameter not empty");
            out = std::jthread([h] {
                if (!h.done())
                {
                    h.resume();
                }
            });
            // Potential undefined behavior: accessing potentially destroyed *this
            // std::cout << "New thread ID: " << p_out->get_id() << '\n';
            std::cout << "New thread ID: " << out.get_id() << '\n'; // this is OK
        }

        void await_resume()
        {
            std::cout << "await_resume " << '\n';
        }
    };
    return awaitable{&out};
}

template <typename T>
struct task
{
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type
    {
        T value_;

        task get_return_object()
        {
            std::cout << "get_return_object " << '\n';
            return task(handle_type::from_promise(*this));
        }

        std::suspend_never initial_suspend()
        {
            std::cout << "initial_suspend " << '\n';
            return {};
        }

        std::suspend_never final_suspend() noexcept
        {
            std::cout << "final_suspend " << '\n';
            return {};
        }

        void return_void()
        {
            std::cout << "return_void " << '\n';
        }

        void unhandled_exception()
        {
            std::cout << "unhandled_exception " << '\n';
        }

        template <std::convertible_to<T> From> // C++20 concept
        std::suspend_always yield_value(From &&from)
        {
            value_ = std::forward<From>(from); // caching the result in promise
            return {};
        }
    };

    handle_type h_;

    task(handle_type h) : h_(h)
    {
    }

    ~task()
    {
        h_.destroy();
    }
};

struct lazy_task
{
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type
    {
        bool started = false;

        lazy_task get_return_object()
        {
            spdlog::info("layz_task::promise_type: {}", __func__);
            return lazy_task(handle_type::from_promise(*this));
        }

        struct initial_awaitable
        {
            promise_type *promise;

            bool await_ready() const noexcept
            {
                spdlog::info("  initial_awaitable: {}", __func__);
                return false;
            }

            void await_suspend(std::coroutine_handle<>)
            {
                spdlog::info("  initial_awaitable: {}", __func__);
            }

            void await_resume()
            {
                spdlog::info("  initial_awaitable: {}", __func__);
                promise->started = true;
            }
        };

        initial_awaitable initial_suspend()
        {
            spdlog::info("layz_task::promise_type: {}", __func__);
            return initial_awaitable{this};
        }

        struct final_awaitable
        {
            promise_type *promise;

            bool await_ready() const noexcept
            {
                spdlog::info("final_awaitable: {}", __func__);
                return false;
            }

            auto await_suspend(std::coroutine_handle<promise_type> handle) noexcept
            {
                spdlog::info("final_awaitable: {}", __func__);
                std::coroutine_handle<void> res = std::noop_coroutine();
                return res;
            }

            void await_resume() noexcept
            {
                spdlog::info("final_awaitable: {}", __func__);
            }
        };

        std::suspend_always final_suspend() noexcept
        {
            spdlog::info("layz_task::promise_type: {}", __func__);
            return {};
        }

        void return_void()
        {
            spdlog::info("layz_task::promise_type: {}", __func__);
        }

        void unhandled_exception()
        {
            spdlog::info("layz_task::promise_type: {}", __func__);
        }
    };

    handle_type h;

    lazy_task(handle_type h) : h(h)
    {

    }

    auto operator ()() const
    {
        return h();
    }
};

task<std::uint32_t> resuming_on_new_thread(std::jthread &out)
{
    std::cout << "Coroutine started on thread: " << std::this_thread::get_id() << '\n';
    co_await switch_to_new_thread(out);

    std::cout << "Coroutine resumed on thread: " << std::this_thread::get_id() << '\n';
}

lazy_task resuming_lazy_task(std::jthread &out)
{
    std::cout << "Coroutine started on thread: " << std::this_thread::get_id() << '\n';
    co_await switch_to_new_thread(out);

    std::cout << "Coroutine resumed on thread: " << std::this_thread::get_id() << '\n';
}

std::function<void()> createLambda() {
    int x = 42;
    return [&]() { std::cout << x << std::endl; }; // 捕获局部变量 x 的引用
}

int main() {
    auto lambda = createLambda(); // x 已被销毁
    lambda(); // 未定义行为：访问悬空引用
    return 0;
}