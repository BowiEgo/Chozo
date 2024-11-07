#pragma once

#include "Chozo/Asset/Asset.h"
#include "Chozo/Renderer/Texture.h"

#include "PoolTask.h"

namespace Chozo {

    class Pool : public RefCounted
    {
    public:
        Pool() {};
        ~Pool() = default;

        inline void Start() { m_Pause = false; }
        inline void Pause() { m_Pause = true; }
        void Update();
        bool TaskExists(Ref<PoolTask> task);
        void AddTask(Ref<PoolTask> task);
        void RemoveTask(Ref<PoolTask> task);
    private:
        bool m_Pause;
        std::vector<Ref<PoolTask>> m_Tasks;
    };
    
} // namespace Chozo
