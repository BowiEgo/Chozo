#pragma once

#include "ProceduralMesh.h"

class RENDER_CORE_API FCube : public FProceduralMesh {
public:
    explicit FCube(const float& width = 1.0f, const float& height = 1.0f, const float& depth = 1.0f,
                   const uint32& widthSegments = 1, const uint32& heightSegments = 1,
                   const uint32& depthSegments = 1);
    ~FCube() override = default;

    inline float GetWidth() const { return m_Width; }
    inline void SetWidth(const float& width) {
        Backup();
        m_Width = width;
    }

    inline float GetHeight() const { return m_Height; }
    inline void SetHeight(const float& height) {
        Backup();
        m_Height = height;
    }

    inline float GetDepth() const { return m_Depth; }
    inline void SetDepth(float& depth) {
        Backup();
        m_Depth = depth;
    }

    inline uint32 GetWidthSegments() const { return m_WidthSegments; }
    inline void SetWidthSegments(const uint32& widthSegments) {
        Backup();
        m_WidthSegments = widthSegments;
    }

    inline uint32 GetHeightSegments() const { return m_HeightSegments; }
    inline void SetHeightSegments(const uint32& heightSegments) {
        Backup();
        m_HeightSegments = heightSegments;
    }

    inline uint32 GetDepthSegments() const { return m_DepthSegments; }
    inline void SetDepthSegments(const uint32& depthSegments) {
        Backup();
        m_DepthSegments = depthSegments;
    }

protected:
    void Backup() override;
    void Backtrace() override;
    FMeshBuffer* Generate() override;

private:
    void BuildPlane(std::string u, std::string v, std::string w, int uDir, int vDir, float width,
                    float height, float depth, uint32 gridX, uint32 gridY, FMeshBuffer* buffer);

private:
    float m_Width, m_Height, m_Depth;
    uint32 m_WidthSegments, m_HeightSegments, m_DepthSegments;
    uint32 m_NumberOfVertices = 0;

    float m_OldWidth, m_OldHeight, m_OldDepth;
    uint32 m_OldWidthSegments, m_OldHeightSegments, m_OldDepthSegments;
};