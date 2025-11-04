#include "Renderer/SceneRenderer.h"
#include "Renderer/Renderer.h"

#include "Renderer/VertexInput.h"
#include "Renderer/Shader.h"
#include "Renderer/UniformBuffer.h"
#include "Renderer/Camera.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/Texture.h"

#include "Scene/Scene.h"

#include "Component/SpriteRendererComponent.h"
#include "Component/TransformComponent.h"
#include "Component/CameraComponent.h"

#include "Core/ServiceLocator.h"

#include "Asset/AssetLibrary.h"
#include "Asset/ShaderAsset.h"
#include "Asset/TextureAsset.h"
#include "Asset/SpriteAtlasAsset.h"

//temp
#include "Input/Input.h"
#include "Core/Time.h"
#include "Core/Application.h"
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>

using namespace Boon;

namespace Boon
{
	static const uint32_t s_MaxQuads = 20000;
	static const uint32_t s_MaxVertices = s_MaxQuads * 4;
	static const uint32_t s_MaxIndices = s_MaxQuads * 6;
	static const uint32_t s_MaxTextureSlots = 32;
}

Boon::SceneRenderer::SceneRenderer(Scene* pScene, bool isSwapchainTarget)
	: SceneRenderer{pScene, 1080, 720, isSwapchainTarget} {}

Boon::SceneRenderer::SceneRenderer(Scene* pScene, int viewportWidth, int viewportHeight, bool isSwapchainTarget)
	: m_pScene{pScene}, m_ViewportWidth{viewportWidth}, m_ViewportHeight{viewportHeight}
{
	m_QuadVertexPositions[0] = { -0.5f, -0.5f, 0.f, 1.f, };
	m_QuadVertexPositions[1] = {  0.5f, -0.5f, 0.f, 1.f, };
	m_QuadVertexPositions[2] = {  0.5f,  0.5f, 0.f, 1.f, };
	m_QuadVertexPositions[3] = { -0.5f,  0.5f, 0.f, 1.f  };

	m_QuadVertexBufferBase = new QuadVertex[s_MaxVertices];

	uint32_t* quadIndices = new uint32_t[s_MaxIndices];

	uint32_t offset = 0;
	for (uint32_t i = 0; i < s_MaxIndices; i += 6)
	{
		quadIndices[i + 0] = offset + 0;
		quadIndices[i + 1] = offset + 1;
		quadIndices[i + 2] = offset + 2;

		quadIndices[i + 3] = offset + 2;
		quadIndices[i + 4] = offset + 3;
		quadIndices[i + 5] = offset + 0;

		offset += 4;
	}

	m_pQuadVertexInput = VertexInput::Create();

	m_pQuadVertexBuffer = VertexBuffer::Create(s_MaxVertices * sizeof(QuadVertex));
	m_pQuadVertexBuffer->SetLayout({
		{ ShaderDataType::Float3, "a_Position"	   },
		{ ShaderDataType::Float4, "a_Color"		   },
		{ ShaderDataType::Float2, "a_TexCoord"     },
		{ ShaderDataType::Float,  "a_TexIndex"     },
		{ ShaderDataType::Float,  "a_TilingFactor" },
		{ ShaderDataType::Int,	  "a_ID"		   }
	});
	m_pQuadVertexInput->AddVertexBuffer(m_pQuadVertexBuffer);
	m_pQuadVertexInput->SetIndexBuffer(IndexBuffer::Create(quadIndices, s_MaxIndices));
	delete[] quadIndices;

	AssetLibrary& assets{ ServiceLocator::Get<AssetLibrary>()};
	AssetHandle quadHandle = assets.Load<ShaderAssetLoader>("shaders/Quad.glsl");
	m_pShader = assets.GetAsset<ShaderAsset>(quadHandle);

	m_pCameraUniformBuffer = UniformBuffer::Create<UBData::Camera>(0);

	FramebufferDescriptor fbDesc;
	fbDesc.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
	fbDesc.Width = viewportWidth;
	fbDesc.Height = viewportHeight;
	fbDesc.SwapChainTarget = isSwapchainTarget;
	m_pOutputFB = Framebuffer::Create(fbDesc);

	m_TextureSlots[0] = Texture2D::Create(TextureDescriptor());
	uint32_t whiteTextureData = 0xffffffff;
	m_TextureSlots[0]->SetData(&whiteTextureData, sizeof(uint32_t));
}
Boon::SceneRenderer::~SceneRenderer()
{
	delete[] m_QuadVertexBufferBase;
}

void Boon::SceneRenderer::Render(Camera* camera, TransformComponent* cameraTransform)
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

	m_CameraData.ViewProjection = camera->GetProjection() * glm::inverse(cameraTransform->GetWorld());
	m_pCameraUniformBuffer->SetValue(m_CameraData);

	m_pOutputFB->Bind();

	Renderer::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	Renderer::Clear();

	// Clear our entity ID attachment to -1
	m_pOutputFB->ClearAttachment(1, -1);

	StartBatch();

	AssetLibrary& assetLib{ ServiceLocator::Get<AssetLibrary>() };

	auto group = m_pScene->GetRegistry().group<TransformComponent, SpriteRendererComponent>();
	for (auto gameObject : group)
	{
		auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(gameObject);

		if (assetLib.IsValidAsset(sprite.SpriteAtlasHandle))
		{
			auto atlas = assetLib.GetAsset<SpriteAtlasAsset>(sprite.SpriteAtlasHandle);
			const SpriteUV& spriteUv = atlas->GetSpriteUV(sprite.Sprite);
			RenderQuad(transform.GetWorld(), atlas->GetTexture(), sprite.Tiling, sprite.Color, (int)gameObject, spriteUv.UV, spriteUv.Size);
		}
		else
			RenderQuad(transform.GetWorld(), sprite.Color, (int)gameObject);
	}

	Flush();

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

