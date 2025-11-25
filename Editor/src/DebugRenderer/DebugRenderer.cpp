#include "DebugRenderer/DebugRenderer.h"
#include "BoonDebug/DebugRenderer.h"

#include <Renderer/Renderer2D.h>
#include <Renderer/Framebuffer.h>
#include <Renderer/Camera.h>

#include <Scene/Scene.h>

#include <Component/TransformComponent.h>
#include <Component/BoxCollider2D.h>
#include <Component/TilemapRendererComponent.h>

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
	if (settings.DebugRenderLayers & DebugRenderLayer::Default)
	{
		auto group = m_pScene->GetAllGameObjectsWith<TransformComponent, TilemapRendererComponent>();
		for (auto gameObject : group)
		{
			auto [transform, tilemap] = group.get<TransformComponent, TilemapRendererComponent>(gameObject);
			{
				if (!tilemap.tilemap.IsValid())
					continue;

				auto map = tilemap.tilemap->GetInstance();
				auto atlas = map->GetAtlas()->GetInstance();
				auto texture = atlas->GetTexture().Instance();

				float ppu = 32.f;

				// Tiles use the atlas sprite 0 as reference size (or any tileId you prefer)
				//const SpriteFrame& f0 = atlas->GetSpriteFrame(0);
				//float tilePixelW = f0.Size.x * texture->GetWidth();
				//float tilePixelH = f0.Size.y * texture->GetHeight();
				//
				//float tileWorldW = tilePixelW / ppu * map->GetUnitSize();
				//float tileWorldH = tilePixelH / ppu * map->GetUnitSize();

				float tileWorldW = map->GetUnitSize();
				float tileWorldH = map->GetUnitSize();

				glm::mat4 world = transform.GetWorld();

				for (auto chunk : map->GetChunks())
				{
					// Same positioning as BuildChunk
					float worldX = (chunk.ChunkX * map->GetChunkSize()) * tileWorldW + map->GetChunkSize() * tileWorldW * 0.5f;
					float worldY = (chunk.ChunkY * map->GetChunkSize()) * tileWorldH + map->GetChunkSize() * tileWorldH * 0.5f;

					// Centered quad (Renderer2D convention)
					glm::vec3 pos = glm::vec3(world * glm::vec4(worldX, worldY, 0, 1));
					glm::vec2 size = glm::vec2(map->GetChunkSize() * tileWorldW, map->GetChunkSize() * tileWorldH);

					m_pRenderer2D->SubmitRect(pos, size, glm::vec4(1, 1, 1, 1));
				}
			}
		}
	}

	Boon::DebugRenderer::Get().FlushQueue(*m_pRenderer2D);

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
