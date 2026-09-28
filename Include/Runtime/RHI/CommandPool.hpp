#pragma once

#include <Core/Header/Handle.hpp>
#include <Core/Header/Types.h>
#include <Core/Memory/Memory.hpp>
#include <Runtime/RHI/CommandList.hpp>
#include <Runtime/RHI/RHITypes.hpp>

namespace CZ {

struct CommandPoolSpecification {
    uint32 QueueIndex;
    CommandPoolFlags Flags;
};

class CommandPoolObj {
public:
    CommandPoolObj(CommandPoolSpecification& spec) : m_Spec(spec) {}
    virtual ~CommandPoolObj() = default;

    CommandPoolObj(const CommandPoolObj&)            = delete;
    CommandPoolObj& operator=(const CommandPoolObj&) = delete;
    CommandPoolObj(CommandPoolObj&&)                 = delete;
    CommandPoolObj& operator=(CommandPoolObj&&)      = delete;

    /// Allocates a command buffer owned by the caller.
    virtual Scope<CommandListObj> AllocateCommandBuffer() = 0;

protected:
    CommandPoolSpecification m_Spec;
};

struct CommandPool : Handle<class CommandPoolObj> {
    using Handle<class CommandPoolObj>::Handle;

    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }
};

} // namespace CZ
