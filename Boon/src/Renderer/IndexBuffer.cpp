#include "Renderer/IndexBuffer.h"
#include "Renderer/RenderApi.h"

#include "Platform/OpenGL/OpenGLIndexBuffer.h"

using namespace Boon;

std::shared_ptr<IndexBuffer> Boon::IndexBuffer::Create(uint32_t* indices, uint32_t count)
{
	switch (RenderAPI::GetAPI())
	{
	case ERenderAPI::OpenGL:
		return std::make_shared<OpenGLIndexBuffer>(indices, count);
	}
	return nullptr;
}