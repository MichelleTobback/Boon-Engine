#pragma once
#include "Renderer/VertexInput.h"

#include <vector>

namespace Boon
{
	class OpenGLVertexInput final : public VertexInput
	{
	public:
		OpenGLVertexInput();
		virtual ~OpenGLVertexInput();

		OpenGLVertexInput(const OpenGLVertexInput& other) = delete;
		OpenGLVertexInput(OpenGLVertexInput&& other) = delete;
		OpenGLVertexInput& operator=(const OpenGLVertexInput& other) = delete;
		OpenGLVertexInput& operator=(OpenGLVertexInput&& other) = delete;

		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& pVertexBuffer) override;
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& pIndexBuffer) override;

	private:
		uint32_t m_ID{};
		uint32_t m_VertexBufferIndex{};
		std::vector<std::shared_ptr<VertexBuffer>> m_pVertexBuffers;
		std::shared_ptr<IndexBuffer> m_pIndexBuffer;
	};
}