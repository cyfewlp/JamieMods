#include "coroutine.h"

namespace LIBC_NAMESPACE_DECL
{
    auto CoroutineTask::awaitable::await_suspend(std::coroutine_handle<> a_handle) const noexcept
        -> std::coroutine_handle<>
    {
        self->m_handle.promise().m_awaitFrom.reset(a_handle.address());
        return self->m_handle;
    }

    auto CoroutineTask::operator co_await() -> awaitable { return awaitable{this}; }

}