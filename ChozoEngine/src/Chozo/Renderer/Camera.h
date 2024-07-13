#pragma once

namespace Chozo {

    class Camera
    {
    public:
        Camera() = default;
        Camera(const glm::mat4& projection) : m_ProjectionMatrix(projection) {}

        const glm::mat4& GetProjection() const { return m_ProjectionMatrix; }
    private:
        glm::mat4 m_ProjectionMatrix;
    };
}
