
#pragma once

#include "Runtime/RHI/CommandList.h"
#include <Core/Layer/LayerStack.h>
#include <Core/Memory/Memory.h>

namespace CZ {

class StartupHostObj {
public:
    StartupHostObj()  = default;
    ~StartupHostObj() = default;

    virtual void Draw(CommandList cmdList) = 0;

    virtual Layer* GetStartupLayer() const = 0;

    bool IsOffscreen() { return m_bOffscreenRendering; }

protected:
    bool m_bOffscreenRendering;
};

using DrawFunc = std::function<void(CommandList)>;

struct StartupHost : Handle<class StartupHostObj> {
    static StartupHost Create();

    bool IsOffscreen() const;

    Layer* GetStartupLayer() const;

    void Draw(CommandList cmdList);
};

} // namespace CZ