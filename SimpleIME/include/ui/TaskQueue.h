//
// Created by jamie on 2025/5/21.
//

#ifndef TASKQUEUE_H
#define TASKQUEUE_H

#include <functional>
#include <queue>

namespace Ime
{
class TaskQueue
{
public:
    using Task = std::function<void()>;

    void AddImeThreadTask(Task &&task)
    {
        const std::scoped_lock lock(m_mutex);
        m_imeThreadTasks.push(std::forward<Task>(task));
    }

    void AddMainThreadTask(Task &&task)
    {
        const std::scoped_lock lock(m_mutex);
        m_mainThreadTasks.push(std::forward<Task>(task));
    }

    void ExecuteImeThreadTasks()
    {
        const std::scoped_lock lock(m_mutex);
        if (m_imeThreadTasks.empty())
        {
            return;
        }
        while (!m_imeThreadTasks.empty())
        {
            m_imeThreadTasks.front()();
            m_imeThreadTasks.pop();
        }
    }

    void ExecuteMainThreadTasks()
    {
        const std::scoped_lock lock(m_mutex);
        if (m_mainThreadTasks.empty())
        {
            return;
        }
        while (!m_mainThreadTasks.empty())
        {
            m_mainThreadTasks.front()();
            m_mainThreadTasks.pop();
        }
    }

    static auto GetInstance() -> TaskQueue &
    {
        static TaskQueue instance;
        return instance;
    }

private:
    std::mutex       m_mutex;
    std::queue<Task> m_imeThreadTasks;
    std::queue<Task> m_mainThreadTasks;
};
} // namespace Ime

#endif // TASKQUEUE_H
