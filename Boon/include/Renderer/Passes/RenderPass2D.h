#pragma once
#include <Renderer/RenderPass.h>
#include <Renderer/Material.h>

#include <unordered_map>
#include <cstdint>
#include <memory>

namespace Boon
{
	class Material;

	class SpriteRenderPass final : public RenderPass
	{
	public:
		SpriteRenderPass(const std::shared_ptr<Material>& material)
			: m_pMaterial{ material } {
		}

		void Execute(RenderContext& context) override;

	private:
		std::shared_ptr<Material> GetOrCreateMaterial(uint64_t gameObjectHandle);

		std::shared_ptr<Material> m_pMaterial;
		std::unordered_map<uint64_t, std::shared_ptr<Material>> m_MaterialCache;
	};

	class TextureRenderPass final : public RenderPass
	{
	public:
		TextureRenderPass(const std::shared_ptr<Material>& material)
			: m_pMaterial{ material } {
		}

		void Execute(RenderContext& context) override;

	private:
		std::shared_ptr<Material> GetOrCreateMaterial(uint64_t gameObjectHandle);

		std::shared_ptr<Material> m_pMaterial;
		std::unordered_map<uint64_t, std::shared_ptr<Material>> m_MaterialCache;
	};

	class TilemapRenderPass final : public RenderPass
	{
	public:
		virtual ~TilemapRenderPass() = default;

		TilemapRenderPass(const std::shared_ptr<Material>& material)
			: m_pMaterial{ material }{ }

		void Execute(RenderContext& context) override;

	private:
		std::shared_ptr<Material> GetOrCreateMaterial(const std::shared_ptr<Texture2D>& texture);

	private:
		std::shared_ptr<Material> m_pMaterial;
		std::unordered_map<uint32_t, std::shared_ptr<Material>> m_MaterialCache;
	};
}
