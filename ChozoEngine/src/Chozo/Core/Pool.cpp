#include "Pool.h"

namespace Chozo {

    void Pool::Update()
    {
        if (m_Pause)
            return;

        auto& tasks = m_Tasks;

        if (tasks.empty())
        {
            m_Pause = true;
        }
        else
        {
            auto& task = tasks[0];

            if (task->GetStatus() == TaskStatus::None && task->GetStatus() != TaskStatus::Executed)
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
        auto it = std::find_if(tasks.begin(), tasks.end(), [&task](const Ref<PoolTask>& t) {
            return t->GetHandle() == task->GetHandle();
        });

        return it != tasks.end();
    }

    void Pool::AddTask(Ref<PoolTask> task)
    {
        m_Tasks.emplace_back(task);
    }

    void Pool::RemoveTask(Ref<PoolTask> task)
    {
        auto& tasks = m_Tasks;
        auto it = std::find_if(tasks.begin(), tasks.end(), [&task](const Ref<PoolTask>& t) {
            return t->GetHandle() == task->GetHandle();
        });

        if (it != tasks.end())
            tasks.erase(it);
    }

} // namespace Chozo
