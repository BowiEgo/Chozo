#pragma once

#include "Chozo/Renderer/Mesh.h"

namespace Chozo {

    enum class MeshLoaderType
    {
        None = 0,
        Assimp,
        GLTF
    };

    inline static std::unordered_map<std::string, MeshLoaderType> s_MeshFileExtensionMap =
    {
        { ".fbx", MeshLoaderType::Assimp },
        { ".obj", MeshLoaderType::Assimp },

        { ".gltf", MeshLoaderType::GLTF },
        { ".glb", MeshLoaderType::GLTF },
    };

    namespace Utils {

        inline MeshLoaderType GetMeshLoaderTypeFromExtension(const std::string& extension)
        {
            const std::string ext = String::ToLowerCopy(extension);
            if (s_MeshFileExtensionMap.find(ext) == s_MeshFileExtensionMap.end()) {
                CZ_CORE_ERROR("Mesh Loader: No such loader for {0}!", ext);
                return MeshLoaderType::None;
            }

            return s_MeshFileExtensionMap.at(ext);
        }
    }

    class MeshLoader
    {
    public:
        virtual ~MeshLoader() = default;

        virtual Ref<MeshSource> LoadFromFile(const fs::path& filepath) = 0;
    };
}