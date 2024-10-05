#pragma once

#include "czpch.h"
#include "Chozo/Core/Layer.h"
#include "imgui.h"
#include "ImGuiKeyCodes.h"

namespace Chozo {

    inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs)
	{
		return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
	}

    inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs)
    {
        return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y);
    }

    class CHOZO_API ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer();
        ~ImGuiLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnImGuiRender() override;
        virtual void OnEvent(Event& e) override;
        void BlockEvents(bool block) { m_BlockEvents = block; }

        void Begin();
        void End();
        
        void SetDarkThemeColors();
    private:
        bool m_BlockEvents = false;
    };
}