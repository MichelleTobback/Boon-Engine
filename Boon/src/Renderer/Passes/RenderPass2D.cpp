#include <Renderer/Passes/RenderPass2D.h>
#include <Renderer/Shader.h>
#include <Renderer/Renderer2D.h>
#include <Renderer/Renderer3D.h>
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
#include <Asset/MaterialAsset.h>

namespace
{
	template<typename Component>
	std::shared_ptr<Boon::Material> ResolveMaterialInstance(
		Component& component,
		const std::shared_ptr<Boon::Material>& fallbackMaterial)
	{
		Boon::MaterialAsset* materialAsset = nullptr;
		Boon::AssetHandle sourceHandle = 0;
		uint32_t sourceVersion = 0;

		if (component.MaterialOverride.IsValid())
		{
			materialAsset = component.MaterialOverride.Get();
			sourceHandle = component.MaterialOverride.Handle();

			if (materialAsset)
				sourceVersion = materialAsset->GetVersion();
		}

		if (materialAsset)
		{
			if (!component.MaterialInstance ||
				component.MaterialInstanceSource != sourceHandle ||
				component.MaterialInstanceVersion != sourceVersion)
			{
				component.MaterialInstance = materialAsset->CreateMaterial();
				component.MaterialInstanceSource = sourceHandle;
				component.MaterialInstanceVersion = sourceVersion;
			}

			return component.MaterialInstance;
		}

		if (!component.MaterialInstance ||
			component.MaterialInstanceSource.IsValid())
		{
			component.MaterialInstance = fallbackMaterial
				? fallbackMaterial->CreateInstance()
				: nullptr;

			component.MaterialInstanceSource = 0;
			component.MaterialInstanceVersion = 0;
		}

		return component.MaterialInstance;
	}
}

void Boon::SpriteRenderPass::Execute(RenderContext& context)
{
	auto group = context.Scene.GetAllGameObjectsWith<TransformComponent, SpriteRendererComponent>();

	for (auto gameObject : group)
	{
		auto [transform, sprite] =
			group.get<TransformComponent, SpriteRendererComponent>(gameObject);

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
		item.UV0 = spriteUv.UV;
		item.UV1 = spriteUv.UV + spriteUv.Size;
		item.Texture = texture;
		item.Color = sprite.Color;
		item.TilingFactor = sprite.Tiling;
		item.EntityID = static_cast<int>((GameObjectID)gameObject);

		if (sprite.MaterialOverride.IsValid())
		{
			std::shared_ptr<Material> material =
				ResolveMaterialInstance(sprite, m_pMaterial);

			if (!material)
				continue;

			material->SetTexture("u_Texture", texture, 0);
			item.MaterialOverride = material;
		}

		context.Renderer2D.SubmitQuad(item);
	}
}

void Boon::TextureRenderPass::Execute(RenderContext& context)
{
	auto group = context.Scene.GetAllGameObjectsWith<TransformComponent, TextureRendererComponent>();

	for (auto gameObject : group)
	{
		auto [transform, tc] =
			group.get<TransformComponent, TextureRendererComponent>(gameObject);

		if (!tc.Texture.IsValid())
			continue;

		auto texture = tc.Texture.Instance();
		if (!texture)
			continue;

		QuadRenderItem2D item{};
		item.Transform = transform.GetWorld();
		item.UV0 = { 0.0f, 0.0f };
		item.UV1 = { 1.0f, 1.0f };
		item.Texture = texture;
		item.Color = tc.Color;
		item.TilingFactor = tc.Tiling;
		item.EntityID = static_cast<int>((GameObjectID)gameObject);

		if (tc.MaterialOverride.IsValid())
		{
			std::shared_ptr<Material> material =
				ResolveMaterialInstance(tc, m_pMaterial);

			if (!material)
				continue;

			material->SetTexture("u_Texture", texture, 0);
			item.MaterialOverride = material;
		}

		context.Renderer2D.SubmitQuad(item);
	}
}

void Boon::TilemapRenderPass::Execute(RenderContext& context)
{
	auto group = context.Scene.GetAllGameObjectsWith<TransformComponent, TilemapRendererComponent>();

	for (auto gameObject : group)
	{
		auto [transform, tilemap] =
			group.get<TransformComponent, TilemapRendererComponent>(gameObject);

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

		std::shared_ptr<Material> material =
			ResolveMaterialInstance(tilemap, m_pMaterial);

		if (!material)
			continue;

		material->SetTexture("u_Texture", texture, 0);

		tilemapAsset->RebuildDirtyChunks();

		for (auto& chunk : tilemapAsset->GetChunks())
		{
			GeometryRenderItem3D item{};
			item.Transform = transform.GetWorld();
			item.VertexInput = chunk.VertexInput;
			item.Material = material;
			item.EntityID = static_cast<int>((GameObjectID)gameObject);

			context.Renderer3D.SubmitGeometry(item);
		}
	}
}