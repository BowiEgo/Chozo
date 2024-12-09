#include "Pool.h"

namespace Chozo {

    void Pool::Update()
    {
        if (m_Pause)
            return;

        if (auto& tasks = m_Tasks; tasks.empty())
        {
            m_Pause = true;
        }
        else
        {
            if (auto& task = tasks[0]; task->GetStatus() == TaskStatus::None && task->GetStatus() != TaskStatus::Executed)
            {
                task->Execute();
            }
            else if (task->GetStatus() == TaskStatus::Finished)
            {
                task->Finish();
                RemoveTask(task);
            }
        }
    }

    bool Pool::TaskExists(Ref<PoolTask> task)
    {
        auto& tasks = m_Tasks;
        const auto it = std::find_if(tasks.begin(), tasks.end(), [&task](const Ref<PoolTask>& t) {
            return t->GetHandle() == task->GetHandle();
        });

        return it != tasks.end();
    }

    void Pool::AddTask(const Ref<PoolTask>& task)
    {
        m_Tasks.emplace_back(task);
    }

    void Pool::RemoveTask(Ref<PoolTask> task)
    {
        auto& tasks = m_Tasks;
        const auto it = std::find_if(tasks.begin(), tasks.end(), [&task](const Ref<PoolTask>& t) {
            return t->GetHandle() == task->GetHandle();
        });

        if (it != tasks.end())
            tasks.erase(it);
    }

} // namespace Chozo
