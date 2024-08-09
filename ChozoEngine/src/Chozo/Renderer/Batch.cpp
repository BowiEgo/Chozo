#include "Batch.h"


namespace Chozo
{

    template <typename T>
    Batch<T>::Batch()
        : m_Count(0)
    {
        uint32_t maxCount = GetMaxCount<T>();
        m_BufferBase = new T[maxCount];
        m_BufferPtr = m_BufferBase;
    }

    template <typename T>
    Batch<T>::~Batch()
    {
        delete[] m_BufferBase;
    }

    template <typename T>
    uint32_t Batch<T>::AddBuffers(const T* buffer, uint32_t count)
    {
        uint32_t maxCount = GetMaxCount<T>();
        if (m_Count + count > maxCount)
            return 0;
        
        std::copy(buffer, buffer + count, m_BufferPtr);
        m_Count += count;
        m_BufferPtr += count;

        return m_Count - count;
    }

    template <typename T>
    uint32_t Batch<T>::AddBuffers(const T* buffer, uint32_t count, uint32_t vertexOffset)
    {
        uint32_t maxCount = GetMaxCount<T>();
        if (m_Count + count > maxCount)
            return 0;
        
        std::transform(buffer, buffer + count, m_BufferPtr, [vertexOffset](const T& v) {
            return T{ v.V1 + vertexOffset, v.V2 + vertexOffset, v.V3 + vertexOffset };
        });
        m_Count += count;
        m_BufferPtr += count;

        return m_Count - count;
    }

    template <typename T>
    bool Batch<T>::RemoveBuffers(uint32_t start, uint32_t count)
    {
        if (start + count > m_Count)
            return false;

        for (uint32_t i = start; i < start + count; ++i)
        {
            m_BufferBase[i] = T();
        }
        
        if (start + count != m_Count)
            std::copy(m_BufferBase + start + count, m_BufferPtr, m_BufferBase + start);
        
        m_BufferPtr -= count;
        m_Count -= count;
        return true;
    }

    template <typename T>
    bool Batch<T>::RemoveBuffers(uint32_t start, uint32_t count, uint32_t transformOffset)
    {
        if (start + count > m_Count)
            return false;

        for (uint32_t i = start; i < start + count; ++i)
        {
            m_BufferBase[i] = T();
        }

        if (start + count != m_Count)
        {
            std::transform(m_BufferBase + start + count, m_BufferPtr, m_BufferBase + start, [transformOffset](const T& v) {
                return T{ v.V1 - transformOffset, v.V2 - transformOffset, v.V3 - transformOffset };
            });
        }
        
        m_BufferPtr -= count;
        m_Count -= count;
        return true;
    }

    UUID BatchManager::SubmitBuffers(const Vertex* vertexBuffer, uint32_t vertexCount, const Index* indexBuffer, uint32_t indexCount, const UUID& segmentID)
    {
        if (vertexCount == 0 || indexCount == 0 )
        {
            CZ_CORE_WARN("BatchManager: Buffer Count should not be 0.");
            return UUID::Invalid();
        }
        if (vertexCount > GetMaxCount<Vertex>())
        {
            CZ_CORE_WARN("BatchManager: Vertex Count is larger than maximum count.");
            return UUID::Invalid();
        }
        if (indexCount > GetMaxCount<Index>())
        {
            CZ_CORE_WARN("BatchManager: Index Count is larger than maximum count.");
            return UUID::Invalid();
        }

        BufferSegment segment = GetSegment(segmentID);
        // CZ_CORE_TRACE("Segment: {0}", segment.ToString());

        // Remove buffer datas if our vertex datas and index datas has allready been submitted.
        if (segment.isValid())
            RemoveBuffers(segment);

        return AddBuffers(vertexBuffer, vertexCount, indexBuffer, indexCount, segment)->ID;
    }

