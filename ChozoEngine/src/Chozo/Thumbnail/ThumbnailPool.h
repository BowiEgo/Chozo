#pragma once

#include "Chozo/Asset/Asset.h"
#include "Chozo/Renderer/Texture.h"

#include "ThumbnailPoolTask.h"

namespace Chozo {

    class ThumbnailPool : public RefCounted
    {
    public:
        ThumbnailPool() {};
        ~ThumbnailPool() = default;

        inline static void Init() { s_Instance = Ref<ThumbnailPool>::Create(); }

        inline static void Start() { s_Pause = false; }
        inline static void Pause() { s_Pause = true; }
        static void Update();
        static bool TaskExists(Ref<Asset> asset);
        static void CreateTask(Ref<Asset> asset, TaskFlags flags = TaskFlags_None);
        static void RemoveTask(Ref<ThumbnailPoolTask> task);
        static void RenderTask(Ref<ThumbnailPoolTask> task);
    private:
        static Ref<ThumbnailPool> s_Instance;
        static bool s_Pause;

        std::vector<Ref<ThumbnailPoolTask>> m_Tasks;
    };
    
} // namespace Chozo