void Boon::SceneRenderer::RenderQuad(const glm::mat4& transform, const glm::vec4& color, int gameObjectHandle)
{
	constexpr size_t quadVertexCount = 4;
	const float textureIndex = 0.0f;
	constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
	const float tilingFactor = 1.0f;

	if (m_QuadIndexCount >= s_MaxIndices)
		NextBatch();

	for (size_t i = 0; i < quadVertexCount; i++)
	{
		m_QuadVertexBufferPtr->Position = transform * m_QuadVertexPositions[i];
		m_QuadVertexBufferPtr->Color = color;
		m_QuadVertexBufferPtr->TexCoord = textureCoords[i];
		m_QuadVertexBufferPtr->TexIndex = textureIndex;
		m_QuadVertexBufferPtr->TilingFactor = tilingFactor;
		m_QuadVertexBufferPtr->GameObjectID = gameObjectHandle;
		m_QuadVertexBufferPtr++;
	}

	m_QuadIndexCount += 6;
}

void Boon::SceneRenderer::RenderQuad(const glm::mat4& transform, const std::shared_ptr<Texture2D>& texture, float tilingFactor, const glm::vec4& color, int gameObjectHandle)
{
	RenderQuad(transform, texture, tilingFactor, color, gameObjectHandle, { 0.f, 0.f }, { 1.f, 1.f });
}

void Boon::SceneRenderer::RenderQuad(const glm::mat4& transform, const std::shared_ptr<Texture2D>& texture, float tilingFactor, 
	const glm::vec4& color, int gameObjectHandle, const glm::vec2& spriteTexCoord, const glm::vec2& spriteTexSize)
{
	constexpr size_t quadVertexCount = 4;

	glm::vec2 texCoords[quadVertexCount] = {
		{ spriteTexCoord.x,                    spriteTexCoord.y },                     // bottom-left
		{ spriteTexCoord.x + spriteTexSize.x,  spriteTexCoord.y },                     // bottom-right
		{ spriteTexCoord.x + spriteTexSize.x,  spriteTexCoord.y + spriteTexSize.y },   // top-right
		{ spriteTexCoord.x,                    spriteTexCoord.y + spriteTexSize.y }    // top-left
	};

	if (m_QuadIndexCount >= s_MaxIndices)
		NextBatch();

	float textureIndex = 0.0f;
	bool found = false;
	for (uint32_t i = 1; i < m_TextureSlotIndex; i++)
	{
		if (m_TextureSlots[i]->GetRendererID() == texture->GetRendererID())
		{
			textureIndex = (float)i;
			found = true;
			break;
		}
	}
	if (!found)
	{
		if (m_TextureSlotIndex >= s_MaxTextureSlots)
			NextBatch();

		textureIndex = (float)m_TextureSlotIndex;
		m_TextureSlots[m_TextureSlotIndex] = texture;
		m_TextureSlotIndex++;
	}

	for (size_t i = 0; i < quadVertexCount; i++)
	{
		m_QuadVertexBufferPtr->Position = transform * m_QuadVertexPositions[i];
		m_QuadVertexBufferPtr->Color = color;
		m_QuadVertexBufferPtr->TexCoord = texCoords[i];
		m_QuadVertexBufferPtr->TexIndex = textureIndex;
		m_QuadVertexBufferPtr->TilingFactor = tilingFactor;
		m_QuadVertexBufferPtr->GameObjectID = gameObjectHandle;
		m_QuadVertexBufferPtr++;
	}

	m_QuadIndexCount += 6;
}

void Boon::SceneRenderer::StartBatch()
{
	m_QuadIndexCount = 0;
	m_QuadVertexBufferPtr = m_QuadVertexBufferBase;
	m_TextureSlotIndex = 1;
}

void Boon::SceneRenderer::NextBatch()
{
	Flush();
	StartBatch();
}

void Boon::SceneRenderer::Flush()
{
	if (m_QuadIndexCount)
	{
		uint32_t dataSize = (uint32_t)((uint8_t*)m_QuadVertexBufferPtr - (uint8_t*)m_QuadVertexBufferBase);
		m_pQuadVertexBuffer->SetData(m_QuadVertexBufferBase, dataSize);

		for (uint32_t i = 0; i < m_TextureSlotIndex; i++)
			m_TextureSlots[i]->Bind(i);

		m_pShader->Bind();
		Renderer::DrawIndexed(m_pQuadVertexInput, m_QuadIndexCount);
	}
}

void Boon::SceneRenderer::SetViewport(int width, int height)
{
	m_ViewportWidth = width;
	m_ViewportHeight = height;
	m_ViewportDirty = true;
}