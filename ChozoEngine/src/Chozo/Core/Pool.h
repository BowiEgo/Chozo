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

        inline static void Init() { s_Instance = Ref<Pool>::Create(); }

        inline static void Start() { s_Pause = false; }
        inline static void Pause() { s_Pause = true; }
        static void Update();
        static bool TaskExists(Ref<PoolTask> task);
        static void AddTask(Ref<PoolTask> task);
        static void RemoveTask(Ref<PoolTask> task);
    private:
        static Ref<Pool> s_Instance;
        static bool s_Pause;

        std::vector<Ref<PoolTask>> m_Tasks;
    };
    
} // namespace Chozo
