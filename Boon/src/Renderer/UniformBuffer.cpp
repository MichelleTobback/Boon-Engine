#include "Renderer/UniformBuffer.h"
#include "Renderer/RenderAPI.h"

#include "Platform/OpenGL/OpenGLUniformBuffer.h"

using namespace Boon;

std::shared_ptr<UniformBuffer> Boon::UniformBuffer::Create(uint32_t size, uint32_t binding)
{
	switch (RenderAPI::GetAPI())
	{
	case ERenderAPI::OpenGL:
		return std::make_shared<OpenGLUniformBuffer>(size, binding);
	}
	return nullptr;
}
