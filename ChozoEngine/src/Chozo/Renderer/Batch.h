#pragma once

#include "czpch.h"
#include "Chozo/Core/UUID.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

#include "DataStructs.h"

namespace Chozo
{
    static const uint32_t s_MaxIndexCount = 200000;

    template <typename T>
    struct MaxCount {
        static const uint32_t value = 0;
    };

    // Right now, we set the max Vertex count same with the max Index count,
    // Because usually the former is larger the latter. And it will greatly simplify the buffer processing.
    template <>
    struct MaxCount<Vertex> {
        static const uint32_t value = s_MaxIndexCount;
    };

    template <>
    struct MaxCount<Index> {
        static const uint32_t value = s_MaxIndexCount;
    };

    template <typename T>
    uint32_t GetMaxCount() {
        return MaxCount<T>::value;
    };

    template<typename T>
    class Batch
    {
    public:

        Batch();
        ~Batch();

        T* GetBuffer() { return m_BufferBase; }
        uint32_t GetCount() { return uint32_t(m_BufferPtr - m_BufferBase); }

        uint32_t AddBuffers(const T* buffer, uint32_t count);
        uint32_t AddBuffers(const T* buffer, uint32_t count, uint32_t transformOffset);

        bool RemoveBuffers(uint32_t start, uint32_t count);
        bool RemoveBuffers(uint32_t start, uint32_t count, uint32_t transformOffset);

        bool operator==(const Batch& other) const
        {
            return this == &other;
        }
    private:
        T* m_BufferBase;
        T* m_BufferPtr = nullptr;
        uint32_t m_Count;
    };

    struct BufferSegment
    {
        UUID ID;
        uint32_t VertexBatchIndex, IndexBatchIndex;
        uint32_t VertexStart, IndexStart;
        uint32_t VertexCount, IndexCount;

        BufferSegment() : VertexBatchIndex(0), IndexBatchIndex(0), VertexStart(0), IndexStart(0), VertexCount(0), IndexCount(0) {}
        bool isValid()
        {
            return ID.isValid() && (VertexCount != 0 || IndexCount != 0);
        }

        std::string ToString() const
        {
            std::stringstream ss;
            ss << "ID: " << ID
                << ", VertexBatchIndex: " << VertexBatchIndex << ", VertexStart:" << VertexStart << ", VertexCount:" << VertexCount
                << ", IndexBatchIndex:" << IndexBatchIndex << ", IndexStart:" << IndexStart << ", IndexCount:" << IndexCount;
            return ss.str();
        }
    };
    inline std::ostream& operator<<(std::ostream& os, const BufferSegment& segment)
    {
        return os << segment.ToString(); 
    }

    class BatchManager {
    public:
        template<typename T>
        std::vector<Ref<Batch<T>>> GetBatches() const;
        inline std::map<uint32_t, Ref<RenderSource>> GetRenderSources() const { return m_RenderSources; }

        UUID SubmitBuffers(const Vertex* vertexBuffer, uint32_t vertexCount, const Index* indexBuffer, uint32_t indexCount, const UUID& segmentID = UUID::Invalid());
        bool RemoveSegment(UUID segmentID);
    private:
        BufferSegment* RemoveBuffers(BufferSegment& segment);
        BufferSegment* AddBuffers(const Vertex* vertexBuffer, const uint32_t vertexCount, const Index* indexBuffer, uint32_t indexCount, BufferSegment& segment);
        std::pair<Ref<Batch<Vertex>>, Ref<Batch<Index>>> CreateBatches();
        void UpdateRenderSource(const uint32_t idx, const uint32_t vertexCount, const uint32_t indexCount);
        
        BufferSegment GetSegment(const UUID& id) const;
        template<typename T>
        Ref<Batch<T>> GetAvailableBatch(const uint32_t count);

        template<typename T>
        bool IsBatchAvailable(const uint32_t batchIndex, const uint32_t count);

        template<typename T>
        int GetBatchIndex(const Ref<Batch<T>>& batch) const;
    private:
        std::vector<Ref<Batch<Vertex>>> m_VertexBatches;
        std::vector<Ref<Batch<Index>>> m_IndexBatches;
        std::unordered_map<UUID, BufferSegment> m_BufferSegments;
        std::map<uint32_t, Ref<RenderSource>> m_RenderSources;
    };

    template<>
    std::vector<Ref<Batch<Vertex>>> BatchManager::GetBatches<Vertex>() const;

    template<>
    std::vector<Ref<Batch<Index>>> BatchManager::GetBatches<Index>() const;

    template <>
    Ref<Batch<Vertex>> BatchManager::GetAvailableBatch(uint32_t count);

    template <>
    Ref<Batch<Index>> BatchManager::GetAvailableBatch(uint32_t count);
    
    template<>
    bool BatchManager::IsBatchAvailable<Vertex>(uint32_t batchIndex, uint32_t count);

    template<>
    bool BatchManager::IsBatchAvailable<Index>(uint32_t batchIndex, uint32_t count);

    template<>
    int BatchManager::GetBatchIndex(const Ref<Batch<Vertex>>& batch) const;

    template<>
    int BatchManager::GetBatchIndex(const Ref<Batch<Index>>& batch) const;
}
