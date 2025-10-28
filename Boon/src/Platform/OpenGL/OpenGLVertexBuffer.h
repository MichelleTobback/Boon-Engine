#pragma once
#include "Renderer/VertexBuffer.h"

namespace Boon
{
	class OpenGLVertexBuffer final : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(float* vertices, uint32_t size);
		OpenGLVertexBuffer(uint32_t size);
		virtual ~OpenGLVertexBuffer();

		OpenGLVertexBuffer(const OpenGLVertexBuffer& other) = delete;
		OpenGLVertexBuffer(OpenGLVertexBuffer&& other) = delete;
		OpenGLVertexBuffer& operator=(const OpenGLVertexBuffer& other) = delete;
		OpenGLVertexBuffer& operator=(OpenGLVertexBuffer&& other) = delete;

		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual void SetData(const void* data, uint32_t size) override;

		inline virtual const VertexBufferLayout& GetLayout() const override { return m_Layout; }
		inline virtual void SetLayout(const VertexBufferLayout& layout) override { m_Layout = layout; }

	private:
		void Init(float* vertices, uint32_t size);

		uint32_t m_ID;
		VertexBufferLayout m_Layout;
	};
}