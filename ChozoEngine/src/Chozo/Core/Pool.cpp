#include "Pool.h"

namespace Chozo {

	Ref<Pool> Pool::s_Instance;
	bool Pool::s_Pause = true;

    void Pool::Update()
    {
        if (!s_Instance || s_Pause)
            return;

        auto& tasks = s_Instance->m_Tasks;

        if (tasks.empty())
        {
            s_Pause = true;
        }
        else
        {
            auto& task = tasks[0];

            if (task->GetStatus() == TaskStatus::Unfinished)
            {
                task->Execute();
            }
            else
            {
                task->Finish();
                RemoveTask(task);
            }
        }
    }

    bool Pool::TaskExists(Ref<PoolTask> task)
    {
        auto& tasks = s_Instance->m_Tasks;
        auto it = std::find_if(tasks.begin(), tasks.end(), [&task](const Ref<PoolTask>& t) {
            return t->GetHandle() == task->GetHandle();
        });

        return it != tasks.end();
    }

    void Pool::AddTask(Ref<PoolTask> task)
    {
        s_Instance->m_Tasks.emplace_back(task);
    }

    void Pool::RemoveTask(Ref<PoolTask> task)
    {
        auto& tasks = s_Instance->m_Tasks;
        auto it = std::find_if(tasks.begin(), tasks.end(), [&task](const Ref<PoolTask>& t) {
            return t->GetHandle() == task->GetHandle();
        });

        if (it != tasks.end())
            tasks.erase(it);
    }

} // namespace Chozo
