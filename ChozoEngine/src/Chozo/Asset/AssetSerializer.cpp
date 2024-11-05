#include "AssetSerializer.h"

#include "Chozo/Core/Application.h"
#include "Chozo/Project/Project.h"
#include "Chozo/Renderer/Renderer.h"
#include "Chozo/Renderer/Texture.h"
#include "Chozo/Renderer/Material.h"
#include "Chozo/Renderer/Geometry/BoxGeometry.h"
#include "Chozo/Renderer/Geometry/SphereGeometry.h"

#include "Chozo/Utilities/SerializationUtils.h"

#include "Chozo/Scene/Entity.h"
#include "Chozo/Scene/Components.h"

#include <yaml-cpp/yaml.h>

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

    static bool PrepareDeserialize(AssetMetadata &metadata, fs::path& dest)
    {
        fs::path path(metadata.FilePath);
        fs::path filepath = Utils::File::GetAssetDirectory() / path;
        dest = filepath.parent_path() / (filepath.filename().string() + ".asset");

        if (!fs::exists(dest))
        {
            CZ_CORE_ERROR("Asset file doesn't exist at {}", dest.string());
            return false;
        }

        return true;
    }

    //==============================================================================
	/// SceneSerializer
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
            out << YAML::Key << "Source" << YAML::Value << sc.Source;
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

            MeshComponent& mc = entity.GetComponent<MeshComponent>();
            out << YAML::Key << "Type" << YAML::Value << (int)mc.Type;

            auto mesh = mc.MeshInstance;
            auto meshSrc = mc.MeshInstance->GetMeshSource();
            out << YAML::Key << "Geometry" << YAML::Value;
            if (BoxGeometry* box = dynamic_cast<BoxGeometry*>(mesh.get()))
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
            if (SphereGeometry* sphere = dynamic_cast<SphereGeometry*>(mesh.get()))
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
            out << YAML::Key << "Handle" << YAML::Value << mc.MaterialHandle;
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

    uint64_t SceneSerializer::Serialize(const AssetMetadata &metadata, Ref<Asset> &asset) const
    {
        fs::path path(metadata.FilePath);
        fs::path filepath = Utils::File::GetAssetDirectory() / path;
        fs::path dest = filepath.parent_path() / (filepath.filename().string() + ".asset");

        FileStreamWriter stream(dest);
        SceneFile file;

        // Write header
        file.Header.CreateAt = metadata.CreateAt;
        file.Header.ModifiedAt = metadata.ModifiedAt;
		stream.WriteRaw<SceneFile::FileHeader>(file.Header);

        // Write YAML
        Ref<Scene> scene = asset.As<Scene>();
		std::string yamlString = SerializeToYAML(scene);

        uint64_t start = stream.GetStreamPosition();
		stream.WriteString(yamlString);
        uint64_t size = stream.GetStreamPosition() - start;

#if 0
        fs::path dest2 = filepath.parent_path() / (filepath.filename().string() + ".txt");
        fs::path dirPath(dest2);
        if (!(fs::exists(dirPath) && fs::is_directory(dirPath)))
            fs::create_directories(dirPath.parent_path().string());

        std::ofstream fout(filepath);
        fout << yamlString.c_str();
#endif
        return size;
    }

    Ref<Asset> SceneSerializer::Deserialize(AssetMetadata &metadata) const
    {
        fs::path dest;
        if (!PrepareDeserialize(metadata, dest))
            return nullptr;

        FileStreamReader stream(dest);
		SceneFile file;

        // Read header
		stream.ReadRaw<SceneFile::FileHeader>(file.Header);
        metadata.CreateAt = file.Header.CreateAt;
        metadata.ModifiedAt = file.Header.ModifiedAt;

        // Read YAML
		std::string yamlString;
		stream.ReadString(yamlString);

		Ref<Scene> scene = DeserializeFromYAML(yamlString);
        return scene;
    }

    std::string SceneSerializer::SerializeToYAML(Ref<Scene> scene) const
    {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << "Untitled";
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
        scene->m_Registry.view<TagComponent>().each([&](auto entityID, TagComponent& tc)
        {
            Entity entity = { entityID, scene.get() };
            if (!entity)
                return;
            
            SerializeEntity(out, entity);
        });
        out << YAML::EndSeq;
        out << YAML::EndMap;

		return std::string(out.c_str());
    }

    Ref<Scene> SceneSerializer::DeserializeFromYAML(const std::string &yamlString) const
    {
        YAML::Node root = YAML::Load(yamlString);
		YAML::Node sceneNode = root["Scene"];
        YAML::Node entitiesNode = root["Entities"];

        std::string sceneName = sceneNode.as<std::string>();
        CZ_CORE_TRACE("Deserializing scene {0}", sceneName);

        auto scene = Ref<Scene>::Create();

        if (entitiesNode)
        {
            for (auto entity : entitiesNode)
            {
                uint64_t uuid = entity["Entity"].as<uint64_t>();

                std::string name;
                auto tagComponent = entity["TagComponent"];
                if (tagComponent)
                    name = tagComponent["Tag"].as<std::string>();

                CZ_CORE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);

                Entity deserializedEntity = scene->CreateEntityWithUUID(uuid, name);

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
                    comp.Source = skyLightComponent["Source"].as<uint64_t>();
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
                Ref<Geometry> geometry;
                if (meshComponent)
                {
                    auto geometryNode = meshComponent["Geometry"];
                    auto type = geometryNode["Type"].as<int>();
                    auto geomType = GeometryType(geometryNode["Type"].as<int>());

                    switch (geomType) {
                        case GeometryType::Box:
                            geometry = Geometry::Create<BoxGeometry>(
                                geometryNode["Width"].as<float>(),
                                geometryNode["Height"].as<float>(),
                                geometryNode["Depth"].as<float>(),
                                geometryNode["WidthSegments"].as<uint32_t>(),
                                geometryNode["HeightSegments"].as<uint32_t>(),
                                geometryNode["DepthSegments"].as<uint32_t>()
                            );
                            break;
                        case GeometryType::Sphere:
                            geometry = Geometry::Create<SphereGeometry>(
                                geometryNode["Radius"].as<float>(),
                                geometryNode["WidthSegments"].as<uint32_t>(),
                                geometryNode["HeightSegments"].as<uint32_t>(),
                                geometryNode["PhiStart"].as<float>(),
                                geometryNode["PhiLength"].as<float>(),
                                geometryNode["ThetaStart"].as<float>(),
                                geometryNode["ThetaLength"].as<float>()
                            );
                            break;
                        default:
                            break;
                    }

                    auto meshType = MeshType(meshComponent["Type"].as<int>());
                    auto materialNode = meshComponent["Material"];
                    auto materialHandle = materialNode["Handle"].as<uint64_t>();

                    auto& mc = deserializedEntity.AddComponent<MeshComponent>(geometry, 0, materialHandle);
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

        return scene;
    }

    //==============================================================================
	/// TextureSerializer
    uint64_t TextureSerializer::Serialize(const AssetMetadata& metadata, Ref<Asset>& asset) const
    {
        fs::path path(metadata.FilePath);
        fs::path filepath = Utils::File::GetAssetDirectory() / path;
        fs::path dest = filepath.parent_path() / (filepath.filename().string() + ".asset");

        FileStreamWriter stream(dest);
		uint64_t start = stream.GetStreamPosition();

        TextureFile file;
        Ref<Texture2D> texture = asset.As<Texture2D>();

        // Write header
        file.Header.CreateAt = metadata.CreateAt;
        file.Header.ModifiedAt = metadata.ModifiedAt;
		stream.WriteRaw<TextureFile::FileHeader>(file.Header);

        // Leave space for Metadata
		uint64_t metadataAbsolutePosition = stream.GetStreamPosition();
		stream.WriteZero(sizeof(TextureFile::Metadata));
        
        file.Data.Format = (uint16_t)texture->GetSpecification().Format;
        file.Data.Samples = texture->GetSpecification().Samples;
        file.Data.Mipmap = (uint8_t)texture->GetSpecification().Mipmap;
        file.Data.Width = texture->GetWidth();
        file.Data.Height = texture->GetHeight();
        file.Data.MinFilter = (uint16_t)texture->GetSpecification().MinFilter;
        file.Data.MagFilter = (uint16_t)texture->GetSpecification().MagFilter;
        file.Data.WrapR = (uint16_t)texture->GetSpecification().WrapR;
        file.Data.WrapS = (uint16_t)texture->GetSpecification().WrapS;
        file.Data.WrapT = (uint16_t)texture->GetSpecification().WrapT;

        // Write buffer
		Buffer buffer;
        texture->CopyToHostBuffer(buffer);
		stream.WriteBuffer(buffer);

        buffer.Release();

        // Write metadata
		uint64_t endOfStream = stream.GetStreamPosition();
		stream.SetStreamPosition(metadataAbsolutePosition);
        stream.WriteRaw(file.Data);
		stream.SetStreamPosition(endOfStream);

        uint64_t size = stream.GetStreamPosition() - start;

        return size;
    }

    Ref<Asset> TextureSerializer::Deserialize(AssetMetadata& metadata) const
    {
        fs::path dest;
        if (!PrepareDeserialize(metadata, dest))
            return nullptr;

        FileStreamReader stream(dest);
		uint64_t streamOffset = stream.GetStreamPosition();

        TextureFile file;

        // Read header
        stream.ReadRaw<TextureFile::FileHeader>(file.Header);
        metadata.CreateAt = file.Header.CreateAt;
        metadata.ModifiedAt = file.Header.ModifiedAt;

        // Read textureFile metadata
        stream.ReadRaw<TextureFile::Metadata>(file.Data);

        Texture2DSpecification spec;
		spec.Format = (ImageFormat)file.Data.Format;
		spec.Samples = file.Data.Samples;
		spec.Width = file.Data.Width;
		spec.Height = file.Data.Height;
		spec.Mipmap = (bool)file.Data.Mipmap;
		spec.MinFilter = (ImageParameter)file.Data.MinFilter;
		spec.MagFilter = (ImageParameter)file.Data.MagFilter;
		spec.WrapR = (ImageParameter)file.Data.WrapR;
		spec.WrapS = (ImageParameter)file.Data.WrapS;
		spec.WrapT = (ImageParameter)file.Data.WrapT;

        // Read buffer
        Buffer buffer;
        stream.ReadBuffer(buffer);

        Ref<Texture2D> texture = Texture2D::Create(buffer, spec);

        buffer.Release();

        return texture;
    }

    //==============================================================================
	/// MaterialSerializer
    uint64_t MaterialSerializer::Serialize(const AssetMetadata &metadata, Ref<Asset> &asset) const
    {
        fs::path path(metadata.FilePath);
        fs::path filepath = Utils::File::GetAssetDirectory() / path;
        fs::path dest = filepath.parent_path() / (filepath.filename().string() + ".asset");

        FileStreamWriter stream(dest);
        MaterialFile file;

        // Write header
        file.Header.CreateAt = metadata.CreateAt;
        file.Header.ModifiedAt = metadata.ModifiedAt;
		stream.WriteRaw<MaterialFile::FileHeader>(file.Header);

        // Write YAML
        Ref<Material> material = asset.As<Material>();
		std::string yamlString = SerializeToYAML(material);

        uint64_t start = stream.GetStreamPosition();
		stream.WriteString(yamlString);
        uint64_t size = stream.GetStreamPosition() - start;

#if 0
        fs::path dest2 = filepath.parent_path() / (filepath.filename().string() + ".txt");
        fs::path dirPath(dest2);
        if (!(fs::exists(dirPath) && fs::is_directory(dirPath)))
            fs::create_directories(dirPath.parent_path().string());

        std::ofstream fout(filepath);
        fout << yamlString.c_str();
#endif
        return size;
    }

    Ref<Asset> MaterialSerializer::Deserialize(AssetMetadata &metadata) const
    {
        fs::path dest;
        if (!PrepareDeserialize(metadata, dest))
            return nullptr;

        FileStreamReader stream(dest);
        MaterialFile file;

        // Read header
		stream.ReadRaw<MaterialFile::FileHeader>(file.Header);
        metadata.CreateAt = file.Header.CreateAt;
        metadata.ModifiedAt = file.Header.ModifiedAt;

        // Read YAML
		std::string yamlString;
		stream.ReadString(yamlString);

		Ref<Material> material = DeserializeFromYAML(yamlString);
        return material;
    }

    std::string MaterialSerializer::SerializeToYAML(Ref<Material> material) const
    {
        YAML::Emitter out;
		out << YAML::BeginMap; // Material
		out << YAML::Key << "Material" << YAML::Value;
		{
			out << YAML::BeginMap;
            out << YAML::Key << "Name" << YAML::Value << material->GetName();
            for (auto [name, value] : material->GetUniforms())
            {
                if (name.find("u_Material") != std::string::npos)
                    Utils::Serialization::SerializeProperty(name, value, out);
            }
            out << YAML::EndMap;
        }

        {
            out << YAML::Key << "Textures" << YAML::Value;
			out << YAML::BeginMap;
            for (auto [name, handle] : material->GetTextureAssetHandles())
            {
                if (name.find("Tex") != std::string::npos)
                    out << YAML::Key << name << YAML::Value << handle;
            }
            std::vector<AssetHandle> textureHandles;
            out << YAML::EndMap;
        }

		out << YAML::EndMap; // Material
		return std::string(out.c_str());
    }

    Ref<Material> MaterialSerializer::DeserializeFromYAML(const std::string &yamlString) const
    {
        YAML::Node root = YAML::Load(yamlString);
		YAML::Node materialNode = root["Material"];
		YAML::Node texturesNode = root["Textures"];

        Ref<Material> material = Material::Create(materialNode["Name"].as<std::string>());
        for (const auto& pair : materialNode)
        {
            std::string key = pair.first.as<std::string>();
            if (key == "Name")
                continue;

            const YAML::Node& valueNode = pair.second;
            UniformValue value;

            if (valueNode.IsScalar()) {
                try {
                    try {
                        value = valueNode.as<bool>();
                    } catch (const YAML::BadConversion& e) {
                    }
                    try {
                        value = valueNode.as<int>();
                    } catch (const YAML::BadConversion& e) {
                    }
                    try {
                        value = valueNode.as<float>();
                    } catch (const YAML::BadConversion& e) {
                    }
                } catch (const YAML::Exception& e) {
                    CZ_CORE_ERROR("Unknown type: {0}", e.what());
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

            material->Set(key, value);
        }

        for (const auto& pair : texturesNode)
        {
            std::string key = pair.first.as<std::string>();
            AssetHandle handle = pair.second.as<std::uint64_t>();
            material->SetTextureHandle(key, handle);
        }

        return material;
    }

    //==============================================================================
	/// MeshSourceSerializer
    uint64_t MeshSourceSerializer::Serialize(const AssetMetadata &metadata, Ref<Asset> &asset) const
    {
        fs::path path(metadata.FilePath);
        fs::path filepath = Utils::File::GetAssetDirectory() / path;
        fs::path dest = filepath.parent_path() / (filepath.filename().string() + ".asset");

        FileStreamWriter stream(dest);
		uint64_t start = stream.GetStreamPosition();

        Ref<MeshSource> meshSource = asset.As<MeshSource>();
		bool hasMaterials = !meshSource->GetMaterials().empty();

        MeshSourceFile file;

        file.Data.Flags = 0;
		if (hasMaterials)
			file.Data.Flags |= (uint32_t)MeshSourceFile::MeshFlags::HasMaterials;

        // Write header
        file.Header.CreateAt = metadata.CreateAt;
        file.Header.ModifiedAt = metadata.ModifiedAt;
		stream.WriteRaw<MeshSourceFile::FileHeader>(file.Header);

        // Leave space for Metadata
		uint64_t metadataAbsolutePosition = stream.GetStreamPosition();
		stream.WriteZero(sizeof(MeshSourceFile::Metadata));

        // Write boudingBox
        file.Data.BoudingBox = meshSource->GetBoundingBox();

        // Write nodes
        file.Data.NodeArrayOffset = stream.GetStreamPosition() - start;
        stream.WriteArray(meshSource->m_Nodes);
        file.Data.NodeArraySize = (stream.GetStreamPosition() - start) - file.Data.NodeArrayOffset;

        // Write submeshes
        file.Data.SubmeshArrayOffset = stream.GetStreamPosition() - start;
        stream.WriteArray(meshSource->m_Submeshes);
        file.Data.SubmeshArraySize = (stream.GetStreamPosition() - start) - file.Data.SubmeshArrayOffset;

        // Write vertex buffer
        file.Data.VertexBufferOffset = stream.GetStreamPosition() - start;
        stream.WriteArray(meshSource->m_Buffer.Vertexs);
        file.Data.VertexBufferSize = (stream.GetStreamPosition() - start) - file.Data.VertexBufferOffset;

        // Write index buffer
        file.Data.IndexBufferOffset = stream.GetStreamPosition() - start;
        stream.WriteArray(meshSource->m_Buffer.Indexs);
        file.Data.IndexBufferSize = (stream.GetStreamPosition() - start) - file.Data.IndexBufferOffset;

        // Write materail buffer
        if (!meshSource->m_Materials.empty())
        {
            std::vector<MeshMaterial> meshMaterials(meshSource->m_Materials.size());
            for (size_t i = 0; i < meshSource->m_Materials.size(); i++)
            {
                MeshMaterial& material = meshMaterials[i];
                Ref<Material> meshSourceMaterial = Application::GetAssetManager()->GetAsset(meshSource->m_Materials[i]);

                material.Name = meshSourceMaterial->GetName();
                material.ShaderName = meshSourceMaterial->GetShader()->GetName();

                material.Albedo             = Utils::GetVec3(meshSourceMaterial->GetUniforms()["u_Material.Albedo"]);
                material.Metalness          = Utils::GetFloat(meshSourceMaterial->GetUniforms()["u_Material.Metalness"]);
                material.Roughness          = Utils::GetFloat(meshSourceMaterial->GetUniforms()["u_Material.Roughness"]);
                material.Ambient            = Utils::GetFloat(meshSourceMaterial->GetUniforms()["u_Material.Ambient"]);
                material.AmbientStrength    = Utils::GetFloat(meshSourceMaterial->GetUniforms()["u_Material.AmbientStrength"]);
                material.Specular           = Utils::GetFloat(meshSourceMaterial->GetUniforms()["u_Material.Specular"]);

                material.EnableAlbedoTex    = Utils::GetBool(meshSourceMaterial->GetUniforms()["u_Material.EnableAlbedoTex"]);
                material.EnableNormalTex    = Utils::GetBool(meshSourceMaterial->GetUniforms()["u_Material.EnableNormalTex"]);
                material.EnableMetalnessTex = Utils::GetBool(meshSourceMaterial->GetUniforms()["u_Material.EnableMetalnessTex"]);
                material.EnableRoughnessTex = Utils::GetBool(meshSourceMaterial->GetUniforms()["u_Material.EnableRoughnessTex"]);

                auto albedoTex              = meshSourceMaterial->GetTexture("u_AlbedoTex");
                auto normalTex              = meshSourceMaterial->GetTexture("u_NormalTex");
                auto metalnessTex           = meshSourceMaterial->GetTexture("u_MetalnessTex");
                auto roughnessTex           = meshSourceMaterial->GetTexture("u_RoughnessTex");

                if (albedoTex)
                {
                    Application::GetAssetManager()->SaveAsset(albedoTex, filepath.parent_path() / albedoTex->GetSpecification().DebugName);
                    material.AlbedoTexture = albedoTex->Handle;
                }

                if (normalTex)
                {
                    Application::GetAssetManager()->SaveAsset(normalTex, filepath.parent_path() / normalTex->GetSpecification().DebugName);
                    material.NormalTexture = normalTex->Handle;
                }

                if (metalnessTex)
                {
                    Application::GetAssetManager()->SaveAsset(metalnessTex, filepath.parent_path() / metalnessTex->GetSpecification().DebugName);
                    material.MetalnessTexture = metalnessTex->Handle;
                }

                if (roughnessTex)
                {
                    Application::GetAssetManager()->SaveAsset(roughnessTex, filepath.parent_path() / roughnessTex->GetSpecification().DebugName);
                    material.RoughnessTexture = roughnessTex->Handle;
                }
            }

            file.Data.MaterialArrayOffset = stream.GetStreamPosition() - start;
            stream.WriteArray(meshMaterials);
            file.Data.MaterialArraySize = (stream.GetStreamPosition() - start) - file.Data.SubmeshArrayOffset;
        }
        else
        {
            file.Data.MaterialArrayOffset = 0;
            file.Data.MaterialArraySize = 0;
        }

        // Write metadata
		uint64_t endOfStream = stream.GetStreamPosition();
		stream.SetStreamPosition(metadataAbsolutePosition);
        stream.WriteRaw(file.Data);
		stream.SetStreamPosition(endOfStream);

        uint64_t size = stream.GetStreamPosition() - start;

        return size;
    }

    Ref<Asset> MeshSourceSerializer::Deserialize(AssetMetadata &metadata) const
    {
        fs::path path(metadata.FilePath);
        fs::path filepath = Utils::File::GetAssetDirectory() / path;
        fs::path dest = filepath.parent_path() / (filepath.filename().string() + ".asset");

		Ref<MeshSource> meshSource = Ref<MeshSource>::Create();

        FileStreamReader stream(dest);
		uint64_t streamOffset = stream.GetStreamPosition();

		MeshSourceFile file;

        // Read header
		stream.ReadRaw<MeshSourceFile::FileHeader>(file.Header);
        metadata.CreateAt = file.Header.CreateAt;
        metadata.ModifiedAt = file.Header.ModifiedAt;

        // Read meshSourceFile metadata
        stream.ReadRaw<MeshSourceFile::Metadata>(file.Data);

        // Read boudingBox
        meshSource->m_BoundingBox = file.Data.BoudingBox;

        // Read nodes
		stream.SetStreamPosition(file.Data.NodeArrayOffset + streamOffset);
        stream.ReadArray(meshSource->m_Nodes);

        // Read submeshes
		stream.SetStreamPosition(file.Data.SubmeshArrayOffset + streamOffset);
        stream.ReadArray(meshSource->m_Submeshes);

        // Read vertex buffer
		stream.SetStreamPosition(file.Data.VertexBufferOffset + streamOffset);
		stream.ReadArray(meshSource->m_Buffer.Vertexs);

        // Read index buffer
        stream.SetStreamPosition(file.Data.IndexBufferOffset + streamOffset);
		stream.ReadArray(meshSource->m_Buffer.Indexs);

        // Read materials
        std::vector<MeshMaterial> meshMaterials;
        stream.SetStreamPosition(file.Data.MaterialArrayOffset + streamOffset);
        stream.ReadArray(meshMaterials);

        meshSource->m_Materials.resize(meshMaterials.size());
        for (size_t i = 0; i < meshMaterials.size(); i++)
        {
            const auto& meshMaterial = meshMaterials[i];

            Ref<Shader> shader = Renderer::GetShaderLibrary()->Get(meshMaterial.ShaderName);
            CZ_CORE_ASSERT(shader, meshMaterial.ShaderName, meshMaterial.Name, "Shader called {} of Material {} doesn't exist.");
            Ref<Material> material = Material::Create(shader, meshMaterial.Name);

            material->Set("u_Material.Albedo", meshMaterial.Albedo);
            material->Set("u_Material.Metalness", meshMaterial.Metalness);
            material->Set("u_Material.Roughness", meshMaterial.Roughness);
            material->Set("u_Material.Ambient", meshMaterial.Ambient);
            material->Set("u_Material.AmbientStrength", meshMaterial.AmbientStrength);
            material->Set("u_Material.Specular", meshMaterial.Specular);

            material->Set("u_Material.EnableAlbedoTex", meshMaterial.EnableAlbedoTex);
            material->Set("u_Material.EnableNormalTex", meshMaterial.EnableNormalTex);
            material->Set("u_Material.EnableMetalnessTex", meshMaterial.EnableMetalnessTex);
            material->Set("u_Material.EnableRoughnessTex", meshMaterial.EnableRoughnessTex);

            auto albedoTex    = Application::GetAssetManager()->GetAsset(meshMaterial.AlbedoTexture);
            auto normalTex    = Application::GetAssetManager()->GetAsset(meshMaterial.NormalTexture);
            auto metalnessTex = Application::GetAssetManager()->GetAsset(meshMaterial.MetalnessTexture);
            auto roughnessTex = Application::GetAssetManager()->GetAsset(meshMaterial.RoughnessTexture);

            if (albedoTex)
                material->Set("u_AlbedoTex", albedoTex.As<Texture2D>());
            if (normalTex)
                material->Set("u_NormalTex", normalTex.As<Texture2D>());
            if (metalnessTex)
                material->Set("u_MetalnessTex", metalnessTex.As<Texture2D>());
            if (roughnessTex)
                material->Set("u_RoughnessTex", roughnessTex.As<Texture2D>());

            Application::GetAssetManager()->AddMemoryOnlyAsset(material);
            meshSource->m_Materials[i] = material->Handle;
        }

        return meshSource;
    }

    std::string MeshSourceSerializer::SerializeToYAML(Ref<MeshSource> meshSource) const
    {
        return std::string();
    }

    Ref<MeshSource> MeshSourceSerializer::DeserializeFromYAML(const std::string &yamlString) const
    {
        return Ref<MeshSource>();
    }
}
