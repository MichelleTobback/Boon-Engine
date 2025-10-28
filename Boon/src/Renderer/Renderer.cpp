#include "Renderer/Renderer.h"
#include "Renderer/RenderAPI.h"

using namespace Boon;

std::unique_ptr<BaseRenderAPI> Boon::Renderer::s_pApi = BaseRenderAPI::Create();

void Boon::Renderer::Init()
{
	s_pApi->Init();
}

void Boon::Renderer::Shutdown()
{
	s_pApi->Shutdown();
}

void Boon::Renderer::BeginFrame()
{
	s_pApi->BeginFrame();
}

void Boon::Renderer::EndFrame()
{
	s_pApi->EndFrame();
}