    BufferSegment *BatchManager::RemoveBuffers(BufferSegment& segment)
    {
        Ref<Batch<Vertex>> vertexBatch = m_VertexBatches[segment.VertexBatchIndex];
        uint32_t oldVertexCount = vertexBatch->GetCount();
        vertexBatch->RemoveBuffers(segment.VertexStart, segment.VertexCount);

        Ref<Batch<Index>> indexBatch = m_IndexBatches[segment.IndexBatchIndex];
        uint32_t oldIndexCount = indexBatch->GetCount();
        indexBatch->RemoveBuffers(segment.IndexStart, segment.IndexCount, segment.VertexCount);

        uint32_t idx = GetBatchIndex(indexBatch);
        // CZ_CORE_INFO("RemoveBuffers: idx: {0}, vertexCount: {1}, indexCount: {2}", idx, oldVertexCount, oldIndexCount);
        UpdateRenderSource(idx, oldVertexCount, oldIndexCount);

        // Change the start position of the segments behind target segment.
        for (auto& pair : m_BufferSegments) {
            BufferSegment& seg = pair.second;
            if (seg.VertexBatchIndex == idx)
            {
                if (seg.VertexStart >= segment.VertexStart + segment.VertexCount)
                    seg.VertexStart -= segment.VertexCount;
            }
            if (seg.IndexBatchIndex == idx)
            {
                if (seg.IndexStart >= segment.IndexStart + segment.IndexCount)
                    seg.IndexStart -= segment.IndexCount;
            }
        }

        return &segment;
    }

    BufferSegment *BatchManager::AddBuffers(const Vertex* vertexBuffer, const uint32_t vertexCount, const Index* indexBuffer, uint32_t indexCount, BufferSegment &segment)
    {
        Ref<Batch<Vertex>> vertexBatch = nullptr;
        Ref<Batch<Index>> indexBatch = nullptr;

        // Check if the batches of the segment is available, if is, we add buffers to the batches.
        if (IsBatchAvailable<Vertex>(segment.VertexBatchIndex, vertexCount) && IsBatchAvailable<Index>(segment.IndexBatchIndex, indexCount))
        {
            vertexBatch = m_VertexBatches[segment.VertexBatchIndex];
            indexBatch = m_IndexBatches[segment.IndexBatchIndex];
        }
        // If not, we find another batches available.
        else
        {
            indexBatch = GetAvailableBatch<Index>(indexCount);
            if (indexBatch != nullptr)
                vertexBatch = m_VertexBatches[GetBatchIndex(indexBatch)];
        }

        // If there's no existed batches available, create new batches.
        if (vertexBatch == nullptr || indexBatch == nullptr) {
            auto [newVertexBatch, newIndexBatch] = CreateBatches();
            vertexBatch = newVertexBatch;
            indexBatch = newIndexBatch;
        }

        uint32_t vertexPos = vertexBatch->AddBuffers(vertexBuffer, vertexCount);
        uint32_t indexPos = indexBatch->AddBuffers(indexBuffer, indexCount, vertexPos);
        uint32_t idx = GetBatchIndex<Index>(indexBatch);

        segment.VertexBatchIndex = idx;
        segment.IndexBatchIndex = idx;
        segment.VertexStart = vertexPos;
        segment.VertexCount = vertexCount;
        segment.IndexStart = indexPos;
        segment.IndexCount = indexCount;

        m_BufferSegments[segment.ID] = segment;

        UpdateRenderSource(idx, vertexBatch->GetCount(), indexBatch->GetCount());

        return &segment;
    }

    std::pair<Ref<Batch<Vertex>>, Ref<Batch<Index>>> BatchManager::CreateBatches()
    {
        Ref<Batch<Vertex>> vertexBatch = std::make_shared<Batch<Vertex>>();
        m_VertexBatches.push_back(vertexBatch);

        Ref<Batch<Index>> indexBatch = std::make_shared<Batch<Index>>();
        m_IndexBatches.push_back(indexBatch);

        // Crate RenderSource for GPU
        Ref<RenderSource> renderSource = std::make_shared<RenderSource>(GetMaxCount<Vertex>(), GetMaxCount<Index>());
        m_RenderSources[m_VertexBatches.size() - 1] = renderSource;
        // CZ_CORE_INFO("CreateRenderSource: VAO, {0}", renderSource->VAO->GetRendererID());

        return { vertexBatch, indexBatch };
    }

