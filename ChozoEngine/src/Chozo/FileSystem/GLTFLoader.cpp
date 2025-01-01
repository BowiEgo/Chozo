#include "GLTFLoader.h"

#include "TextureImporter.h"

#include "Chozo/Core/Application.h"
#include "Chozo/Renderer/Renderer.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <tiny_gltf.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Chozo {

    namespace GLTFLoaderUtils {
        static glm::mat4 Mat4FromGLTFMatrix4x4(const std::vector<double>& matrix)
        {
            if (matrix.size() != 16) {
                throw std::runtime_error("Invalid matrix size. Expected a 4x4 matrix with 16 elements.");
            }

            return glm::mat4{
                matrix[0], matrix[1], matrix[2], matrix[3],
                matrix[4], matrix[5], matrix[6], matrix[7],
                matrix[8], matrix[9], matrix[10], matrix[11],
                matrix[12], matrix[13], matrix[14], matrix[15]
            };
        }

        static size_t ComponentTypeByteSize(const int type) {
            switch (type) {
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                case TINYGLTF_COMPONENT_TYPE_BYTE:
                  return sizeof(char);
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                case TINYGLTF_COMPONENT_TYPE_SHORT:
                  return sizeof(short);
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                case TINYGLTF_COMPONENT_TYPE_INT:
                  return sizeof(int);
                case TINYGLTF_COMPONENT_TYPE_FLOAT:
                    return sizeof(float);
                case TINYGLTF_COMPONENT_TYPE_DOUBLE:
                    return sizeof(double);
                default:
                    return 0;
            }
        }

        static void PrintNodes(const tinygltf::Scene &scene)
        {
            for (const int node : scene.nodes)
            {
                CZ_CORE_INFO("GLTFLoader: Node {0}", node);
            }
        }

        void CountPrimitivesInNode(const tinygltf::Model &model, const int nodeIndex, int &primitiveCount)
        {
            const tinygltf::Node &node = model.nodes[nodeIndex];

            if (node.mesh >= 0) {
                const tinygltf::Mesh &mesh = model.meshes[node.mesh];
                primitiveCount += static_cast<int>(mesh.primitives.size());
            }

            for (const int child : node.children) {
                CountPrimitivesInNode(model, child, primitiveCount);
            }
        }

        static int CountPrimitivesInScene(const tinygltf::Model &model, const tinygltf::Scene &scene) {
            int primitiveCount = 0;

            for (const int nodeIndex : scene.nodes) {
                CountPrimitivesInNode(model, nodeIndex, primitiveCount);
            }

            return primitiveCount;
        }

        static glm::mat4 GetLocalTransforms(const tinygltf::Node &node)
        {
            glm::mat4 result;

            if (node.matrix.size() == 16) {
                result = GLTFLoaderUtils::Mat4FromGLTFMatrix4x4(node.matrix);
            } else {
                auto translation = glm::mat4{1.0f};
                auto rotation = glm::mat4{1.0f};
                auto scale = glm::mat4{1.0f};

                if (node.translation.size() == 3) {
                    translation = glm::translate(glm::mat4(1.0f),
                        glm::vec3(
                            node.translation[0],
                            node.translation[1],
                            node.translation[2]
                        )
                    );
                }

                if (node.rotation.size() == 4) {
                    const glm::quat quat(
                        static_cast<float>(node.rotation[3]),
                        static_cast<float>(node.rotation[0]),
                        static_cast<float>(node.rotation[1]),
                        static_cast<float>(node.rotation[2])
                    );
                    rotation = glm::toMat4(quat);
                }

                if (node.scale.size() == 3) {
                    scale = glm::scale(glm::mat4(1.0f), glm::vec3(
                        node.scale[0],
                        node.scale[1],
                        node.scale[2]));
                }

                result = translation * rotation * scale;
            }

            return result;
        }

        static void ComputeTangentAndBitangent(Triangle &triangle)
        {
            const glm::vec3 edge1 = triangle.V1.Position - triangle.V0.Position;
            const glm::vec3 edge2 = triangle.V2.Position - triangle.V0.Position;

            const glm::vec2 deltaUV1 = triangle.V1.TexCoord - triangle.V0.TexCoord;
            const glm::vec2 deltaUV2 = triangle.V2.TexCoord - triangle.V0.TexCoord;

            if (const float determinant = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y; determinant != 0.0f)
            {
                const float f = 1.0f / determinant;

                glm::vec3 tangent;
                tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                triangle.V0.Tangent += tangent;
                triangle.V1.Tangent += tangent;
                triangle.V2.Tangent += tangent;

                glm::vec3 bitangent;
                bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
                bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
                bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
                triangle.V0.Bitangent += bitangent;
                triangle.V1.Bitangent += bitangent;
                triangle.V2.Bitangent += bitangent;
            }
        }
    }

    Ref<MeshSource> GLTFLoader::LoadFromFile(const fs::path &filepath)
    {
        CZ_CORE_INFO("GLTFLoader: {0}", filepath.string());

        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string err;
        std::string warn;

        bool ret = false;

        if (filepath.extension() == ".glb")
        {
            ret = loader.LoadBinaryFromFile(&model, &err, &warn, filepath.string());
        }
        else {
            ret = loader.LoadASCIIFromFile(&model, &err, &warn, filepath.string());
        }

        if (!warn.empty()) {
            printf("Warn: %s\n", warn.c_str());
        }

        if (!err.empty()) {
            printf("Err: %s\n", err.c_str());
        }

        if (!ret) {
            printf("Failed to parse glTF\n");
            return nullptr;
        }

        m_GLTFModel = model;
        auto scene = m_GLTFModel.scenes[m_GLTFModel.defaultScene > -1 ? m_GLTFModel.defaultScene : 0];
        GLTFLoaderUtils::PrintNodes(scene);

        Ref<MeshSource> meshSource = Ref<MeshSource>::Create();

        // Load buffers (e.g., vertex and index data)
        {
            CZ_CORE_INFO("GLTFLoader: Loading buffers...");
            CZ_CORE_INFO("GLTFLoader: Node size {0}", scene.nodes.size());

            auto primitiveCount = GLTFLoaderUtils::CountPrimitivesInScene(m_GLTFModel, scene);
            auto nodeCount = m_GLTFModel.nodes.size();
            auto materialCount = m_GLTFModel.nodes.size();

            meshSource->m_Submeshes.reserve(primitiveCount);
            meshSource->m_Nodes.reserve(nodeCount);
            meshSource->m_Materials.reserve(materialCount);

            m_CurrVertexPos = 0;
            m_CurrIndexPos = 0;

            for (int i : scene.nodes)
            {
                auto& node = m_GLTFModel.nodes[i];
                if (node.mesh == -1)
                {
                    TraverseNodes(node, meshSource, -1);
                }
                else
                {
                    const tinygltf::Mesh& mesh = m_GLTFModel.meshes[node.mesh];

                    MeshNode& meshNode = meshSource->m_Nodes.emplace_back();
                    meshNode.Parent = meshSource->m_Nodes.size();
                    meshNode.Name = node.name;
                    meshNode.LocalTransform = GLTFLoaderUtils::GetLocalTransforms(node);
                    meshNode.Submeshes.resize(mesh.primitives.size());

                    HandleMesh(node, meshNode, meshNode, meshSource);
                }
            }
        }

        // Load textures
        {
        }

        Release();

        return meshSource;
    }

    void GLTFLoader::Release()
    {
        m_GLTFModel.buffers.clear();
        m_GLTFModel.bufferViews.clear();
        m_GLTFModel.accessors.clear();
        m_GLTFModel.images.clear();
        m_GLTFModel.materials.clear();
        m_GLTFModel.meshes.clear();
        m_GLTFModel.nodes.clear();
        m_GLTFModel.scenes.clear();
        m_GLTFModel.textures.clear();
        m_GLTFModel.skins.clear();
        m_GLTFModel.samplers.clear();
        m_GLTFModel.animations.clear();
        m_GLTFModel.extensionsUsed.clear();
        m_GLTFModel.extensionsRequired.clear();

        m_GLTFModel.asset.version.clear();
        m_GLTFModel.asset.generator.clear();
        m_GLTFModel.asset.copyright.clear();
    }

    void GLTFLoader::TraverseNodes(const tinygltf::Node& node, Ref<MeshSource> meshSource, const uint32_t parentMeshNodeIdx)
    {
        CZ_CORE_INFO("GLTFLoader: TraverseNodes: {0}", node.mesh);
        const auto childCount = node.children.size();
        const auto currIdx = meshSource->m_Nodes.size();
        MeshNode& meshNode = meshSource->m_Nodes.emplace_back();

        if (parentMeshNodeIdx != -1)
            meshNode.Parent = parentMeshNodeIdx;
        meshNode.Name = node.name;
        meshNode.LocalTransform = GLTFLoaderUtils::GetLocalTransforms(node);
        meshNode.Children.resize(childCount);

        for (unsigned int i = 0; i < childCount; i++)
        {
            meshNode.Children[i] = meshSource->m_Nodes.size();

            if (auto childNode = m_GLTFModel.nodes[node.children[i]]; childNode.mesh == -1)
            {
                TraverseNodes(childNode, meshSource, currIdx);
            }
            else
            {
                const tinygltf::Mesh& mesh = m_GLTFModel.meshes[childNode.mesh];

                MeshNode& childMeshNode = meshSource->m_Nodes.emplace_back();
                childMeshNode.Parent = currIdx;
                childMeshNode.Name = childNode.name;
                childMeshNode.LocalTransform = GLTFLoaderUtils::GetLocalTransforms(childNode);
                childMeshNode.Submeshes.resize(mesh.primitives.size());

                HandleMesh(childNode, childMeshNode, meshNode, meshSource);
            }
        }
    }

    void GLTFLoader::HandleMesh(const tinygltf::Node& node, MeshNode& meshNode, const MeshNode& parentMeshNode, Ref<MeshSource> meshSource)
    {
        const tinygltf::Mesh& mesh = m_GLTFModel.meshes[node.mesh];

        for (unsigned int i = 0; i < mesh.primitives.size(); i++)
        {
            auto primitive = mesh.primitives[i];

            meshNode.Submeshes[i] = meshSource->m_Submeshes.size();

            Submesh& submesh = meshSource->m_Submeshes.emplace_back();
            submesh.NodeName = node.name;
            submesh.LocalTransform = meshNode.LocalTransform;
            submesh.Transform = parentMeshNode.LocalTransform * submesh.LocalTransform;
            submesh.MeshName = mesh.name;
            submesh.BaseVertex = m_CurrVertexPos;
            submesh.BaseIndex = m_CurrIndexPos;

            // Vertices
            HandleVertices(primitive, submesh, meshSource);

            // Indices
            HandleIndices(primitive, submesh, meshSource);

            // Materials
            HandleMaterials(primitive, submesh, meshSource);
        }
    }

    void GLTFLoader::HandleVertices(const tinygltf::Primitive& primitive, Submesh& submesh, Ref<MeshSource> meshSource)
    {
        auto& aabb = submesh.BoundingBox;
        aabb.Min = { FLT_MAX, FLT_MAX, FLT_MAX };
        aabb.Max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

        const bool hasPositions = primitive.attributes.find("POSITION") != primitive.attributes.end();
        const bool hasNormals = primitive.attributes.find("NORMAL") != primitive.attributes.end();
        const bool hasTangents = primitive.attributes.find("TANGENT") != primitive.attributes.end();
        const bool hasTexcoords = primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end();

        if (hasPositions)
        {
            const tinygltf::Accessor& positionAccessor = m_GLTFModel.accessors[primitive.attributes.at("POSITION")];

            const float* positions = nullptr;
            const float* normals = nullptr;
            const float* tangents = nullptr;
            const float* texcoords = nullptr;

            {
                const tinygltf::Accessor& accessors = positionAccessor;
                const tinygltf::BufferView& bufferView = m_GLTFModel.bufferViews[accessors.bufferView];
                const auto offset = bufferView.byteOffset + accessors.byteOffset;
                CZ_CORE_ASSERT("bufferView.byteOffset is not aligned with accessor.byteOffset", offset % sizeof(float) == 0);
                const tinygltf::Buffer& buffer = m_GLTFModel.buffers[bufferView.buffer];
                const uint8_t* data = buffer.data.data() + offset;

                positions = reinterpret_cast<const float*>(data);
            }

            if (hasNormals)
            {
                const tinygltf::Accessor& accessors = m_GLTFModel.accessors[primitive.attributes.at("NORMAL")];
                const tinygltf::BufferView& bufferView = m_GLTFModel.bufferViews[accessors.bufferView];
                const auto offset = bufferView.byteOffset + accessors.byteOffset;
                CZ_CORE_ASSERT("bufferView.byteOffset is not aligned with accessor.byteOffset", offset % sizeof(float) == 0);
                const tinygltf::Buffer& buffer = m_GLTFModel.buffers[bufferView.buffer];
                const uint8_t* data = buffer.data.data() + offset;

                normals = reinterpret_cast<const float*>(data);
            }

            if (hasTangents)
            {
                const tinygltf::Accessor& accessors = m_GLTFModel.accessors[primitive.attributes.at("TANGENT")];
                const tinygltf::BufferView& bufferView = m_GLTFModel.bufferViews[accessors.bufferView];
                const auto offset = bufferView.byteOffset + accessors.byteOffset;
                CZ_CORE_ASSERT("bufferView.byteOffset is not aligned with accessor.byteOffset", offset % sizeof(float) == 0);
                const tinygltf::Buffer& buffer = m_GLTFModel.buffers[bufferView.buffer];
                const uint8_t* data = buffer.data.data() + offset;

                tangents = reinterpret_cast<const float*>(data);
            }

            if (hasTexcoords)
            {
                const tinygltf::Accessor& accessors = m_GLTFModel.accessors[primitive.attributes.at("TEXCOORD_0")];
                const tinygltf::BufferView& bufferView = m_GLTFModel.bufferViews[accessors.bufferView];
                const auto offset = bufferView.byteOffset + accessors.byteOffset;
                CZ_CORE_ASSERT("bufferView.byteOffset is not aligned with accessor.byteOffset", offset % sizeof(float) == 0);
                const tinygltf::Buffer& buffer = m_GLTFModel.buffers[bufferView.buffer];
                const uint8_t* data = buffer.data.data() + offset;

                texcoords = reinterpret_cast<const float*>(data);
            }

            submesh.VertexCount = static_cast<uint32_t>(positionAccessor.count);
            m_CurrVertexPos += submesh.VertexCount;

            for (size_t i = 0; i < positionAccessor.count; i++)
            {
                Vertex vertex{};

                vertex.Position = glm::vec3(
                    positions[i * 3 + 0],
                    positions[i * 3 + 1],
                    positions[i * 3 + 2]);

                vertex.Normal = hasNormals
                    ? glm::vec3(
                        normals[i * 3 + 0],
                        normals[i * 3 + 1],
                        normals[i * 3 + 2])
                    : glm::vec3(0.0f, 1.0f, 0.0f);

                vertex.Tangent = hasTangents
                    ? glm::vec3(
                        tangents[i * 3 + 0],
                        tangents[i * 3 + 1],
                        tangents[i * 3 + 2])
                    : glm::vec3(0.0f, 0.0f, 0.0f);

                vertex.TexCoord = hasTexcoords
                    ? glm::vec2(
                        texcoords[i * 2 + 0],
                        texcoords[i * 2 + 1])
                    : glm::vec2(0.0f, 0.0f);

                if (positionAccessor.minValues.size() == 3 && positionAccessor.maxValues.size() == 3)
                {
                    aabb.Min = glm::vec3(positionAccessor.minValues[0], positionAccessor.minValues[1], positionAccessor.minValues[2]);
                    aabb.Max = glm::vec3(positionAccessor.maxValues[0], positionAccessor.maxValues[1], positionAccessor.maxValues[2]);
                }
                else
                {
                    aabb.Min.x = glm::min(vertex.Position.x, aabb.Min.x);
                    aabb.Min.y = glm::min(vertex.Position.y, aabb.Min.y);
                    aabb.Min.z = glm::min(vertex.Position.z, aabb.Min.z);
                    aabb.Max.x = glm::max(vertex.Position.x, aabb.Max.x);
                    aabb.Max.y = glm::max(vertex.Position.y, aabb.Max.y);
                    aabb.Max.z = glm::max(vertex.Position.z, aabb.Max.z);
                }

                submesh.BoundingBox.Min = glm::min(submesh.BoundingBox.Min, vertex.Position);
                submesh.BoundingBox.Max = glm::max(submesh.BoundingBox.Max, vertex.Position);

                meshSource->m_Buffer.Vertexs.push_back(vertex);
            }
        }
    }

    void GLTFLoader::HandleIndices(const tinygltf::Primitive& primitive, Submesh& submesh, Ref<MeshSource> meshSource)
    {
        if (primitive.indices != -1)
        {
            const tinygltf::Accessor& indexAccessor = m_GLTFModel.accessors[primitive.indices];
            const tinygltf::BufferView& indexBufferView = m_GLTFModel.bufferViews[indexAccessor.bufferView];
            const tinygltf::Buffer& indexBuffer = m_GLTFModel.buffers[indexBufferView.buffer];

            const uint8_t* indexData = indexBuffer.data.data() + indexBufferView.byteOffset + indexAccessor.byteOffset;
            size_t indexCount = indexAccessor.count;

            submesh.IndexCount = static_cast<uint32_t>(indexCount);
            m_CurrIndexPos += submesh.IndexCount;

            // Make sure index component type legal（unsigned short or unsigned int）
            if (indexAccessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT &&
                indexAccessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
            {
                std::cerr << "Unsupported index component type: " << indexAccessor.componentType << std::endl;
                return;
            }

            for (size_t i = 0; i < indexCount; i += 3) {
                Index index{};
                if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    const auto* indices = reinterpret_cast<const uint16_t*>(indexData);
                    index.V1 = indices[i];
                    index.V2 = indices[i + 1];
                    index.V3 = indices[i + 2];
                } else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                    const auto* indices = reinterpret_cast<const uint32_t*>(indexData);
                    index.V1 = indices[i];
                    index.V2 = indices[i + 1];
                    index.V3 = indices[i + 2];
                }

                meshSource->m_Buffer.Indexs.push_back(index);
                meshSource->m_Buffer.IndexCount++;
                meshSource->m_Buffer.IndicesCount += 3;

                Triangle triangle{
                    meshSource->m_Buffer.Vertexs[index.V1 + submesh.BaseVertex],
                    meshSource->m_Buffer.Vertexs[index.V2 + submesh.BaseVertex],
                    meshSource->m_Buffer.Vertexs[index.V3 + submesh.BaseVertex],
                };

                // Tangent & Bitangent
                GLTFLoaderUtils::ComputeTangentAndBitangent(triangle);
                meshSource->m_TriangleCache[meshSource->m_Submeshes.size() - 1].emplace_back(triangle);
            }

            // Normalize Tangent & Bitangent
            auto& triangles = meshSource->m_TriangleCache[meshSource->m_Submeshes.size() - 1];
            for (Triangle& triangle : triangles) {
                triangle.V0.Tangent = glm::normalize(triangle.V0.Tangent);
                triangle.V1.Tangent = glm::normalize(triangle.V1.Tangent);
                triangle.V2.Tangent = glm::normalize(triangle.V2.Tangent);

                triangle.V0.Bitangent = glm::normalize(triangle.V0.Bitangent);
                triangle.V1.Bitangent = glm::normalize(triangle.V1.Bitangent);
                triangle.V2.Bitangent = glm::normalize(triangle.V2.Bitangent);
            }
        }
    }

    void GLTFLoader::HandleMaterials(const tinygltf::Primitive& primitive, Submesh& submesh, Ref<MeshSource> meshSource)
    {
        if (primitive.material != -1)
        {
            tinygltf::Material &mat = m_GLTFModel.materials[primitive.material];

            CZ_CORE_TRACE("GLTFLoader::HandleMaterials: Material \"{0}\"", mat.name);
            auto mi = Material::Create(Renderer::GetRendererData().m_ShaderLibrary->Get("Geometry"), mat.name);

            // PBR
            {
                glm::vec3 baseColor = {
                    mat.pbrMetallicRoughness.baseColorFactor[0],
                    mat.pbrMetallicRoughness.baseColorFactor[1],
                    mat.pbrMetallicRoughness.baseColorFactor[2],
                    // mat.pbrMetallicRoughness.baseColorFactor[3],
                };
                mi->Set("u_Material.BaseColor", baseColor);
                mi->Set("u_Material.Metallic", (float)mat.pbrMetallicRoughness.metallicFactor);
                mi->Set("u_Material.Roughness", (float)mat.pbrMetallicRoughness.roughnessFactor);

                const int baseColorTexIdx = mat.pbrMetallicRoughness.baseColorTexture.index;
                const int metallicRoughnessTexIdx = mat.pbrMetallicRoughness.metallicRoughnessTexture.index;

                HandleTextures(PBRMaterialTextureType::BaseColor, mat, baseColorTexIdx, mi);
                HandleTextures(PBRMaterialTextureType::MetallicRoughness, mat, metallicRoughnessTexIdx, mi);
            }

            // Normal
            {
                const int normalTexIdx = mat.normalTexture.index;
                HandleTextures(PBRMaterialTextureType::Normal, mat, normalTexIdx, mi);
            }

            // Occlusion
            {
                const int occlusionTexIdx = mat.occlusionTexture.index;
                mi->Set("u_Material.OcclusionStrength", 1.0f);
                HandleTextures(PBRMaterialTextureType::Occlusion, mat, occlusionTexIdx, mi);
            }

            // Emissive
            {
                glm::vec3 emissive = {
                    mat.emissiveFactor[0],
                    mat.emissiveFactor[1],
                    mat.emissiveFactor[2],
                };
                mi->Set("u_Material.Emissive", emissive);

                const int emissiveTexIdx = mat.emissiveTexture.index;
                mi->Set("u_Material.EmissiveStrength", 1.0f);
                HandleTextures(PBRMaterialTextureType::Emissive, mat, emissiveTexIdx, mi);
            }

            fs::path filepath = Utils::File::GetAssetDirectory();
            filepath /= fs::path("Materials/" + mat.name);
            Application::GetAssetManager()->ExportAsset(mi, filepath);

            auto& material = meshSource->m_Materials.emplace_back();
            material = mi->Handle;
        }
    }

    void GLTFLoader::HandleTextures(const PBRMaterialTextureType& propType, const tinygltf::Material& mat, int textureIdx, Ref<Material>& mi)
    {
        if (textureIdx >= 0 && textureIdx < m_GLTFModel.textures.size())
        {
            const std::string propName = PBRMaterialTextureTypeToString(propType);
            const tinygltf::Texture& tex = m_GLTFModel.textures[textureIdx];

            if (tex.source >= 0 && tex.source < m_GLTFModel.images.size())
            {
                if (const tinygltf::Image &image = m_GLTFModel.images[tex.source]; !image.image.empty())
                {
                    Texture2DSpecification spec;
                    spec.WrapS = ImageParameter::REPEAT;
                    spec.WrapT = ImageParameter::REPEAT;
                    spec.MinFilter = ImageParameter::LINEAR;
                    spec.MagFilter = ImageParameter::LINEAR;
                    spec.DebugName = mat.name + "_" + propName;

                    spec.Format = (image.component == 3) ? ImageFormat::RGB : ImageFormat::RGBA;
                    spec.Width = image.width;
                    spec.Height = image.height;

                    const tinygltf::BufferView& bufferView = m_GLTFModel.bufferViews[image.bufferView];
                    const tinygltf::Buffer& buffer = m_GLTFModel.buffers[bufferView.buffer];

                    const uint8_t* dataStart = buffer.data.data() + bufferView.byteOffset;
                    size_t dataLength = bufferView.byteLength;

                    const Buffer imageBuffer = TextureImporter::ToBufferFromMemory(
                        Buffer(dataStart, dataLength),
                        spec.Format,
                        spec.Width,
                        spec.Height
                    );

                    // TODO: Validate imageBuffer

                    const Ref<Texture2D> texture = Texture2D::Create(imageBuffer, spec);

                    fs::path filepath = Utils::File::GetAssetDirectory();
                    filepath /= fs::path("Textures/" + spec.DebugName);
                    Application::GetAssetManager()->ExportAsset(texture, filepath);

                    mi->Set("u_" + propName + "Map", texture);
                    mi->Set("u_Material.Enable" + propName + "Map", true);
                }
            }
        }
    }
}

