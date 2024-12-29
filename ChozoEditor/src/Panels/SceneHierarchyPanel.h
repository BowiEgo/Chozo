#pragma once

#include <utility>

#include "Chozo.h"
#include "PropertiesPanel.h"

namespace Chozo {

    class SceneHierarchyPanel
    {
        using Callback = std::function<void(const Entity& entity)>;
    public:
        SceneHierarchyPanel();
        explicit SceneHierarchyPanel(const Ref<Scene>& context);

        void OnImGuiRender();

        void SetSelectedEntity(const Entity& entity) {
            m_SelectedEntity = entity;
            PropertiesPanel::SetContext(m_Context);
            PropertiesPanel::SetSelectedEntity(entity);
            if (m_Callback)
                m_Callback(entity);
        }

        Entity GetSelectedEntity() const { return m_SelectedEntity; }
        static SceneHierarchyPanel& Get() { return *s_Instance; }

        static void SetContext(const Ref<Scene>& context);
        static Entity DrawAddComponent(Entity entity = Entity(entt::null, nullptr));

        static void RegisterOnSelectedChange(const Callback &callback) {
            s_Instance->m_Callback = callback;
        }
    private:
        void DrawEntityNode(Entity entity);
    private:
		static SceneHierarchyPanel* s_Instance;

        Ref<Scene> m_Context;
        Entity m_SelectedEntity;
        bool m_CreatingEntity{};

        Callback m_Callback;
    };
}
