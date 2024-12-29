#pragma once

#include "Chozo/Renderer/Texture.h"

#include "PoolTask.h"

namespace Chozo {

    class Pool final : public RefCounted
    {
    public:
        Pool() = default;
        ~Pool() override = default;

        inline void Start() { m_Pause = false; }
        inline void Pause() { m_Pause = true; }
        void Update();
        bool TaskExists(Ref<PoolTask> task);
        void AddTask(const Ref<PoolTask>& task);
        void RemoveTask(Ref<PoolTask> task);
    private:
        bool m_Pause{true};
        std::vector<Ref<PoolTask>> m_Tasks;
    };
    
} // namespace Chozo
