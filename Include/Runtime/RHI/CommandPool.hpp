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

class CommandPoolObj;

struct CommandPool : Handle<class CommandPoolObj> {
    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }

    CommandList AllocateCommandBuffer();
};

} // namespace CZ
