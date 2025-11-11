#pragma once
#include "VertexBufferLayout.h"

#include <functional>
#include <memory>

namespace Boon
{
    enum class PrimitiveType
    {
        Triangles,
        Lines
    };

    class Shader;
    class IndexBuffer;
    class VertexBuffer;
    class VertexInput;
    class RenderBatch
    {
    public:
        using BatchCallback = std::function<void()>;

        void Initialize(uint32_t maxVertices,
            const VertexBufferLayout& layout,
            const std::shared_ptr<Shader>& shader,
            PrimitiveType primitiveType,
            std::shared_ptr<IndexBuffer> indexBuffer = nullptr);

        RenderBatch() = default;
        ~RenderBatch();

        void Begin();
        void Flush();
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

        inline void SetShader(const std::shared_ptr<Shader>& shader) { m_Shader = shader; }
        inline void SetPrimitiveType(PrimitiveType type) { m_PrimitiveType = type; }

        inline uint32_t GetVertexCount() const { return m_VertexCount; }

    private:
        std::shared_ptr<VertexInput>  m_VertexInput;
        std::shared_ptr<VertexBuffer> m_VertexBuffer;
        std::shared_ptr<IndexBuffer>  m_IndexBuffer;
        std::shared_ptr<Shader>       m_Shader;

        uint32_t m_MaxVertices = 0;
        uint32_t m_VertexCount = 0;

        uint8_t* m_BufferBase = nullptr;
        uint8_t* m_BufferPtr = nullptr;

        PrimitiveType m_PrimitiveType;

        BatchCallback m_PreFlushFunc;
        BatchCallback m_PostFlushFunc;
        BatchCallback m_BeginBatchFunc;
    };
}
