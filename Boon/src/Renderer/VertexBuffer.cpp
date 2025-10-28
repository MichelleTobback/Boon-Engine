#include "Renderer/VertexBuffer.h"
#include "Renderer/RenderApi.h"

#include "Platform/OpenGL/OpenGLVertexBuffer.h"

using namespace Boon;

std::shared_ptr<VertexBuffer> Boon::VertexBuffer::Create(float* vertices, uint32_t size)
{
	switch (RenderAPI::GetAPI())
	{
	case ERenderAPI::OpenGL:
		return std::make_shared<OpenGLVertexBuffer>(vertices, size);
	}
	return nullptr;
}

std::shared_ptr<VertexBuffer> Boon::VertexBuffer::Create(uint32_t size)
{
	switch (RenderAPI::GetAPI())
	{
	case ERenderAPI::OpenGL:
		return std::make_shared<OpenGLVertexBuffer>(size);
	}
	return nullptr;
}
