#pragma once

#include "Chozo.h"
#include "PropertiesPanel.h"

namespace Chozo {

    class SceneHierarchyPanel
    {
    public:
        SceneHierarchyPanel();
        SceneHierarchyPanel(const Ref<Scene> context);

        void OnImGuiRender();

        inline void SetSelectedEntity(Entity entity) {
            m_SelectionContext = entity;
            PropertiesPanel::SetContext(m_Context);
            PropertiesPanel::SetSelectedEntity(entity);
        }
        inline Entity GetSelectedEntity() const { return m_SelectionContext; }
    public:
		inline static SceneHierarchyPanel& Get() { return *s_Instance; }
        static void SetContext(const Ref<Scene> context);
        static Entity DrawAddComponent(Entity entity = Entity(entt::null, nullptr));
        static void DrawMaterialProperties(Entity entity);
    private:
        void DrawEntityNode(Entity entity);
        void DrawComponents(Entity entity);
        void DrawGeometryProperties(Entity entity);
    private:
		static SceneHierarchyPanel* s_Instance;

        Ref<Scene> m_Context;
        Entity m_SelectionContext;
        bool m_CreatingEntity;
    };
}
