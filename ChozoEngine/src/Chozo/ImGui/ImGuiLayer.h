#pragma once

#include "prxpch.h"
#include "Chozo/Layer.h"
#include "imgui.h"
#include "ImGuiKeyCodes.h"

namespace Chozo {
    class CHOZO_API ImGuiLayer : public Layer
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