#pragma once
#include "Renderer/RenderBatch.h"
#include "Renderer/Texture.h"
#include "Renderer/RenderQueue2D.h"

#include <array>
#include <memory>
#include <glm/glm.hpp>

namespace Boon
{
	struct RenderContext;
	struct Renderer2DCreateInfo
	{
		std::shared_ptr<Shader> pSpriteShader;
		std::shared_ptr<Shader> pLineShader;
	};

	struct QuadMaterialData
	{
		glm::vec4 Color{ 1.0f };
		float TilingFactor = 1.0f;

		glm::vec3 Padding{};
	};

	struct QuadBatchKey
	{
		std::shared_ptr<Pipeline> Pipeline = nullptr;

		bool operator==(const QuadBatchKey& other) const
		{
			return Pipeline.get() == other.Pipeline.get();
		}
	};

	struct QuadBatchKeyHasher
	{
		size_t operator()(const QuadBatchKey& key) const
		{
			return std::hash<void*>{}(key.Pipeline.get());
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

		void SubmitQuad(const QuadRenderItem2D& item);
		void SubmitQuad(const glm::mat4& transform, const std::shared_ptr<Material>& material, int gameObjectHandle);
		void SubmitQuad(const glm::mat4& transform, const std::shared_ptr<Material>& material, int gameObjectHandle, const glm::vec2& spriteTexCoord, const glm::vec2& spriteTexSize);
		void SubmitQuad(
			const glm::mat4& transform, 
			const std::shared_ptr<Texture2D>& texture, 
			const glm::vec4& color, 
			const float tiling,
			int gameObjectHandle, 
			const glm::vec2& spriteTexCoord, 
			const glm::vec2& spriteTexSize);

		void SubmitLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color);
		void SubmitRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		void SubmitRect(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		void SubmitRect(const glm::mat4& transform, const glm::vec2& size, const glm::vec4& color);
		void SubmitPolygon(const std::vector<glm::vec3>& positions, const glm::vec4& color);

		void SubmitGeometry(const GeometryRenderItem2D& item);

		static bool IsCompatibleMaterial(const std::shared_ptr<Material>& material, const std::shared_ptr<Pipeline>& pipeline)
		{
			if (!material)
				return true;

			return material->GetPipeline() == pipeline;
		}

	private:
		static constexpr uint32_t s_MaxTextureSlots = 32;
		struct QuadBatchState
		{
			RenderBatch Batch;

			std::array<std::shared_ptr<Texture2D>, s_MaxTextureSlots> TextureSlots;
			uint32_t TextureSlotIndex = 1;
		};

		void FlushRenderQueue(RenderContext& ctx);
		QuadBatchState& GetOrCreateQuadBatch(const std::shared_ptr<Pipeline>& pipeline);

		RenderQueue2D m_RenderQueue;

		uint32_t m_TextureSlotIndex = 1; // 0 = white texture

		glm::vec4 m_QuadVertexPositions[4];
		std::shared_ptr<IndexBuffer> m_QuadIndexBuffer;
		std::shared_ptr<Pipeline> m_pQuadPipeline;

		std::unordered_map<QuadBatchKey, QuadBatchState, QuadBatchKeyHasher> m_QuadBatches;

		std::shared_ptr<Texture2D> m_pWhiteTexture;

		RenderBatch m_LineBatch{};
	};
}