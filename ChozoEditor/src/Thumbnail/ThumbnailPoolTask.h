#pragma once

#include "Chozo/Core/PoolTask.h"

#include "Chozo/Asset/Asset.h"
#include "Chozo/Renderer/Texture.h"

#include "czpch.h"

namespace Chozo {

    enum Thumbnail_PoolTaskFlags_ {
        PoolTaskFlags_None     = 0,
        PoolTaskFlags_Export   = BIT(0),
        PoolTaskFlags_Import   = BIT(1),
        PoolTaskFlags_Process  = BIT(2),
    };

    class ThumbnailPoolTask : public PoolTask
    {
    public:
        Ref<Asset> Source;
        SharedBuffer ImageData;
        PoolTaskFlags Flags;

        ThumbnailPoolTask() = default;
        ThumbnailPoolTask(Ref<Asset> asset, PoolTaskFlags flags)
            : Source(asset), ImageData(SharedBuffer()), Flags(flags)
        {
        }

        virtual void Execute() override;
        virtual void Finish() override;
    private:
    };
}