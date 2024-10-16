#pragma once

#include "Chozo/Asset/Asset.h"
#include "Chozo/Renderer/Texture.h"

#include "czpch.h"

namespace Chozo {

    enum class TaskStatus {
        Unfinished,
        Finished,
    };

    typedef int PoolTaskFlags;

    class PoolTask : public RefCounted
    {
    public:
        PoolTask() = default;

        virtual bool operator==(const PoolTask& other) const
		{
			return m_Handle == other.GetHandle();
		}

        virtual TaskStatus GetStatus() = 0;
        virtual void Execute() = 0;
        virtual void Finish() = 0;

        inline UUID GetHandle() const { return m_Handle; }
    protected:
        UUID m_Handle;
    };
}