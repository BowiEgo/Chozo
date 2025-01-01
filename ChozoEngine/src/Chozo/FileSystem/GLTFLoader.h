#pragma once

#include <tiny_gltf.h>

#include "MeshLoader.h"
#include "PBRMaterialProps.h"

namespace Chozo {

    class GLTFLoader final : public MeshLoader
    {
    public:
        GLTFLoader() = default;
        ~GLTFLoader() override = default;

        Ref<MeshSource> LoadFromFile(const fs::path& filepath) override;
    private:
        void Release();
        void TraverseNodes(const tinygltf::Node& node, Ref<MeshSource> meshSource, const uint32_t parentMeshNodeIdx);
        void HandleMesh(const tinygltf::Node& node, MeshNode& meshNode, const MeshNode& parentMeshNode, Ref<MeshSource> meshSource);
        void HandleVertices(const tinygltf::Primitive& primitive, Submesh& submesh, Ref<MeshSource> meshSource);
        void HandleIndices(const tinygltf::Primitive& primitive, Submesh& submesh, Ref<MeshSource> meshSource);
        void HandleMaterials(const tinygltf::Primitive& primitive, Submesh& submesh, Ref<MeshSource> meshSource);
        void HandleTextures(const PBRMaterialTextureType& propType, const tinygltf::Material& mat, int textureIdx, Ref<Material>& mi);
    private:
        uint32_t m_CurrVertexPos = 0;
        uint32_t m_CurrIndexPos = 0;

        tinygltf::Model m_GLTFModel;
    };
}
