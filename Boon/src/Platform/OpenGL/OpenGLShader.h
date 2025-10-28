#pragma once
#include "Renderer/Shader.h"
#include <string>

namespace Boon
{
	class OpenGLShader final : public Shader
	{
	public:
		OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~OpenGLShader();

		OpenGLShader(const OpenGLShader& other) = delete;
		OpenGLShader(OpenGLShader&& other) = delete;
		OpenGLShader& operator=(const OpenGLShader& other) = delete;
		OpenGLShader& operator=(OpenGLShader&& other) = delete;

		virtual void Bind() const override;
		virtual void Unbind() const override;

	private:
		uint32_t m_ID;
	};
}