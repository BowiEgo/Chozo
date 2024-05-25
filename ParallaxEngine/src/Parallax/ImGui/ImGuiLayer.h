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

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnImGuiRender() override;

        void Begin();
        void End();
    private:
        float m_Time = 0.0f;
    };
}