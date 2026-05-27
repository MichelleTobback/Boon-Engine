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

		RenderPhaseID GetPhase() const override
		{
			return RenderPhases::Opaque;
		}

		int GetOrder() const override
		{
			return 100;
		}

	private:
		std::shared_ptr<Material> m_pMaterial;
	};

	class TextureRenderPass final : public RenderPass
	{
	public:
		TextureRenderPass(const std::shared_ptr<Material>& material)
			: m_pMaterial{ material } {
		}

		void Execute(RenderContext& context) override;

		RenderPhaseID GetPhase() const override
		{
			return RenderPhases::Opaque;
		}

		int GetOrder() const override
		{
			return 200;
		}

	private:

		std::shared_ptr<Material> m_pMaterial;
	};

	class TilemapRenderPass final : public RenderPass
	{
	public:
		virtual ~TilemapRenderPass() = default;

		TilemapRenderPass(const std::shared_ptr<Material>& material)
			: m_pMaterial{ material }{ }

		void Execute(RenderContext& context) override;

		RenderPhaseID GetPhase() const override
		{
			return RenderPhases::Opaque;
		}

		int GetOrder() const override
		{
			return 0;
		}

	private:
		std::shared_ptr<Material> m_pMaterial;
	};
}
