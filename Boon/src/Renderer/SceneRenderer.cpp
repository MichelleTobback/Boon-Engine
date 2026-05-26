#include "Renderer/SceneRenderer.h"
#include "Renderer/Renderer.h"
#include "Renderer/VertexInput.h"
#include "Renderer/Shader.h"
#include "Renderer/UniformBuffer.h"
#include "Renderer/Camera.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/Texture.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/RenderPass.h"
#include "Renderer/Passes/RenderPass2D.h"
#include "Renderer/Material.h"

#include "Scene/Scene.h"

#include <Component/TransformComponent.h>
#include <Component/CameraComponent.h>

#include "Asset/AssetLibrary.h"
#include "Asset/ShaderAsset.h"
#include "Asset/TextureAsset.h"
#include "Asset/SpriteAtlasAsset.h"

#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>

using namespace Boon;

Boon::SceneRenderer::SceneRenderer(const SceneRendererCreateInfo& desc)
	: m_pScene{desc.pScene}, m_ViewportWidth{static_cast<int>(desc.Width)}, m_ViewportHeight{ static_cast<int>(desc.Height) }
{
	
	//scene
	m_pCameraUniformBuffer = UniformBuffer::Create<UBData::Camera>(0);
	m_pObjectUniformBuffer = UniformBuffer::Create<UBData::Object>(1);

	FramebufferDescriptor fbDesc;
	fbDesc.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
	fbDesc.Width = desc.Width;
	fbDesc.Height = desc.Height;
	fbDesc.SwapChainTarget = desc.bIsSwapchainTarget;
	m_pOutputFB = Framebuffer::Create(fbDesc);

	Renderer2DCreateInfo renderer2dDesc{};
	AssetLibrary& assetLib = *desc.AssetLib;
	renderer2dDesc.pSpriteShader = assetLib.Load<ShaderAsset>("shaders/Quad.glsl")->GetInstance();
	renderer2dDesc.pLineShader = assetLib.Load<ShaderAsset>("shaders/Line.glsl")->GetInstance();

	PipelineDescriptor tilemapDesc{};
	tilemapDesc.Shader = assetLib.Load<ShaderAsset>("shaders/Tilemap.glsl")->GetInstance();
	std::shared_ptr<Pipeline> tilemapPipeline = std::make_shared<Pipeline>(tilemapDesc);
	m_pDefaultTilemapMaterial = std::make_shared<Material>(tilemapPipeline);

	PipelineDescriptor quadDesc{};
	quadDesc.Shader = assetLib.Load<ShaderAsset>("shaders/Quad.glsl")->GetInstance();
	std::shared_ptr<Pipeline> quadPipeline = std::make_shared<Pipeline>(quadDesc);

	m_pDefaultQuadMaterial = std::make_shared<Material>(quadPipeline, sizeof(QuadMaterialData));
	QuadMaterialData quadData{};
	quadData.Color = glm::vec4(1.0f);
	quadData.TilingFactor = 1.0f;
	m_pDefaultQuadMaterial->SetValue(0, quadData);

	m_pRenderer2D = std::make_unique<Renderer2D>(renderer2dDesc);

	m_Passes.push_back(std::make_unique<SpriteRenderPass>(m_pDefaultQuadMaterial));
	m_Passes.push_back(std::make_unique<TextureRenderPass>(m_pDefaultQuadMaterial));
	m_Passes.push_back(std::make_unique<TilemapRenderPass>(m_pDefaultTilemapMaterial));
}
Boon::SceneRenderer::~SceneRenderer()
{
	
}

void Boon::SceneRenderer::Render(Camera* camera, TransformComponent* cameraTransform)
{
	RenderContext context{ *m_pScene, *m_pRenderer2D, *m_pObjectUniformBuffer };

	BeginScene(context, camera, cameraTransform);

	for (auto& pass : m_Passes)
		pass->Execute(context);

	EndScene(context);
}

void Boon::SceneRenderer::BeginScene(RenderContext& ctx, Camera* camera, TransformComponent* cameraTransform)
{
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

	Renderer::SetClearColor({ 0.f, 0.f, 0.f, 1 });
	Renderer::Clear();

	// Clear our entity ID attachment to -1
	m_pOutputFB->ClearAttachment(1, -1);

	m_pRenderer2D->Begin(ctx);
}

void Boon::SceneRenderer::EndScene(RenderContext& ctx)
{
	m_pRenderer2D->End(ctx);

	m_pOutputFB->Unbind();

	if (m_ViewportDirty)
	{
		m_pOutputFB->Resize(m_ViewportWidth, m_ViewportHeight);

		auto view = m_pScene->GetRegistry().view<CameraComponent>();
		for (auto entity : view)
		{
			CameraComponent& cam = view.get<CameraComponent>(entity);

			cam.Camera.SetAspectRatio((float)m_ViewportWidth, (float)m_ViewportHeight);
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

Renderer2D* Boon::SceneRenderer::GetRenderer2D() const { return m_pRenderer2D.get(); }