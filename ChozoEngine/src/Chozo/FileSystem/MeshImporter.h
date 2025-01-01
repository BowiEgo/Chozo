#pragma once

#include "Chozo/Renderer/Mesh.h"

#include "MeshLoader.h"

namespace Chozo {
    
    class MeshImporter
    {
    public:
        static void Init();

        static Ref<MeshSource> ToMeshSourceFromFile(const std::string &path);
    private:
        static std::unordered_map<MeshLoaderType, Scope<MeshLoader>> s_Loaders;
    };
} // namespace Chozo
