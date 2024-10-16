#pragma once

#include "Chozo.h"

namespace Chozo {

    enum class PropertyType
    {
        None,
        Tag,
        Transform,
        Camera,
        Sprite,
        Circle,
        Mesh,
        SkyLight,
        Light,
        Geometry,
        Material
    };

    class PropertiesPanel
    {
    public:
        PropertiesPanel();
        PropertiesPanel(const Ref<Scene> context);

        void OnImGuiRender();

        static void SetSelectedEntity(Entity entity, std::vector<PropertyType> filter = {
            PropertyType::Tag,
            PropertyType::Transform,
            PropertyType::Camera,
            PropertyType::Sprite,
            PropertyType::Circle,
            PropertyType::Mesh,
            PropertyType::SkyLight,
            PropertyType::Light,
            PropertyType::Geometry,
            PropertyType::Material,
        });
        inline static Entity GetSelectedEntity() { return s_Instance->m_SelectionContext; }
		inline static PropertiesPanel& Get() { return *s_Instance; }
        inline static Ref<Scene> GetContext() { return s_Instance->m_Context; }
        static void SetContext(const Ref<Scene> context);
    private:
        bool Filtered(PropertyType type);
        void DrawEntityNode(Entity entity);
        void DrawComponents(Entity entity);
        void DrawTagProperties(Entity entity);
        void DrawTransformProperties(Entity entity);
        void DrawCameraProperties(Entity entity);
        void DrawSpriteProperties(Entity entity);
        void DrawCircleProperties(Entity entity);
        void DrawMeshProperties(Entity entity);
        void DrawSkyLightProperties(Entity entity);
        void DrawLightProperties(Entity entity);
        void DrawGeometryProperties(Entity entity);
        void DrawMaterialProperties(Entity entity);

        void OpenMaterialPanel(Entity entity);
        void OnSelectedChange(Entity entity);
    private:
		static PropertiesPanel* s_Instance;

        Ref<Scene> m_Context;
        Entity m_SelectionContext;
        std::vector<PropertyType> m_Filter;
        bool m_CreatingEntity;
    };
}
