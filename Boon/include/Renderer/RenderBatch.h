#pragma once
#include <Renderer/RendererTypes.h>

#include <functional>
#include <memory>

namespace Boon
{
    class Material;
    class IndexBuffer;
    class VertexBuffer;
    class VertexInput;
    class RenderBatch
    {
    public:
        using BatchCallback = std::function<void()>;

        /**
         * @brief Initialize the render batch with required resources.
         *
         * @param maxVertices Maximum number of vertices per batch.
         * @param pipeline Pipeline used when flushing the batch.
         * @param indexBuffer Optional index buffer for indexed rendering.
         */
        void Initialize(uint32_t maxVertices,
            const std::shared_ptr<Material>& material,
            std::shared_ptr<IndexBuffer> indexBuffer = nullptr);

        RenderBatch() = default;

        /**
         * @brief Destroy the render batch and release resources.
         */
        ~RenderBatch();

        /**
         * @brief Start recording vertices for the batch.
         */
        void Begin();

        /**
         * @brief Flush the current batch to the GPU and issue draw calls.
         */
        void Flush();

        /**
         * @brief Advance to the next batch (used when capacity is reached).
         */
        void NextBatch();

        template<typename VertexType>
        inline VertexType& PushVertex()
        {
            if (m_VertexCount >= m_MaxVertices)
                NextBatch();

            VertexType* v = reinterpret_cast<VertexType*>(m_BufferPtr);
            m_BufferPtr += sizeof(VertexType);
            m_VertexCount++;
            return *v;
        }

        inline void BindPreFlushCallback(BatchCallback cb) { m_PreFlushFunc = cb; }
        inline void BindPostFlushCallback(BatchCallback cb) { m_PostFlushFunc = cb; }
        inline void BindBeginBatchCallback(BatchCallback cb) { m_BeginBatchFunc = cb; }

        inline void SetMaterial(const std::shared_ptr<Material>& material) { m_pMaterial = material; }

        inline uint32_t GetVertexCount() const { return m_VertexCount; }

    private:
        std::shared_ptr<VertexInput>  m_VertexInput;
        std::shared_ptr<VertexBuffer> m_VertexBuffer;
        std::shared_ptr<IndexBuffer>  m_IndexBuffer;
        std::shared_ptr<Material> m_pMaterial;

        uint32_t m_MaxVertices = 0;
        uint32_t m_VertexCount = 0;

        uint8_t* m_BufferBase = nullptr;
        uint8_t* m_BufferPtr = nullptr;

        BatchCallback m_PreFlushFunc;
        BatchCallback m_PostFlushFunc;
        BatchCallback m_BeginBatchFunc;
    };
}
