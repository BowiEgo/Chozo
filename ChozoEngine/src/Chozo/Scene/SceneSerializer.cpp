#include "SceneSerializer.h"

// #include <fstream>

#include <yaml-cpp/yaml.h>

#include "Chozo/Core/Base.h"
#include "Chozo/Renderer/Geometry/BoxGeometry.h"
#include "Chozo/Renderer/Geometry/SphereGeometry.h"
#include "Entity.h"
#include "Components.h"

namespace YAML {
    
    template<>
    struct convert<glm::vec3>
    {
        static Node encode(const glm::vec3& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            return node;
        }

        static bool decode(const Node& node, glm::vec3& rhs)
        {
            if (!node.IsSequence() || node.size() != 3)
                return false;
            
            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };

    template<>
    struct convert<glm::vec4>
    {
        static Node encode(const glm::vec4& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            return node;
        }

        static bool decode(const Node& node, glm::vec4& rhs)
        {
            if (!node.IsSequence() || node.size() != 4)
                return false;
            
            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            rhs.w = node[3].as<float>();
            return true;
        }
    };
        
    template <>
    struct convert<glm::mat3>
    {
        static Node encode(const glm::mat3& rhs)
        {
            Node node;
            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    node.push_back(rhs[i][j]);
                }
            }
            return node;
        }

        static bool decode(const Node& node, glm::mat3& rhs)
        {
            if (!node.IsSequence() || node.size() != 9)
                return false;

            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    rhs[i][j] = node[i * 3 + j].as<float>();
                }
            }

            return true;
        }
    };

    template <>
    struct convert<glm::mat4>
    {
        static Node encode(const glm::mat4& rhs)
        {
            Node node;
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    node.push_back(rhs[i][j]);
                }
            }
            return node;
        }

        static bool decode(const Node& node, glm::mat4& rhs)
        {
            if (!node.IsSequence() || node.size() != 16)
                return false;

            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    rhs[i][j] = node[i * 4 + j].as<float>();
                }
            }

            return true;
        }
    };
}

namespace Chozo {

