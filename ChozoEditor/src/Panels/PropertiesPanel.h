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
        explicit PropertiesPanel(const Ref<Scene>& context);

        void OnImGuiRender();

        static void SetSelectedEntity(const Entity& entity, const std::vector<PropertyType> &filter = {
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
        static Entity GetSelectedEntity() { return s_Instance->m_SelectionContext; }
		static PropertiesPanel& Get() { return *s_Instance; }
        static Ref<Scene> GetContext() { return s_Instance->m_Context; }
        static void SetContext(const Ref<Scene>& context);
    private:
        bool Filtered(PropertyType type);
        void DrawComponents(const Entity& entity);
        void DrawTagProperties(Entity entity) const;
        void DrawTransformProperties(const Entity& entity) const;
        void DrawCameraProperties(const Entity& entity);
        void DrawSpriteProperties(const Entity& entity) const;
        void DrawCircleProperties(const Entity& entity);
        void DrawMeshProperties(const Entity& entity);
        void DrawSkyLightProperties(const Entity& entity);
        void DrawLightProperties(const Entity& entity);
        void DrawGeometryProperties(const Entity& entity);
        void DrawMaterialProperties(const Entity& entity);

        void OnSelectedChange(const Entity& entity);
    private:
		static PropertiesPanel* s_Instance;

        Ref<Scene> m_Context;
        Entity m_SelectionContext;
        std::vector<PropertyType> m_Filter;
        bool m_CreatingEntity{};
    };
}
