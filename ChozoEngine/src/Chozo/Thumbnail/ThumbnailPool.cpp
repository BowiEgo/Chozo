#include "ThumbnailPool.h"

#include "ThumbnailManager.h"
#include "ThumbnailRenderer.h"

namespace Chozo {

	Ref<ThumbnailPool> ThumbnailPool::s_Instance;
	bool ThumbnailPool::s_Pause = true;

    void ThumbnailPool::Update()
    {
        if (!s_Instance || s_Pause)
            return;

        auto tasks = s_Instance->m_Tasks;

        if (!tasks.empty())
        {
            auto& task = tasks[0];

            if (task->Status == TaskStatus::Unfinished)
            {
                RenderTask(task);
            }
            else
            {
                ThumbnailManager::SetThumbnail(task->Source->Handle, task->Thumbnail);
                RemoveTask(task);
            }
        }
        else
        {
            s_Pause = true;
        }
    }

    bool ThumbnailPool::TaskExists(Ref<Asset> asset)
    {
        for (const auto& task : s_Instance->m_Tasks)
        {
            if (task->Source == asset)
                return true;
        }
        return false;
    }

    void ThumbnailPool::CreateTask(Ref<Asset> asset, TaskFlags flags)
    {
        if (!TaskExists(asset))
        {
            s_Instance->m_Tasks.emplace_back(Ref<ThumbnailPoolTask>::Create(asset, flags));
        }
    }

    void ThumbnailPool::RemoveTask(Ref<ThumbnailPoolTask> task)
    {
        auto& tasks = s_Instance->m_Tasks;
        auto it = std::find_if(tasks.begin(), tasks.end(), [&task](const Ref<ThumbnailPoolTask>& t) {
            return t->ID == task->ID;
        });

        if (it != tasks.end())
            tasks.erase(it);
    }

    void ThumbnailPool::RenderTask(Ref<ThumbnailPoolTask> task)
    {
        ThumbnailRenderer::RenderTask(task);
    }

} // namespace Chozo
