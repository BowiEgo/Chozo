
#pragma once

#include <Core/Layer/LayerStack.hpp>
#include <Core/Memory/Memory.hpp>
#include <Runtime/RHI/CommandList.hpp>

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
    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }

    static StartupHost Create();

    bool IsOffscreen() const;

    Layer* GetStartupLayer() const;

    void Draw(CommandList cmdList);
};

} // namespace CZ