#pragma once

#include "Chozo/Renderer/Mesh.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

namespace Chozo {
    
    class MeshImporter
    {
    public:
		enum class MaterialPropType {
			BaseColor,
			Metallic,
			Roughness,
			Normal,
			Ambient,
			AmbientStrength,
			Specular
		};

        static Ref<MeshSource> ToMeshSourceFromFile(const std::string &path);
    private:
		static void TraverseNodes(Ref<MeshSource> meshSource, void* assimpNode, uint32_t nodeIndex, const glm::mat4& parentTransform = glm::mat4(1.0f), uint32_t level = 0);
		static void ApplyTextureByType(Ref<Material> target, aiMaterial* aiMaterial, const aiScene* scene, MaterialPropType propType, fs::path filePath);
    };
} // namespace Chozo
