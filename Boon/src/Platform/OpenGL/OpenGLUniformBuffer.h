#pragma once
#include "Renderer/UniformBuffer.h"
#include <string>

namespace Boon
{
	class OpenGLUniformBuffer final : public UniformBuffer
	{
	public:
		OpenGLUniformBuffer(size_t size, uint32_t binding);
		virtual ~OpenGLUniformBuffer();

		OpenGLUniformBuffer(const OpenGLUniformBuffer& other) = delete;
		OpenGLUniformBuffer(OpenGLUniformBuffer&& other) = delete;
		OpenGLUniformBuffer& operator=(const OpenGLUniformBuffer& other) = delete;
		OpenGLUniformBuffer& operator=(OpenGLUniformBuffer&& other) = delete;

	private:
		void SetData(const void* data, size_t size, uint32_t offset) override;

		uint32_t m_ID = 0;
		size_t m_Size = 0;
		uint32_t m_Binding = 0;
	};
}