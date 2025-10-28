#include "Renderer/Shader.h"
#include "Renderer/RenderApi.h"
#include "Platform/OpenGL/OpenGLShader.h"

using namespace Boon;

std::shared_ptr<Shader> Boon::Shader::Create(const std::string& vertexSrc, const std::string& fragmentSrc)
{
	switch (RenderAPI::GetAPI())
	{
	case ERenderAPI::OpenGL:
		return std::make_shared<OpenGLShader>(vertexSrc, fragmentSrc);
	}
	return nullptr;
}
