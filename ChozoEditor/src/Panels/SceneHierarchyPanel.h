#pragma once

#include "Chozo.h"

namespace Chozo {

    class SceneHierarchyPanel
    {
    public:
        SceneHierarchyPanel() = default;
        SceneHierarchyPanel(const Ref<Scene> context);

        void SetContext(const Ref<Scene> scene);
        void OnImGuiRender();

        Entity inline GetSelectedEntity() { return m_SelectionContext; }
    private:
        void DrawEntityNode(Entity entity);
        void DrawComponents(Entity entity);
    private:
        Ref<Scene> m_Context;
        Entity m_SelectionContext;
    };
}
