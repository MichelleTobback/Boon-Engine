#include "Renderer/SceneRenderer.h"
#include "Renderer/Renderer.h"

using namespace Boon;

void Boon::SceneRenderer::Render()
{
	Renderer::BeginFrame();

	Renderer::EndFrame();
}
