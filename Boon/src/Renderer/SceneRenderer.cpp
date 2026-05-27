#include "Renderer/SceneRenderer.h"
#include "Renderer/Renderer.h"
#include "Renderer/VertexInput.h"
#include "Renderer/Shader.h"
#include "Renderer/UniformBuffer.h"
#include "Renderer/Camera.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/Texture.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/Renderer3D.h"
#include "Renderer/RenderPass.h"
#include "Renderer/Passes/RenderPass2D.h"
#include "Renderer/Material.h"
#include "Renderer/MaterialFactory.h"

#include "Scene/Scene.h"

#include <Component/TransformComponent.h>
#include <Component/CameraComponent.h>

#include "Asset/AssetLibrary.h"
#include "Asset/ShaderAsset.h"
#include "Asset/TextureAsset.h"
#include "Asset/SpriteAtlasAsset.h"

#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>

#include <algorithm>

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

	AssetLibrary& assetLib = *desc.AssetLib;

	auto quadShader = assetLib.Load<ShaderAsset>("shaders/Quad.glsl");
	auto tilemapShader = assetLib.Load<ShaderAsset>("shaders/Tilemap.glsl");
	auto lineShader = assetLib.Load<ShaderAsset>("shaders/Line.glsl");

	Renderer2DCreateInfo renderer2dDesc{};

	if (quadShader.IsValid())
		m_pDefaultQuadMaterial = MaterialFactory::CreateFromShaderAsset(*quadShader.Get());

	renderer2dDesc.pSpriteMaterial = m_pDefaultQuadMaterial;

	if (tilemapShader.IsValid())
		m_pDefaultTilemapMaterial = MaterialFactory::CreateFromShaderAsset(*tilemapShader.Get());

	if (lineShader.IsValid())
		renderer2dDesc.pLineMaterial = MaterialFactory::CreateFromShaderAsset(*lineShader.Get(), BlendMode::Alpha, DepthMode::ReadWrite, CullMode::None, PrimitiveType::Lines);

	m_pRenderer2D = std::make_unique<Renderer2D>(renderer2dDesc);
	m_pRenderer3D = std::make_unique<Renderer3D>();

	AddPass(std::make_unique<SpriteRenderPass>(m_pDefaultQuadMaterial));
	AddPass(std::make_unique<TextureRenderPass>(m_pDefaultQuadMaterial));
	AddPass(std::make_unique<TilemapRenderPass>(m_pDefaultTilemapMaterial));
}
Boon::SceneRenderer::~SceneRenderer()
{
	
}

void Boon::SceneRenderer::Render(Camera* camera, TransformComponent* cameraTransform)
{
	RenderContext context{ *m_pScene, *m_pRenderer2D, *m_pRenderer3D, *m_pObjectUniformBuffer };

	if (m_bPassesDirty)
	{
		SortPasses();
		m_bPassesDirty = false;
	}

	BeginScene(context, camera, cameraTransform);

	RenderPhaseID currentPhase = m_Passes.front()->GetPhase();

	for (auto& pass : m_Passes)
	{
		if (pass->GetPhase() != currentPhase)
		{
			m_pRenderer2D->FlushRenderQueue(context);
			m_pRenderer3D->End(context);
			m_pRenderer3D->Begin(context);

			currentPhase = pass->GetPhase();
		}

		context.CurrentPhase = pass->GetPhase();
		pass->Execute(context);
	}

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
	m_pRenderer3D->Begin(ctx);
}

void Boon::SceneRenderer::EndScene(RenderContext& ctx)
{
	m_pRenderer2D->End(ctx);
	m_pRenderer3D->End(ctx);

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

void Boon::SceneRenderer::AddPass(std::unique_ptr<RenderPass> pass)
{
	if (!pass)
		return;

	m_Passes.push_back(std::move(pass));
	m_bPassesDirty = true;
}

void Boon::SceneRenderer::SortPasses()
{
	std::sort(m_Passes.begin(), m_Passes.end(),
		[](const std::unique_ptr<RenderPass>& a, const std::unique_ptr<RenderPass>& b)
		{
			if (a->GetPhase() != b->GetPhase())
				return a->GetPhase() < b->GetPhase();

			return a->GetOrder() < b->GetOrder();
		});
}