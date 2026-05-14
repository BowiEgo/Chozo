#pragma once

#include "Runtime/RHI/CommandList.h"
#include <Core/Header/Handle.h>
#include <Core/Header/Types.h>
#include <Core/Memory/Memory.h>
#include <Runtime/RHI/RHITypes.h>

namespace CZ {

struct CommandPoolSpecification {
    uint32 QueueIndex;
    CommandPoolFlags Flags;
};

class CommandPoolObj;

struct CommandPool : Handle<class CommandPoolObj> {
    template <typename T> T* As() { return static_cast<T*>(RHIInternalReader::Unwrap(*this)); }

    CommandList AllocateCommandBuffer();
};

} // namespace CZ
