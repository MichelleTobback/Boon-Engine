#pragma once

#include "Renderer/RenderBatch.h"
#include "Renderer/Texture.h"
#include "Renderer/RenderQueue2D.h"
#include "Renderer/QuadMaterialData.h"
#include "Renderer/Material.h"

#include <array>
#include <memory>
#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>

namespace Boon
{
	struct RenderContext;

	struct Renderer2DCreateInfo
	{
		std::shared_ptr<Material> pSpriteMaterial;
		std::shared_ptr<Material> pLineMaterial;
	};

	struct QuadBatchKey
	{
		std::shared_ptr<Material> Material = nullptr;

		bool operator==(const QuadBatchKey& other) const
		{
			return Material.get() == other.Material.get();
		}
	};

	struct QuadBatchKeyHasher
	{
		size_t operator()(const QuadBatchKey& key) const
		{
			return std::hash<void*>{}(key.Material.get());
		}
	};

	class IndexBuffer;

	class Renderer2D final
	{
	public:
		Renderer2D(const Renderer2DCreateInfo& desc);
		~Renderer2D();

		void Begin(RenderContext& ctx);
		void End(RenderContext& ctx);

		void FlushRenderQueue(RenderContext& ctx);

		void SubmitQuad(const QuadRenderItem2D& item);

		void SubmitQuad(
			const glm::mat4& transform,
			const std::shared_ptr<Material>& material,
			int gameObjectHandle);

		void SubmitQuad(
			const glm::mat4& transform,
			const std::shared_ptr<Material>& material,
			int gameObjectHandle,
			const glm::vec2& spriteTexCoord,
			const glm::vec2& spriteTexSize);

		void SubmitQuad(
			const glm::mat4& transform,
			const std::shared_ptr<Texture2D>& texture,
			const glm::vec4& color,
			float tiling,
			int gameObjectHandle,
			const glm::vec2& spriteTexCoord,
			const glm::vec2& spriteTexSize);

		void SubmitLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color);

		void SubmitRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		void SubmitRect(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		void SubmitRect(const glm::mat4& transform, const glm::vec2& size, const glm::vec4& color);

		void SubmitPolygon(const std::vector<glm::vec3>& positions, const glm::vec4& color);

	private:
		static constexpr uint32_t s_MaxTextureSlots = 32;

		struct QuadBatchState
		{
			RenderBatch Batch;
			std::shared_ptr<Material> Material = nullptr;

			std::array<std::shared_ptr<Texture2D>, s_MaxTextureSlots> TextureSlots{};
			uint32_t TextureSlotIndex = 1;
		};

		QuadBatchState& GetOrCreateQuadBatch(const std::shared_ptr<Material>& material);

	private:
		RenderQueue2D m_RenderQueue;

		glm::vec4 m_QuadVertexPositions[4]{};

		std::shared_ptr<IndexBuffer> m_QuadIndexBuffer = nullptr;

		std::shared_ptr<Material> m_pDefaultQuadMaterial = nullptr;
		std::shared_ptr<Material> m_pDefaultLineMaterial = nullptr;

		std::unordered_map<QuadBatchKey, QuadBatchState, QuadBatchKeyHasher> m_QuadBatches;

		std::shared_ptr<Texture2D> m_pWhiteTexture = nullptr;

		RenderBatch m_LineBatch{};
	};
}