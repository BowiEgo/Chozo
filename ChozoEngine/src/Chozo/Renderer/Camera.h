#pragma once

#include "czpch.h"
#include <glm/glm.hpp>

namespace Chozo {

    class Camera
    {
    public:
        Camera() = default;
        Camera(const glm::mat4& projection)
            : m_ProjectionMatrix(projection) {}

        virtual ~Camera() = default;

        virtual void Reset() = 0;

        const glm::mat4& GetProjection() const { return m_ProjectionMatrix; }
    protected:
        glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
    };
}
