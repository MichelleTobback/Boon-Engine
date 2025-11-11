#include "DebugRenderer/DebugRenderer.h"

#include <Renderer/Renderer2D.h>
#include <Renderer/Framebuffer.h>
#include <Renderer/Camera.h>

#include <Scene/Scene.h>

#include <Component/TransformComponent.h>
#include <Component/BoxCollider2D.h>

using namespace BoonEditor;

BoonEditor::DebugRenderer::DebugRenderer(Scene* pScene, Framebuffer* pFramebuffer)
	: m_pScene{pScene}, m_pFramebuffer{pFramebuffer}, m_pRenderer2D{std::make_unique<Renderer2D>()}
{
}

BoonEditor::DebugRenderer::~DebugRenderer()
{
}

void BoonEditor::DebugRenderer::Render(EditorViewportSettings settings, Camera* camera, TransformComponent* cameraTransform)
{
	if (settings.DebugRenderLayers & DebugRenderLayer::Disabled)
		return;

	Begin();

	if (settings.DebugRenderLayers & DebugRenderLayer::Collision)
	{
		auto group = m_pScene->GetAllGameObjectsWith<TransformComponent, BoxCollider2D>();
		for (auto gameObject : group)
		{
			auto [transform, collider] = group.get<TransformComponent, BoxCollider2D>(gameObject);
			{
				m_pRenderer2D->SubmitRect(transform.GetWorld(), collider.Size, glm::vec4(1.f, 1.f, 1.f, 1.f));
			}
		}
	}

	End();
}

void BoonEditor::DebugRenderer::Begin()
{
	m_pFramebuffer->Bind();
	m_pRenderer2D->Begin();
}

void BoonEditor::DebugRenderer::End()
{
	m_pRenderer2D->End();
	m_pFramebuffer->Unbind();
}
