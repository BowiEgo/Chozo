#include "Geometry.h"

#include "BoxGeometry.h"
#include "SphereGeometry.h"
#include "QuadGeometry.h"

namespace Chozo {
        
    void Geometry::SetLocalTransform(const glm::mat4 &transform)
    {
        if (m_LocalTransform == transform)
            return;

        m_LocalTransform = transform;
        CallGenerate();
    }

    void Geometry::CallGenerate()
    {
        MeshBuffer* buffer = Generate();
        m_TempBuffer.Vertexs = buffer->Vertexs;
        m_TempBuffer.Indexs = buffer->Indexs;
        m_TempBuffer.IndexCount = buffer->IndexCount;
        m_TempBuffer.IndicesCount = buffer->IndicesCount;

        SetBufferChanged(true);
        delete buffer;

        AfterGenerate(true);
    }

    void Geometry::AfterGenerate(bool successed)
    {
        if (successed)
        {
            m_MeshSource->GetBuffer()->Vertexs = m_TempBuffer.Vertexs;
            m_MeshSource->GetBuffer()->Indexs = m_TempBuffer.Indexs;
            m_MeshSource->GetBuffer()->IndexCount = m_TempBuffer.IndexCount;
            m_MeshSource->GetBuffer()->IndicesCount = m_TempBuffer.IndicesCount;
        }
        else
            Backtrace();

        SetBufferChanged(false);

        Invalidate();
    }
}
