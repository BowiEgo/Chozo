#pragma once

#include "MeshLoader.h"
#include "PBRMaterialProps.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Chozo {

    class AssimpLoader final : public MeshLoader
    {
    public:
        AssimpLoader() = default;
        ~AssimpLoader() override = default;

        Ref<MeshSource> LoadFromFile(const fs::path& filepath) override;
    private:
        void TraverseNodes(Ref<MeshSource> meshSource, void* assimpNode, uint32_t nodeIndex, const glm::mat4& parentTransform = glm::mat4(1.0f), uint32_t level = 0);
        void ApplyTextureByType(Ref<Material> target, aiMaterial* aiMaterial, const aiScene* scene, PBRMaterialTextureType propType, fs::path filePath);
    };
}