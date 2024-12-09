#pragma once

#include "Chozo/Asset/Asset.h"

#include "czpch.h"

namespace Chozo {

    enum class TaskStatus {
        None,
        Executed,
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

        virtual void Execute() = 0;
        virtual void Finish() = 0;

        inline UUID GetHandle() const { return m_Handle; }
        inline void SetStatus(TaskStatus status) { m_Status = status; };
        inline TaskStatus GetStatus() const { return m_Status; };
    protected:
        UUID m_Handle;
        TaskStatus m_Status = TaskStatus::None;
    };
}