#include <Runtime/Window/Window.hpp>

#include <Core/Memory/Memory.hpp>

#include "SDLWindow/SDLWindowObj.hpp"

namespace CZ {

Window Window::Create(const WindowSpecifaciton& spec) {
    return Window(CZ_NEW(MEMORY_USAGE_RUNTIME, SDLWindowObj, spec));
}

DEFINE_HANDLE_DESTROY(WindowObj)

bool Window::Init(std::string& err) { return m_Obj->Init(err); }

void Window::Shutdown() { m_Obj->Shutdown(); }

void Window::OnUpdate() { m_Obj->OnUpdate(); }

bool Window::ShouldClose() const { return m_Obj->ShouldClose(); }

Extent2D Window::GetSize() const { return m_Obj->GetSize(); }

Extent2D Window::GetFrameBufferSize() const { return m_Obj->GetFrameBufferSize(); }

Extent2D Window::GetFrameBufferScale() const { return m_Obj->GetFrameBufferScale(); }

float Window::GetPixelRatio() const { return m_Obj->GetPixelRatio(); }

std::vector<const char*> Window::GetRequiredExtensions(std::string& err) const {
    return m_Obj->GetRequiredExtensions(err);
}

WindowHandle Window::GetNativeHandle() const { return m_Obj->GetNativeHandle(); }

WindowHandle Window::GetWindowWrapper() const { return m_Obj->GetWindowWrapper(); }

void Window::SetEventCallback(const EventCallback& callback) { m_Obj->SetEventCallback(callback); }

void Window::SetVSync(bool enabled) { m_Obj->SetVSync(enabled); }

bool Window::IsVSyncEnabled() const { return m_Obj->IsVSyncEnabled(); }

bool Window::CheckAndResetVSyncDirty() { return m_Obj->CheckAndResetVSyncDirty(); }

} // namespace CZ