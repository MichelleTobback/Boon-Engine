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

void Boon::Renderer::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) 
{ 
	s_pApi->SetViewport(x, y, width, height); 
}

void Boon::Renderer::SetClearColor(const glm::vec4& color) 
{ 
	s_pApi->SetClearColor(color); 
}

void Boon::Renderer::Clear() 
{ 
	s_pApi->Clear(); 
}

void Boon::Renderer::DrawLines(const std::shared_ptr<VertexInput>& vertexInput, uint32_t lineCount)
{
	s_pApi->DrawLines(vertexInput, lineCount);
}

void Boon::Renderer::DrawIndexed(const std::shared_ptr<VertexInput>& vertexInput, uint32_t indexCount) 
{ 
	s_pApi->DrawIndexed(vertexInput, indexCount); 
}
