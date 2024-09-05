#pragma once

#include "czpch.h"

#include "Chozo/Renderer/Mesh.h"

namespace Chozo
{

    enum class GeometryType
    {
        Plane = 0, Box, Sphere, Cone, Cylinder
    };

    class Geometry : public MeshSource
    {
    public:
        Geometry() = default;
        ~Geometry() = default;

        // static Ref<Geometry> Create(const GeometryType type, GeometryProps props);

        operator bool() const { return m_Buffer.IndexCount != 0; }
    protected:
        virtual void Backup() {};
        virtual void Backtrace() {};
        virtual MeshBuffer* Generate() { return nullptr; };
    };
}
