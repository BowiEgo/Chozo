#pragma once

#include "ContentItem.h"
#include <imgui.h>

namespace Chozo {
    
    class ContentSelection
    {
    public:
        ContentSelection() = default;
        ~ContentSelection() = default;

        void OnImGuiRender();
    private:
        bool m_Disabled = false;
        bool m_Dragging = false;
        ImVec2 m_StartPos, m_EndPos;
    };
} // namespace Chozo
