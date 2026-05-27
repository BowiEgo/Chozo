#pragma once

#include <Runtime/Window/Window.hpp>

#include "SDL3/SDL_video.h"
#include <SDL3/SDL.h>

namespace CZ {

class SDLWindowObj : public WindowObj {
public:
    SDLWindowObj(const WindowSpecifaciton& spec);
    ~SDLWindowObj() override;

    bool Init(std::string& err) override;

    void Shutdown() override;

    void OnUpdate() override;

    bool ShouldClose() const override;

    Extent2D GetSize() const override;

    Extent2D GetFrameBufferSize() const override;

    Extent2D GetFrameBufferScale() const override;

    float GetPixelRatio() const override;

    std::vector<const char*> GetRequiredExtensions(std::string& err) const override;

    WindowHandle GetNativeHandle() const override;

    SDL_Window* GetSDLWindow() const { return static_cast<SDL_Window*>(m_Window); }

    void SetEventPreprocessor(std::function<void(const SDL_Event&)> preprocessor) {
        m_EventPreprocessor = preprocessor;
    }

    void ProcessEventWithPreprocessor(const SDL_Event& event) {
        if (m_EventPreprocessor) m_EventPreprocessor(event);
    }

private:
    std::function<void(const SDL_Event&)> m_EventPreprocessor;
};

} // namespace CZ