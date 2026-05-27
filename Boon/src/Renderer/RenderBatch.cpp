#include "Renderer/RenderBatch.h"
#include "Renderer/VertexInput.h"
#include "Renderer/VertexBuffer.h"
#include "Renderer/IndexBuffer.h"
#include "Renderer/Shader.h"
#include "Renderer/Renderer.h"
#include "Renderer/Material.h"

namespace Boon
{
    void RenderBatch::Initialize(uint32_t maxVertices, const std::shared_ptr<Material>& material, std::shared_ptr<IndexBuffer> indexBuffer)
    {
        m_MaxVertices = maxVertices;
        m_IndexBuffer = indexBuffer;
        m_pMaterial = material;

        const PipelineDescriptor& desc = m_pMaterial->GetPipeline()->GetDescriptor();

        // Create vertex input and buffers
        m_VertexInput = VertexInput::Create();
        m_VertexBuffer = VertexBuffer::Create(maxVertices * desc.Layout.GetStride());
        m_VertexBuffer->SetLayout(desc.Layout);
        m_VertexInput->AddVertexBuffer(m_VertexBuffer);

        if (m_IndexBuffer)
            m_VertexInput->SetIndexBuffer(m_IndexBuffer);

        m_BufferBase = new uint8_t[maxVertices * desc.Layout.GetStride()];
    }

    RenderBatch::~RenderBatch()
    {
        if (m_BufferBase)
            delete[] m_BufferBase;
    }

    void RenderBatch::Begin()
    {
        m_VertexCount = 0;
        m_BufferPtr = m_BufferBase;

        if (m_BeginBatchFunc) m_BeginBatchFunc();
    }

    void RenderBatch::Flush()
    {
        if (!m_pMaterial || m_VertexCount == 0)
            return;

        if (m_PreFlushFunc)
            m_PreFlushFunc();

        uint32_t dataSize = static_cast<uint32_t>(m_BufferPtr - m_BufferBase);
        if (dataSize > 0)
            m_VertexBuffer->SetData(m_BufferBase, dataSize);

        m_pMaterial->Bind();

        switch (m_pMaterial->GetPipeline()->GetDescriptor().Primitive)
        {
        case PrimitiveType::Triangles:
            if (m_IndexBuffer)
            {
                // Use static index buffer (like quads/meshes)
                Renderer::DrawIndexed(m_VertexInput, m_IndexBuffer->GetCount());
            }
            else
            {
                // Non-indexed geometry (like circle fan or custom procedural)
                Renderer::DrawArrays(m_VertexInput, m_VertexCount);
            }
            break;

        case PrimitiveType::Lines:
            Renderer::DrawLines(m_VertexInput, m_VertexCount / 2);
            break;
        }

        if (m_PostFlushFunc)
            m_PostFlushFunc();

        m_pMaterial->Unbind();
    }

    void RenderBatch::NextBatch()
    {
        Flush();
        Begin();
    }
}