    void BatchManager::UpdateRenderSource(const uint32_t idx, const uint32_t vertexCount, const uint32_t indexCount)
    {
        if (m_RenderSources.find(idx) != m_RenderSources.end())
        {
            Ref<Batch<Vertex>> vertexBatch = m_VertexBatches[idx];
            Ref<Batch<Index>> indexBatch = m_IndexBatches[idx];

            m_RenderSources[idx]->VBO->SetData(0, vertexCount * sizeof(Vertex), (float*)vertexBatch->GetBuffer());
            m_RenderSources[idx]->IBO->SetData(0, indexCount * 3, (unsigned int*)indexBatch->GetBuffer());
        }
    }

    BufferSegment BatchManager::GetSegment(const UUID& id) const
    {
        auto it = m_BufferSegments.find(id);
        if (it != m_BufferSegments.end()) {
            return it->second;
        }
        return BufferSegment(); // Return an invalid segment if not found
    }

    bool BatchManager::RemoveSegment(UUID segmentID)
    {
        BufferSegment segment = GetSegment(segmentID);
        if (!segment.isValid())
            return false;

        RemoveBuffers(segment);

        size_t erasedCount = m_BufferSegments.erase(segmentID);
        return erasedCount > 0;
    }

    template<>
    std::vector<Ref<Batch<Vertex>>> BatchManager::GetBatches<Vertex>() const
    {
        return m_VertexBatches;
    }

    template<>
    std::vector<Ref<Batch<Index>>> BatchManager::GetBatches<Index>() const
    {
        return m_IndexBatches;
    }

    template <>
    Ref<Batch<Vertex>> BatchManager::GetAvailableBatch(const uint32_t count)
    {
        uint32_t maxCount = GetMaxCount<Vertex>();
        std::vector<Ref<Batch<Vertex>>> batches = GetBatches<Vertex>();
        for (auto& batch : batches)
        {
            if (batch->GetCount() + count <= maxCount)
                return batch;
        }
        return nullptr;
    }

    template <>
    bool BatchManager::IsBatchAvailable<Vertex>(const uint32_t batchIndex, const uint32_t count)
    {
        if (batchIndex >= m_VertexBatches.size()) {
            return false;
        }
        Ref<Batch<Vertex>> batch = m_VertexBatches[batchIndex];
        if (!batch)
            return false;

        uint32_t maxCount = GetMaxCount<Vertex>();
        return batch->GetCount() + count <= maxCount;
    }

    template <>
    bool BatchManager::IsBatchAvailable<Index>(const uint32_t batchIndex, const uint32_t count)
    {
        if (batchIndex >= m_IndexBatches.size()) {
            return false;
        }

        Ref<Batch<Index>> batch = m_IndexBatches[batchIndex];
        if (!batch)
            return false;

        uint32_t maxCount = GetMaxCount<Index>();
        return batch->GetCount() + count <= maxCount;
    }

    template <>
    Ref<Batch<Index>> BatchManager::GetAvailableBatch(const uint32_t count)
    {
        uint32_t maxCount = GetMaxCount<Index>();
        std::vector<Ref<Batch<Index>>> batches = GetBatches<Index>();
        for (auto& batch : batches)
        {
            if (batch->GetCount() + count <= maxCount)
                return batch;
        }
        return nullptr;
    }

    template<>
    int BatchManager::GetBatchIndex(const Ref<Batch<Vertex>>& batch) const {
        std::vector<Ref<Batch<Vertex>>> batches = GetBatches<Vertex>();

        auto it = std::find(batches.begin(), batches.end(), batch);
        if (it != batches.end())
            return std::distance(batches.begin(), it);
        else
            return -1;
    }

    template<>
    int BatchManager::GetBatchIndex(const Ref<Batch<Index>>& batch) const {
        std::vector<Ref<Batch<Index>>> batches = GetBatches<Index>();

        auto it = std::find(batches.begin(), batches.end(), batch);
        if (it != batches.end())
            return std::distance(batches.begin(), it);
        else
            return -1;
    }
}
