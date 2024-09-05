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
}

namespace Chozo {

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
                    auto& tc = deserializedEntity.GetComponent<TransformComponent>();
                    tc.Translation = transformComponent["Translation"].as<glm::vec3>();
                    tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
                    tc.Scale = transformComponent["Scale"].as<glm::vec3>();
                }

                auto cameraComponent = entity["CameraComponent"];
                if (cameraComponent)
                {
                    auto& cc = deserializedEntity.AddComponent<CameraComponent>();

                    const auto& cameraProps = cameraComponent["Camera"];
                    cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());

                    cc.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveVerticalFOV"].as<float>());
                    cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
                    cc.Camera.SetOrthographicFarClip(cameraProps["PerspectiveFar"].as<float>());

                    cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
                    cc.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
                    cc.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

                    cc.Primary = cameraComponent["Primary"].as<bool>();
                    cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
                }

                auto spriteRendererComponent = entity["SpriteRendererComponent"];
                if (spriteRendererComponent)
                {
                    auto& sc = deserializedEntity.AddComponent<SpriteRendererComponent>();
                    sc.Color = spriteRendererComponent["Color"].as<glm::vec4>();
                }

                auto circleRendererComponent = entity["CircleRendererComponent"];
                if (circleRendererComponent)
                {
                    auto& sc = deserializedEntity.AddComponent<CircleRendererComponent>();
                    sc.Color = circleRendererComponent["Color"].as<glm::vec4>();
                    sc.Thickness = circleRendererComponent["Thickness"].as<float>();
                    sc.Fade = circleRendererComponent["Fade"].as<float>();
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
                            geom = std::make_shared<BoxGeometry>(
                                geometry["Width"].as<float>(),
                                geometry["Height"].as<float>(),
                                geometry["Depth"].as<float>(),
                                geometry["WidthSegments"].as<uint32_t>(),
                                geometry["HeightSegments"].as<uint32_t>(),
                                geometry["DepthSegments"].as<uint32_t>()
                            );
                            break;
                        case GeometryType::Sphere:
                            geom = std::make_shared<SphereGeometry>(
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
                    auto& mc = deserializedEntity.AddComponent<MeshComponent>(geom, meshType);
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