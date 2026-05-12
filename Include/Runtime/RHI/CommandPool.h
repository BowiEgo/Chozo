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

struct CommandPool : Handle<CommandPoolObj> {
    static void Destroy(CommandPool cmdList);

    CommandList AllocateCommandBuffer();
};

} // namespace CZ
