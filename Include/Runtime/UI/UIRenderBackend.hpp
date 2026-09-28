#pragma once

#include <Core/Header/UUID.hpp>
#include <Core/Memory/Memory.hpp>
#include <Runtime/RHI/CommandList.hpp>
#include <Runtime/RHI/GraphicsContext.hpp>
#include <Runtime/RHI/Texture.hpp>
#include <Runtime/Window/Window.hpp>

#include <cstdint>
#include <string>

struct ImDrawData; // ImGui stays out of this header on purpose.

namespace CZ {

/**
 * Contract between the editor UI (dear ImGui) and a graphics backend.
 *
 * The editor drives the interface; the concrete implementation lives inside the graphics backend
 * module (for Vulkan: `Source/Backend/Vulkan/VulkanUIBackend.*`), which is the only place that
 * knows about ImGui's renderer backend, SDL events and native handles.
 *
 * Texture registrations are identified by the texture's stable `UUID`, never by its address: an
 * address can be reused by a new object after the old one is destroyed, which previously led to
 * drawing with a descriptor set that referenced a destroyed image view.
 */
class UIRenderBackendObj {
public:
    virtual ~UIRenderBackendObj() = default;

    /// Binds the backend to a window and a graphics context. Returns false and fills `err` when
    /// the UI cannot be rendered (e.g. the backend has no UI support for this context).
    virtual bool Init(Window window, GraphicsContext ctx, std::string& err) = 0;

    virtual void NewFrame() = 0;

    virtual void Draw(ImDrawData* drawData, CommandList cmdList) = 0;

    /// Registers a texture and returns the handle the UI should use for it, or 0 when the
    /// texture cannot be represented (the caller draws without it in that case).
    virtual uint64_t RegisterTexture(Texture texture) = 0;

    /// Releases a registration. `textureID` is a value, so calling this with an already destroyed
    /// texture handle is safe.
    virtual void ReleaseTexture(const UUID& textureID) = 0;

    /// Releases every registration (shutdown, or when the UI is rebuilt).
    virtual void ReleaseAllTextures() = 0;

    virtual void Shutdown() = 0;
};

/// Owns the UI backend created by `CreateUIRenderBackend`.
using UIRenderBackendPtr = Scope<UIRenderBackendObj>;

/// Creates the UI backend exported by the loaded graphics backend module. Returns nullptr and
/// fills `err` when the backend module is missing or does not provide a UI backend.
UIRenderBackendPtr CreateUIRenderBackend(std::string& err);

} // namespace CZ
