#include <Renderer/Passes/RenderPass2D.h>
#include <Renderer/Shader.h>
#include <Renderer/Renderer2D.h>
#include <Renderer/Material.h>

#include <Scene/Scene.h>

#include <Component/TilemapRendererComponent.h>
#include <Component/SpriteRendererComponent.h>
#include <Component/TransformComponent.h>
#include <Component/TextureRendererComponent.h>

#include <Asset/AssetLibrary.h>
#include <Asset/ShaderAsset.h>
#include <Asset/TextureAsset.h>
#include <Asset/SpriteAtlasAsset.h>

void Boon::SpriteRenderPass::Execute(RenderContext& context)
{
	auto group = context.Scene.GetAllGameObjectsWith<TransformComponent, SpriteRendererComponent>();

	for (auto gameObject : group)
	{
		auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(gameObject);

		if (!sprite.SpriteAtlasHandle.IsValid())
			continue;

		auto atlas = sprite.SpriteAtlasHandle.Instance();

		if (!atlas->GetTexture().IsValid())
			continue;

		auto texture = atlas->GetTexture().Instance();
		if (!texture)
			continue;

		const SpriteFrame& spriteUv = atlas->GetSpriteFrame(sprite.Sprite);

		QuadRenderItem2D item{};
		item.Transform = transform.GetWorld();
		item.Color = sprite.Color;
		item.TilingFactor = sprite.Tiling;
		item.Texture = texture;
		item.UV0 = spriteUv.UV;
		item.UV1 = spriteUv.UV + spriteUv.Size;
		item.EntityID = static_cast<int>((GameObjectID)gameObject);

		context.Renderer2D.SubmitQuad(item);
	}
}

void Boon::TextureRenderPass::Execute(RenderContext& context)
{
	auto group = context.Scene.GetAllGameObjectsWith<TransformComponent, TextureRendererComponent>();

	for (auto gameObject : group)
	{
		auto [transform, tc] = group.get<TransformComponent, TextureRendererComponent>(gameObject);

		if (!tc.Texture.IsValid())
			continue;

		auto texture = tc.Texture.Instance();
		if (!texture)
			continue;

		QuadRenderItem2D item{};
		item.Transform = transform.GetWorld();
		item.Color = tc.Color;
		item.TilingFactor = tc.Tiling;
		item.Texture = texture;
		item.UV0 = { 0.0f, 0.0f };
		item.UV1 = { 1.0f, 1.0f };
		item.EntityID = static_cast<int>((GameObjectID)gameObject);

		context.Renderer2D.SubmitQuad(item);
	}
}

void Boon::TilemapRenderPass::Execute(RenderContext& context)
{
	auto group = context.Scene.GetAllGameObjectsWith<TransformComponent, TilemapRendererComponent>();

	for (auto gameObject : group)
	{
		auto [transform, tilemap] = group.get<TransformComponent, TilemapRendererComponent>(gameObject);

		if (!tilemap.tilemap.IsValid())
			continue;

		auto tilemapAsset = tilemap.tilemap.Instance();

		if (!tilemapAsset->GetAtlas().IsValid())
			continue;

		auto atlas = tilemapAsset->GetAtlas().Instance();

		if (!atlas->GetTexture().IsValid())
			continue;

		auto texture = atlas->GetTexture().Instance();
		if (!texture)
			continue;

		auto baseMaterial = tilemap.MaterialOverride;

		if (!baseMaterial)
			baseMaterial = m_pMaterial;

		if (!baseMaterial)
			continue;

		if (!tilemap.MaterialInstance)
			tilemap.MaterialInstance = baseMaterial->CreateInstance();

		tilemap.MaterialInstance->SetTexture("u_Texture", texture, 0);

		tilemapAsset->RebuildDirtyChunks();

		for (auto& chunk : tilemapAsset->GetChunks())
		{
			GeometryRenderItem2D item{};
			item.Transform = transform.GetWorld();
			item.VertexInput = chunk.VertexInput;
			item.Material = tilemap.MaterialInstance;
			item.EntityID = static_cast<int>((GameObjectID)gameObject);

			context.Renderer2D.SubmitGeometry(item);
		}
	}
}
