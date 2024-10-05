#pragma once

#include "ContentItem.h"
#include <imgui.h>

namespace Chozo {
    
    class ContentSelection
    {
    public:
        ContentSelection();
        ~ContentSelection();

        void OnImGuiRender();

        void Clear();
        void Push(Ref<ContentItem> item);
        void Select(Ref<ContentItem> item);
        void Deselect(Ref<ContentItem> item);
        void Deselect(AssetHandle handle);

        inline std::unordered_map<AssetHandle, Ref<ContentItem>> GetSelection() { return m_Selection; }
        inline uint32_t Size() { return m_Selection.size(); }
    private:
        bool m_Selecting = false;
        ImVec2 m_StartPos, m_EndPos;
        std::unordered_map<AssetHandle, Ref<ContentItem>> m_Selection;
    };
} // namespace Chozo
