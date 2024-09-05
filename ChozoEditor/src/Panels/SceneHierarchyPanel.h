#pragma once

#include "Chozo.h"

namespace Chozo {

    static std::string s_MeshLeafs[] = {"Geometry", "Material"};

    class SceneHierarchyPanel
    {
    public:
        SceneHierarchyPanel() = default;
        SceneHierarchyPanel(const Ref<Scene> context);

        void SetContext(const Ref<Scene> scene);
        void OnImGuiRender();

        inline void SetSelectedEntity(Entity entity) {
            m_SelectionContext = entity;
            m_SelectionMeshLeaf = -1;
        }
        inline Entity GetSelectedEntity() const { return m_SelectionContext; }
    private:
        void DrawEntityNode(Entity entity);
        void DrawComponents(Entity entity);
        void DrawGeometryProperties(Entity entity);
        void DrawMaterialProperties(Entity entity);
    private:
        Ref<Scene> m_Context;
        Entity m_SelectionContext;
        uint8_t m_SelectionMeshLeaf;
    };
}
