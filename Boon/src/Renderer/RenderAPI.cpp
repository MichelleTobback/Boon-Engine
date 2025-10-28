#include "Renderer/RenderAPI.h"

#include "../Platform/OpenGL/OpenGLApi.h"

using namespace Boon;

RenderAPI Boon::BaseRenderAPI::s_Api = RenderAPI::OpenGL;

std::unique_ptr<Boon::BaseRenderAPI> Boon::BaseRenderAPI::Create()
{
	switch (s_Api)
	{
	case RenderAPI::OpenGL:
		return std::move(std::make_unique<OpenGLApi>());
	}

	return nullptr;
}
