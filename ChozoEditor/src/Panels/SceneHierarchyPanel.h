#pragma once

#include "Chozo.h"
#include "PropertiesPanel.h"

namespace Chozo {

    class SceneHierarchyPanel
    {
    public:
        SceneHierarchyPanel();
        explicit SceneHierarchyPanel(const Ref<Scene>& context);

        void OnImGuiRender();

        void SetSelectedEntity(const Entity& entity) {
            m_SelectionContext = entity;
            PropertiesPanel::SetContext(m_Context);
            PropertiesPanel::SetSelectedEntity(entity);
        }

        Entity GetSelectedEntity() const { return m_SelectionContext; }
        static SceneHierarchyPanel& Get() { return *s_Instance; }

        static void SetContext(const Ref<Scene>& context);
        static Entity DrawAddComponent(Entity entity = Entity(entt::null, nullptr));
    private:
        void DrawEntityNode(Entity entity);
    private:
		static SceneHierarchyPanel* s_Instance;

        Ref<Scene> m_Context;
        Entity m_SelectionContext;
        bool m_CreatingEntity{};
    };
}
