#pragma once

#include "czpch.h"

#include "Chozo/Renderer/Mesh.h"

namespace Chozo
{

    enum class GeometryType
    {
        Plane = 0, Box, Sphere, Cone, Cylinder
    };

    class Geometry : public Mesh
    {
    public:
        Geometry() = default;
        ~Geometry() = default;

        template<typename T, typename... Args>
        static Ref<T> Create(Args&&... args);

        inline glm::mat4 GetLocalTransform() { return m_LocalTransform; }
        void SetLocalTransform(const glm::mat4& transform);

        inline void SetBufferChanged(bool changed) { m_Is_Buffer_Changed = changed; }
        inline MeshBuffer* GetTempBuffer() { return &m_TempBuffer; }

        void CallGenerate();
    protected:
        void AfterGenerate(bool successed);
        virtual void Backup() {};
        virtual void Backtrace() {};
        virtual MeshBuffer* Generate() = 0;
    protected:
        glm::mat4 m_LocalTransform{1.0f};
    private:
        MeshBuffer m_TempBuffer;
        bool m_Is_Buffer_Changed = false;
    };

    template <typename T, typename... Args>
    inline Ref<T> Geometry::Create(Args &&...args)
    {
        Ref<T> geom = Ref<T>::Create(std::forward<Args>(args)...);
        geom->m_MeshSource = Ref<MeshSource>::Create();
        geom->CallGenerate();

        return geom;
    }
}
