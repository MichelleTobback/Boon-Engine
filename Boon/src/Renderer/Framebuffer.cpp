#include "Renderer/Framebuffer.h"
#include "Renderer/RenderApi.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"

using namespace Boon;

std::shared_ptr<Framebuffer> Boon::Framebuffer::Create(const FramebufferDescriptor& spec)
{
	switch (RenderAPI::GetAPI())
	{
	case ERenderAPI::OpenGL:
		return std::make_shared<OpenGLFramebuffer>(spec);
	}
	return nullptr;
}
