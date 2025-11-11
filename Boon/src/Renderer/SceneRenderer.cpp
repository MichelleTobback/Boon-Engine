#include "Renderer/SceneRenderer.h"
#include "Renderer/Renderer.h"

#include "Renderer/VertexInput.h"
#include "Renderer/Shader.h"
#include "Renderer/UniformBuffer.h"
#include "Renderer/Camera.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/Texture.h"
#include "Renderer/Renderer2D.h"

#include "Scene/Scene.h"

#include "Component/SpriteRendererComponent.h"
#include "Component/TransformComponent.h"
#include "Component/CameraComponent.h"

#include "Core/ServiceLocator.h"

#include "Asset/AssetLibrary.h"
#include "Asset/ShaderAsset.h"
#include "Asset/TextureAsset.h"
#include "Asset/SpriteAtlasAsset.h"

#ifdef BOON_WITH_EDITOR
	#include "Component/BoxCollider2D.h"
#endif //BOON_WITH_EDITOR

//temp
#include "Input/Input.h"
#include "Core/Time.h"
#include "Core/Application.h"
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>

using namespace Boon;

Boon::SceneRenderer::SceneRenderer(Scene* pScene, bool isSwapchainTarget)
	: SceneRenderer{pScene, 1080, 720, isSwapchainTarget} {}

Boon::SceneRenderer::SceneRenderer(Scene* pScene, int viewportWidth, int viewportHeight, bool isSwapchainTarget)
	: m_pScene{pScene}, m_ViewportWidth{viewportWidth}, m_ViewportHeight{viewportHeight}
{
	
	//scene
	m_pCameraUniformBuffer = UniformBuffer::Create<UBData::Camera>(0);

	FramebufferDescriptor fbDesc;
	fbDesc.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
	fbDesc.Width = viewportWidth;
	fbDesc.Height = viewportHeight;
	fbDesc.SwapChainTarget = isSwapchainTarget;
	m_pOutputFB = Framebuffer::Create(fbDesc);

	m_pRenderer2D = std::make_unique<Renderer2D>();
}
Boon::SceneRenderer::~SceneRenderer()
{
	
}

void Boon::SceneRenderer::Render(Camera* camera, TransformComponent* cameraTransform)
{
	BeginScene(camera, cameraTransform);

	AssetLibrary& assetLib{ ServiceLocator::Get<AssetLibrary>() };

	{
		auto group = m_pScene->GetAllGameObjectsWith<TransformComponent, SpriteRendererComponent>();
		for (auto gameObject : group)
		{
			auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(gameObject);

			if (assetLib.IsValidAsset(sprite.SpriteAtlasHandle))
			{
				auto atlas = assetLib.GetAsset<SpriteAtlasAsset>(sprite.SpriteAtlasHandle);
				const SpriteFrame& spriteUv = atlas->GetSpriteFrame(sprite.Sprite);
				m_pRenderer2D->SubmitQuad(transform.GetWorld(), atlas->GetTexture(), sprite.Tiling, sprite.Color, (int)gameObject, spriteUv.UV, spriteUv.Size);
			}
			else
				m_pRenderer2D->SubmitQuad(transform.GetWorld(), sprite.Color, (int)gameObject);
		}
	}

#ifdef BOON_WITH_EDITOR
	{
		auto group = m_pScene->GetAllGameObjectsWith<TransformComponent, BoxCollider2D>();
		for (auto gameObject : group)
		{
			auto [transform, collider] = group.get<TransformComponent, BoxCollider2D>(gameObject);
			{
				//if (!collider.DrawDebug)
				//	continue;
				m_pRenderer2D->SubmitRect(transform.GetWorld(), collider.Size, glm::vec4(1.f, 1.f, 1.f, 1.f));
			}
		}
	}
#endif // BOON_WITH_EDITOR

	EndScene();
}

void Boon::SceneRenderer::BeginScene(Camera* camera, TransformComponent* cameraTransform)
{
	Scene* pScene = m_pScene;

	if (!camera || !cameraTransform)
	{
		auto view = m_pScene->GetRegistry().view<TransformComponent, CameraComponent>();
		for (auto entity : view)
		{
			auto [transform, cam] = view.get<TransformComponent, CameraComponent>(entity);

			if (cam.Active)
			{
				camera = &cam.Camera;
				cameraTransform = &transform;
				break;
			}
		}
	}

	if (camera && cameraTransform)
	{
		m_CameraData.ViewProjection = camera->GetProjection() * glm::inverse(cameraTransform->GetWorld());
		m_pCameraUniformBuffer->SetValue(m_CameraData);
	}

	m_pOutputFB->Bind();

	Renderer::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	Renderer::Clear();

	// Clear our entity ID attachment to -1
	m_pOutputFB->ClearAttachment(1, -1);

	m_pRenderer2D->Begin();
}

void Boon::SceneRenderer::EndScene()
{
	m_pRenderer2D->End();

	m_pOutputFB->Unbind();

	if (m_ViewportDirty)
	{
		m_pOutputFB->Resize(m_ViewportWidth, m_ViewportHeight);

		auto view = m_pScene->GetRegistry().view<CameraComponent>();
		for (auto entity : view)
		{
			CameraComponent& cam = view.get<CameraComponent>(entity);

			cam.Camera.SetAspectRatio(m_ViewportWidth, m_ViewportHeight);
		}

		m_ViewportDirty = false;
	}
}

void Boon::SceneRenderer::SetViewport(int width, int height)
{
	m_ViewportWidth = width;
	m_ViewportHeight = height;
	m_ViewportDirty = true;
}