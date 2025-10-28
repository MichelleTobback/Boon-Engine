#pragma once
#include "Renderer/IndexBuffer.h"

namespace Boon
{
	class OpenGLIndexBuffer final : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
		virtual ~OpenGLIndexBuffer();

		OpenGLIndexBuffer(const OpenGLIndexBuffer& other) = delete;
		OpenGLIndexBuffer(OpenGLIndexBuffer&& other) = delete;
		OpenGLIndexBuffer& operator=(const OpenGLIndexBuffer& other) = delete;
		OpenGLIndexBuffer& operator=(OpenGLIndexBuffer&& other) = delete;

		virtual void Bind() const override;
		virtual void Unbind() const override;
		inline virtual uint32_t GetCount() const override { return m_Count; }

	private:
		void Init(uint32_t* indices, uint32_t count);

		uint32_t m_ID{};
		uint32_t m_Count;
	};
}