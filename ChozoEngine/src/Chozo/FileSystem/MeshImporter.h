#pragma once

#include "Chozo/Renderer/Mesh.h"

namespace Chozo {
    
    class MeshImporter
    {
    public:
        static Ref<MeshSource> ToMeshSourceFromFile(const std::string &path);
    private:
		static void TraverseNodes(Ref<MeshSource> meshSource, void* assimpNode, uint32_t nodeIndex, const glm::mat4& parentTransform = glm::mat4(1.0f), uint32_t level = 0);
    };
} // namespace Chozo
