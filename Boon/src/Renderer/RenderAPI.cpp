#include "Renderer/RenderAPI.h"

#include "../Platform/OpenGL/OpenGLApi.h"

using namespace Boon;

ERenderAPI Boon::BaseRenderAPI::s_Api = ERenderAPI::OpenGL;

std::unique_ptr<Boon::BaseRenderAPI> Boon::BaseRenderAPI::Create()
{
	switch (s_Api)
	{
	case ERenderAPI::OpenGL:
		return std::move(std::make_unique<OpenGLApi>());
	}

	return nullptr;
}
