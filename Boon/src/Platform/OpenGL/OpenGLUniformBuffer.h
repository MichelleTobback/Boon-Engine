#pragma once
#include "Renderer/UniformBuffer.h"
#include <string>

namespace Boon
{
	class OpenGLUniformBuffer final : public UniformBuffer
	{
	public:
		OpenGLUniformBuffer(uint32_t size, uint32_t binding);
		virtual ~OpenGLUniformBuffer();

		OpenGLUniformBuffer(const OpenGLUniformBuffer& other) = delete;
		OpenGLUniformBuffer(OpenGLUniformBuffer&& other) = delete;
		OpenGLUniformBuffer& operator=(const OpenGLUniformBuffer& other) = delete;
		OpenGLUniformBuffer& operator=(OpenGLUniformBuffer&& other) = delete;

	private:
		uint32_t m_ID;

		// Inherited via UniformBuffer
		void SetData(const void* data, uint32_t size, uint32_t offset) override;
	};
}