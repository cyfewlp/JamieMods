//
// Created by jamie on 2025/5/21.
//

#ifndef TASKQUEUE_H
#define TASKQUEUE_H

#include "common/config.h"

#include <functional>
#include <queue>

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
class TaskQueue
{
public:
    using Task = std::function<void()>;

    void AddImeThreadTask(Task &&task)
    {
        m_imeThreadTasks.push(std::forward<Task>(task));
    }

    void AddMainThreadTask(Task &&task)
    {
        m_mainThreadTasks.push(std::forward<Task>(task));
    }

    void ExecuteImeThreadTasks()
    {
        if (m_imeThreadTasks.empty())
        {
            return;
        }
        m_imeThreadTasks.front()();
        m_imeThreadTasks.pop();
    }

    void ExecuteMainThreadTasks()
    {
        if (m_mainThreadTasks.empty())
        {
            return;
        }
        m_mainThreadTasks.front()();
        m_mainThreadTasks.pop();
    }

    static auto GetInstance() -> TaskQueue &
    {
        static TaskQueue instance;
        return instance;
    }

private:
    std::queue<Task> m_imeThreadTasks;
    std::queue<Task> m_mainThreadTasks;
};
}
}

#endif // TASKQUEUE_H
