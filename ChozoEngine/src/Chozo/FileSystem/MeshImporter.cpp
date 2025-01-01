#include "MeshImporter.h"

#include "AssimpLoader.h"
#include "GLTFLoader.h"

namespace Chozo {

    std::unordered_map<MeshLoaderType, Scope<MeshLoader>> MeshImporter::s_Loaders;

    void MeshImporter::Init()
    {
        s_Loaders.clear();
        s_Loaders.emplace(MeshLoaderType::Assimp, CreateScope<AssimpLoader>());
        s_Loaders.emplace(MeshLoaderType::GLTF, CreateScope<GLTFLoader>());
    }

    Ref<MeshSource> MeshImporter::ToMeshSourceFromFile(const std::string &path)
    {
        const fs::path filePath(path);
        const MeshLoaderType type = Utils::GetMeshLoaderTypeFromExtension(filePath.extension().string());

        if (type == MeshLoaderType::None)
            return nullptr;

        const auto& loader = s_Loaders[type];

        return loader->LoadFromFile(filePath);
    }
} // namespace Chozo
