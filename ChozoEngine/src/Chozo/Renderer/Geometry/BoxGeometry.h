#pragma once

#include "Geometry.h"

namespace Chozo
{

    class BoxGeometry : public Geometry
    {
    public:
        BoxGeometry(const float& width = 1.0f, const float& height = 1.0f, const float& depth = 1.0f, const uint32_t& widthSegments = 1, const uint32_t& heightSegments = 1, const uint32_t& depthSegments = 1);
        ~BoxGeometry() = default;

        inline float GetWidth() { return m_Width; }
        inline void SetWidth(float& width) {
            Backup();
            m_Width = width; CallGenerate();
        }

        inline float GetHeight() { return m_Height; }
        inline void SetHeight(float& height) {
            Backup();
            m_Height = height; CallGenerate();
        }

        inline float GetDepth() { return m_Depth; }
        inline void SetDepth(float& depth) {
            Backup();
            m_Depth = depth; CallGenerate();
        }

        inline uint32_t GetWidthSegments() { return m_WidthSegments; }
        inline void SetWidthSegments(uint32_t& widthSegments) {
            Backup();
            m_WidthSegments = widthSegments; CallGenerate();
        }

        inline uint32_t GetHeightSegments() { return m_HeightSegments; }
        inline void SetHeightSegments(uint32_t& heightSegments) {
            Backup();
            m_HeightSegments = heightSegments; CallGenerate();
        }

        inline uint32_t GetDepthSegments() { return m_DepthSegments; }
        inline void SetDepthSegments(uint32_t& depthSegments) {
            Backup();
            m_DepthSegments = depthSegments; CallGenerate();
        }
    protected:
        virtual void Backup() override;
        virtual void Backtrace() override;
        virtual MeshBuffer* Generate() override;
    private:
        void BuildPlane(std::string u, std::string v, std::string w, int uDir, int vDir, float width, float height, float depth, uint32_t gridX, uint32_t gridY, MeshBuffer* buffer);
    private:
        float m_Width, m_Height, m_Depth;
        uint32_t m_WidthSegments, m_HeightSegments, m_DepthSegments;
        uint32_t m_NumberOfVertices = 0;

        float m_OldWidth, m_OldHeight, m_OldDepth;
        uint32_t m_OldWidthSegments, m_OldHeightSegments, m_OldDepthSegments;
    };
}