    YAML::Emitter& operator<<(YAML::Emitter& out, const std::pair<float, float>& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.first << v.second << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const std::tuple<float, float, float>& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << std::get<0>(v) << std::get<1>(v) << std::get<2>(v) << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const std::tuple<float, float, float, float>& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << std::get<0>(v) << std::get<1>(v) << std::get<2>(v) << std::get<3>(v) << YAML::EndSeq;
        return out;
    }


    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::mat3& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq;
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                out << v[i][j];
        out << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::mat4& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq;
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                out << v[i][j];
        out << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const std::vector<glm::mat4>& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq;
        for (const auto& mat : v) {
            for (int i = 0; i < 4; ++i)
                for (int j = 0; j < 4; ++j)
                    out << mat[i][j];
        }
        out << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const std::vector<int>& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq;
        for (const auto& val : v) {
            out << val;
        }
        out << YAML::EndSeq;
        return out;
    }

    SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
        : m_Scene(scene)
    {
    }

    static void SerializeEntity(YAML::Emitter& out, Entity entity)
    {
        CZ_CORE_ASSERT(entity.HasComponent<IDComponent>(), "");

        out << YAML::BeginMap;
        out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

        {
            out << YAML::Key << "TagComponent";
            out << YAML::BeginMap;

            auto& tag = entity.GetComponent<TagComponent>().Tag;
            out << YAML::Key << "Tag" << YAML::Value << tag;

            out << YAML::EndMap;
        }

        if (entity.HasComponent<TransformComponent>())
        {
            out << YAML::Key << "TransformComponent";
            out << YAML::BeginMap;

            auto& tc = entity.GetComponent<TransformComponent>();
            out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
            out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
            out << YAML::Key << "Scale" << YAML::Value << tc.Scale;

            out << YAML::EndMap;
        }

        if (entity.HasComponent<CameraComponent>())
        {
            out << YAML::Key << "CameraComponent";
            out << YAML::BeginMap;

            auto& cc = entity.GetComponent<CameraComponent>();
            auto& camera = cc.Camera;

            out << YAML::Key << "Camera" << YAML::Value;
            out << YAML::BeginMap;
            out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
            out << YAML::Key << "PerspectiveVerticalFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
            out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
            out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
            out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
            out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
            out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
            out << YAML::EndMap;

            out << YAML::Key << "Primary" << YAML::Value << cc.Primary;
            out << YAML::Key << "FixedAspectRatio" << YAML::Value << cc.FixedAspectRatio;
            
            out << YAML::EndMap;
        }

        if (entity.HasComponent<SkyLightComponent>())
        {
            out << YAML::Key << "SkyLightComponent";
            out << YAML::BeginMap;

            auto& sc = entity.GetComponent<SkyLightComponent>();
            out << YAML::Key << "Intensity" << YAML::Value << sc.Intensity;
            out << YAML::Key << "Lod" << YAML::Value << sc.Lod;
            out << YAML::Key << "Dynamic" << YAML::Value << sc.Dynamic;
            out << YAML::Key << "SourcePath" << YAML::Value << sc.SourcePath;
            out << YAML::Key << "TurbidityAzimuthInclination" << YAML::Value << sc.TurbidityAzimuthInclination;

            out << YAML::EndMap;
        }

        if (entity.HasComponent<SpriteRendererComponent>())
        {
            out << YAML::Key << "SpriteRendererComponent";
            out << YAML::BeginMap;

            auto& sp = entity.GetComponent<SpriteRendererComponent>();
            out << YAML::Key << "Color" << YAML::Value << sp.Color;

            out << YAML::EndMap;
        }

        if (entity.HasComponent<CircleRendererComponent>())
        {
            out << YAML::Key << "CircleRendererComponent";
            out << YAML::BeginMap;

            auto& cp = entity.GetComponent<CircleRendererComponent>();
            out << YAML::Key << "Color" << YAML::Value << cp.Color;
            out << YAML::Key << "Thickness" << YAML::Value << cp.Thickness;
            out << YAML::Key << "Fade" << YAML::Value << cp.Fade;

            out << YAML::EndMap;
        }

        if (entity.HasComponent<MeshComponent>())
        {
            out << YAML::Key << "MeshComponent";
            out << YAML::BeginMap;

            auto& mc = entity.GetComponent<MeshComponent>();
            out << YAML::Key << "Type" << YAML::Value << (int)mc.Type;

            auto& meshSrc = mc.MeshSrc;
            out << YAML::Key << "Geometry" << YAML::Value;
            if (BoxGeometry* box = dynamic_cast<BoxGeometry*>(meshSrc.get()))
            {
                out << YAML::BeginMap;
                out << YAML::Key << "Type" << YAML::Value << int(GeometryType::Box);
                out << YAML::Key << "Width" << YAML::Value << box->GetWidth();
                out << YAML::Key << "Height" << YAML::Value << box->GetHeight();
                out << YAML::Key << "Depth" << YAML::Value << box->GetDepth();
                out << YAML::Key << "WidthSegments" << YAML::Value << box->GetWidthSegments();
                out << YAML::Key << "HeightSegments" << YAML::Value << box->GetHeightSegments();
                out << YAML::Key << "DepthSegments" << YAML::Value << box->GetDepthSegments();
                out << YAML::EndMap;
            }
            if (SphereGeometry* sphere = dynamic_cast<SphereGeometry*>(meshSrc.get()))
            {
                out << YAML::BeginMap;
                out << YAML::Key << "Type" << YAML::Value << int(GeometryType::Sphere);
                out << YAML::Key << "Radius" << YAML::Value << sphere->GetRadius();
                out << YAML::Key << "PhiLength" << YAML::Value << sphere->GetPhiLength();
                out << YAML::Key << "PhiStart" << YAML::Value << sphere->GetPhiStart();
                out << YAML::Key << "ThetaStart" << YAML::Value << sphere->GetThetaStart();
                out << YAML::Key << "ThetaLength" << YAML::Value << sphere->GetThetaLength();
                out << YAML::Key << "WidthSegments" << YAML::Value << sphere->GetWidthSegments();
                out << YAML::Key << "HeightSegments" << YAML::Value << sphere->GetHeightSegments();
                out << YAML::EndMap;
            }

            out << YAML::Key << "Material" << YAML::Value;
            out << YAML::BeginMap;
            out << YAML::Key << "Name" << YAML::Value << mc.MaterialInstance->GetName();
            for (auto& pair : mc.MaterialInstance->GetUniforms())
            {
                std::visit([&](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, glm::mat3>)
                    {}
                    if constexpr (std::is_same_v<T, glm::mat4>)
                    {}
                    else
                    {
                        out << YAML::Key << pair.first << YAML::Value << arg;
                    }
                }, pair.second);
            }
            out << YAML::EndMap;

            out << YAML::EndMap;
        }

        if (entity.HasComponent<DirectionalLightComponent>())
        {
            out << YAML::Key << "DirectionalLightComponent";
            out << YAML::BeginMap;

            auto& lc = entity.GetComponent<DirectionalLightComponent>();
            out << YAML::Key << "Direction" << YAML::Value << lc.Direction;
            out << YAML::Key << "Color" << YAML::Value << lc.Color;
            out << YAML::Key << "Intensity" << YAML::Value << lc.Intensity;

            out << YAML::EndMap;
        }

        if (entity.HasComponent<PointLightComponent>())
        {
            out << YAML::Key << "PointLightComponent";
            out << YAML::BeginMap;

            auto& pc = entity.GetComponent<PointLightComponent>();
            out << YAML::Key << "Color" << YAML::Value << pc.Color;
            out << YAML::Key << "Intensity" << YAML::Value << pc.Intensity;

            out << YAML::EndMap;
        }

        if (entity.HasComponent<SpotLightComponent>())
        {
            out << YAML::Key << "SpotLightComponent";
            out << YAML::BeginMap;

            auto& sc = entity.GetComponent<SpotLightComponent>();
            out << YAML::Key << "Intensity" << YAML::Value << sc.Intensity;
            out << YAML::Key << "Direction" << YAML::Value << sc.Direction;
            out << YAML::Key << "AngleAttenuation" << YAML::Value << sc.AngleAttenuation;
            out << YAML::Key << "Color" << YAML::Value << sc.Color;
            out << YAML::Key << "Angle" << YAML::Value << sc.Angle;

            out << YAML::EndMap;
        }

        out << YAML::EndMap;
    }

    void SceneSerializer::Serialize(const std::string& filepath)
    {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << "Untitled";
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
        m_Scene->m_Registry.view<TagComponent>().each([&](auto entityID, TagComponent& tc)
        {
            Entity entity = { entityID, m_Scene.get() };
            if (!entity)
                return;
            
            SerializeEntity(out, entity);
        });
        out << YAML::EndSeq;
        out << YAML::EndMap;

        // Check if filepath directories exist. If not, create them.
        std::filesystem::path dirPath(filepath);
        if (!(std::filesystem::exists(dirPath) && std::filesystem::is_directory(dirPath)))
            std::filesystem::create_directories(dirPath.parent_path().string());

        std::ofstream fout(filepath);
        fout << out.c_str();
    }

    void SceneSerializer::SerializeRuntime(const std::string& filepath)
    {
        // Not implemented
        CZ_CORE_ASSERT(false, "API Not implemented!");
    }

    bool SceneSerializer::Deserialize(const std::string& filepath)
    {
        std::ifstream stream(filepath);
        if (!stream.is_open()) {
            CZ_CORE_ERROR("Failed to open file: %s", filepath.c_str());
            return false;
        }
        std::stringstream strStream;
        strStream << stream.rdbuf();

        YAML::Node data;
        try {
            data = YAML::Load(strStream.str());
        } catch (const YAML::ParserException& e) {
            CZ_CORE_ERROR("Failed to parse YAML file: %s", e.what());
            return false;
        } catch (const YAML::BadFile& e) {
            CZ_CORE_ERROR("Failed to load YAML file: %s", e.what());
            return false;
        } catch (const std::exception& e) {
            CZ_CORE_ERROR("An error occurred while loading the YAML file: %s", e.what());
            return false;
        }

        try {
            if (!data["Scene"])
            {
                CZ_CORE_ERROR("YAML file does not contain a 'Scene' node");
                return false;
            }
        } catch (const YAML::BadSubscript& e) {
            CZ_CORE_ERROR("Failed to load YAML file: %s", e.what());
            return false;
        }
        
        std::string sceneName = data["Scene"].as<std::string>();
        CZ_CORE_TRACE("Deserializing scene {0}", sceneName);

        auto entities = data["Entities"];
        if (entities)
        {
            for (auto entity : entities)
            {
                uint64_t uuid = entity["Entity"].as<uint64_t>();

                std::string name;
                auto tagComponent = entity["TagComponent"];
                if (tagComponent)
                    name = tagComponent["Tag"].as<std::string>();

                CZ_CORE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);

                Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, name);

                auto transformComponent = entity["TransformComponent"];
                if (transformComponent)
                {
                    auto& comp = deserializedEntity.GetComponent<TransformComponent>();
                    comp.Translation = transformComponent["Translation"].as<glm::vec3>();
                    comp.Rotation = transformComponent["Rotation"].as<glm::vec3>();
                    comp.Scale = transformComponent["Scale"].as<glm::vec3>();
                }

                auto cameraComponent = entity["CameraComponent"];
                if (cameraComponent)
                {
                    auto& comp = deserializedEntity.AddComponent<CameraComponent>();

                    const auto& cameraProps = cameraComponent["Camera"];
                    comp.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());

                    comp.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveVerticalFOV"].as<float>());
                    comp.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
                    comp.Camera.SetOrthographicFarClip(cameraProps["PerspectiveFar"].as<float>());

                    comp.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
                    comp.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
                    comp.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

                    comp.Primary = cameraComponent["Primary"].as<bool>();
                    comp.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
                }

                auto skyLightComponent = entity["SkyLightComponent"];
                if (skyLightComponent)
                {
                    auto& comp = deserializedEntity.AddComponent<SkyLightComponent>();
                    
                    comp.Intensity = skyLightComponent["Intensity"].as<float>();
                    comp.Lod = skyLightComponent["Lod"].as<float>();
                    comp.Dynamic = skyLightComponent["Dynamic"].as<bool>();
                    comp.SourcePath = skyLightComponent["SourcePath"].as<std::string>();
                    comp.TurbidityAzimuthInclination = skyLightComponent["TurbidityAzimuthInclination"].as<glm::vec3>();
                }

                auto spriteRendererComponent = entity["SpriteRendererComponent"];
                if (spriteRendererComponent)
                {
                    auto& comp = deserializedEntity.AddComponent<SpriteRendererComponent>();
                    comp.Color = spriteRendererComponent["Color"].as<glm::vec4>();
                }

                auto circleRendererComponent = entity["CircleRendererComponent"];
                if (circleRendererComponent)
                {
                    auto& comp = deserializedEntity.AddComponent<CircleRendererComponent>();
                    comp.Color = circleRendererComponent["Color"].as<glm::vec4>();
                    comp.Thickness = circleRendererComponent["Thickness"].as<float>();
                    comp.Fade = circleRendererComponent["Fade"].as<float>();
                }

                auto meshComponent = entity["MeshComponent"];
                Ref<Geometry> geom;
                if (meshComponent)
                {
                    auto geometry = meshComponent["Geometry"];
                    auto type = geometry["Type"].as<int>();
                    auto geomType = GeometryType(geometry["Type"].as<int>());

                    switch (geomType) {
                        case GeometryType::Box:
                            geom = Ref<BoxGeometry>::Create(
                                geometry["Width"].as<float>(),
                                geometry["Height"].as<float>(),
                                geometry["Depth"].as<float>(),
                                geometry["WidthSegments"].as<uint32_t>(),
                                geometry["HeightSegments"].as<uint32_t>(),
                                geometry["DepthSegments"].as<uint32_t>()
                            );
                            break;
                        case GeometryType::Sphere:
                            geom = Ref<SphereGeometry>::Create(
                                geometry["Radius"].as<float>(),
                                geometry["WidthSegments"].as<uint32_t>(),
                                geometry["HeightSegments"].as<uint32_t>(),
                                geometry["PhiStart"].as<float>(),
                                geometry["PhiLength"].as<float>(),
                                geometry["ThetaStart"].as<float>(),
                                geometry["ThetaLength"].as<float>()
                            );
                            break;
                        default:
                            break;
                    }

                    geom->SetEntityID(deserializedEntity);
                    auto meshType = MeshType(meshComponent["Type"].as<int>());

                    auto material = meshComponent["Material"];
                    Ref<Material> mate = Material::Create(material["Name"].as<std::string>());
                    for (const auto& pair : material)
                    {
                        std::string key = pair.first.as<std::string>();
                        if (key == "Name")
                            continue;

                        const YAML::Node& valueNode = pair.second;
                        UniformValue value;

                        if (valueNode.IsScalar()) {
                            try {
                                if (valueNode.Tag() == "tag:yaml.org,2002:bool") {
                                    value = valueNode.as<bool>();
                                } else if (valueNode.Tag() == "tag:yaml.org,2002:int") {
                                    value = valueNode.as<int>();
                                } else if (valueNode.Tag() == "tag:yaml.org,2002:float" || valueNode.Tag() == "tag:yaml.org,2002:float") {
                                    value = valueNode.as<float>();
                                } else {
                                    try {
                                        value = valueNode.as<float>();
                                    } catch (const YAML::TypedBadConversion<float>&) {
                                        // value = valueNode.as<std::string>();
                                    }
                                }
                            } catch (const YAML::Exception& e) {
                                CZ_CORE_ERROR("YAML parsing error: {0}", e.what());
                            }
                        } else if (valueNode.IsSequence()) {
                            if (valueNode.size() == 2) {
                                value = glm::vec2(valueNode[0].as<float>(), valueNode[1].as<float>());
                            } else if (valueNode.size() == 3) {
                                value = glm::vec3(valueNode[0].as<float>(), valueNode[1].as<float>(), valueNode[2].as<float>());
                            } else if (valueNode.size() == 4) {
                                value = glm::vec4(valueNode[0].as<float>(), valueNode[1].as<float>(), valueNode[2].as<float>(), valueNode[3].as<float>());
                            }
                        }

                        mate->Set(key, value);
                    }

                    auto& mc = deserializedEntity.AddComponent<MeshComponent>(geom, meshType, mate);
                }
            
                auto dirLightComponent = entity["DirectionalLightComponent"];
                if (dirLightComponent)
                {
                    auto& comp = deserializedEntity.AddComponent<DirectionalLightComponent>();
                    comp.Direction = dirLightComponent["Direction"].as<glm::vec3>();
                    comp.Color = dirLightComponent["Color"].as<glm::vec3>();
                    comp.Intensity = dirLightComponent["Intensity"].as<float>();
                }

                auto pointLightComponent = entity["PointLightComponent"];
                if (pointLightComponent)
                {
                    auto& comp = deserializedEntity.AddComponent<PointLightComponent>();
                    comp.Color = pointLightComponent["Color"].as<glm::vec3>();
                    comp.Intensity = pointLightComponent["Intensity"].as<float>();
                }

                auto spotLightComponent = entity["SpotLightComponent"];
                if (spotLightComponent)
                {
                    auto& comp = deserializedEntity.AddComponent<SpotLightComponent>();
                    comp.Intensity = spotLightComponent["Intensity"].as<float>();
                    comp.Direction = spotLightComponent["Direction"].as<glm::vec3>();
                    comp.AngleAttenuation = spotLightComponent["AngleAttenuation"].as<float>();
                    comp.Color = spotLightComponent["Color"].as<glm::vec3>();
                    comp.Angle = spotLightComponent["Angle"].as<float>();
                }
            }
        }

        return true;
    }

    bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
    {
        // Not implemented
        CZ_CORE_ASSERT(false, "API Not implemented!");
        return false;
    }
}