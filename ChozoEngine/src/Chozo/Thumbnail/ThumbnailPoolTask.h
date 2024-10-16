#pragma once

#include "Chozo/Asset/Asset.h"
#include "Chozo/Renderer/Texture.h"

#include "czpch.h"

namespace Chozo {

    enum class TaskStatus {
        Unfinished,
        Finished,
    };

    typedef int TaskFlags;

    enum TaskFlags_ {
        TaskFlags_None   = 0,
        TaskFlags_Export = 1 << 0,
    };

    class ThumbnailPoolTask : public RefCounted
    {
    public:
        UUID ID;
        Ref<Asset> Source;
        Ref<Texture2D> Thumbnail;
        TaskFlags Flags;
        TaskStatus Status;

        ThumbnailPoolTask() = default;
        ThumbnailPoolTask(Ref<Asset> asset, TaskFlags flags)
            : ID(UUID()), Source(asset), Flags(flags), Status(TaskStatus::Unfinished)
        {}
    };
}