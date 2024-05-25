#pragma once

#include "prxpch.h"
#include "Parallax/Layer.h"
#include "imgui.h"
#include "ImGuiKeyCodes.h"

namespace Parallax {
    class PARALLAX_API ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer();
        ~ImGuiLayer();

        void OnAttach();
        void OnDetach();
        void OnUpdate();
    private:
    private:
        float m_Time = 0.0f;
    };
}