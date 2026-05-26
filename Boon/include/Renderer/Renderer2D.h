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

	private:
		void FlushRenderQueue(RenderContext& ctx);

		RenderQueue2D m_RenderQueue;

		glm::vec4 m_QuadVertexPositions[4];

		RenderBatch m_QuadBatch{};
		RenderBatch m_LineBatch{};

		static constexpr uint32_t s_MaxTextureSlots = 32;
		std::array<std::shared_ptr<Texture2D>, s_MaxTextureSlots> m_TextureSlots;
		uint32_t m_TextureSlotIndex = 1; // 0 = white texture

		std::shared_ptr<Material> m_pQuadMaterialTemplate;
	};
}