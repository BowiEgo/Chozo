#pragma once

#include "czpch.h"

#include "Chozo/Renderer/Mesh.h"

namespace Chozo
{

    enum class GeometryType
    {
        Plane = 0, Box, Sphere, Cone, Cylinder
    };

    class Geometry
    {
    public:
        Geometry() {};
        ~Geometry() = default;

        std::vector<Vertex> const GetVertices() { return m_Vertices; }
        std::vector<Index> const GetIndices() { return m_Indices; }
        uint32_t const GetIndexCount() { return m_IndexCount; }
        uint32_t const GetTriangleCount() { return m_TriangleCount; }

        static Geometry Create(const GeometryType type);
    protected:
		std::vector<Vertex> m_Vertices;
		std::vector<Index> m_Indices;

        uint32_t m_IndexCount = 0, m_TriangleCount = 0;
    };
}
