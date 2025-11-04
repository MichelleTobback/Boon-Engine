#include "Renderer/Texture.h"
#include "Renderer/RenderApi.h"

#include "Platform/OpenGL/OpenGLTexture.h"

using namespace Boon;

std::shared_ptr<Texture2D> Boon::Texture2D::Create(const TextureDescriptor& descriptor)
{
	switch (RenderAPI::GetAPI())
	{
	case ERenderAPI::OpenGL:
		return std::make_shared<OpenGLTexture2D>(descriptor);
	}
	return nullptr;
}
