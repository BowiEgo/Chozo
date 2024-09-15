#include "QuadGeometry.h"

#include <glm/glm.hpp>

namespace Chozo
{
    QuadGeometry::QuadGeometry(const float &width, const float &height, const uint32_t &widthSegments, const uint32_t &heightSegments)
    {
        CallGenerate();
    }

    MeshBuffer *QuadGeometry::Generate()
    {
        m_NumberOfVertices = 0;

        MeshBuffer* buffer = new MeshBuffer();

        buffer->Vertexs = {
            {{ -0.5f, -0.5f, 0.0f }, {}, { 0.0f, 0.0f }, {}, {}, (int)m_EntityID},
            {{  0.5f, -0.5f, 0.0f }, {}, { 1.0f, 0.0f }, {}, {}, (int)m_EntityID},
            {{  0.5f,  0.5f, 0.0f }, {}, { 1.0f, 1.0f }, {}, {}, (int)m_EntityID},
            {{ -0.5f,  0.5f, 0.0f }, {}, { 0.0f, 1.0f }, {}, {}, (int)m_EntityID},
        };

        buffer->Indexs = {
            { 0, 1, 2 },
            { 2, 3, 0 }
        };

        buffer->IndicesCount = 6;
        buffer->IndexCount = 2;

        return buffer;
    }

    void QuadGeometry::Backup()
    {
    }

    void QuadGeometry::Backtrace()
    {
    }
}
