#pragma once
#include "Renderer/RenderBatch.h"
#include "Renderer/Texture.h"

#include <array>
#include <memory>
#include <glm/glm.hpp>

namespace Boon
{
	class Renderer2D final
	{
	public:
		Renderer2D();
		~Renderer2D();

		void Begin();
		void End();

		void SubmitQuad(const glm::mat4& transform, const glm::vec4& color, int gameObjectHandle);
		void SubmitQuad(const glm::mat4& transform, const std::shared_ptr<Texture2D>& texture, float tilingFactor, const glm::vec4& color, int gameObjectHandle);
		void SubmitQuad(const glm::mat4& transform, const std::shared_ptr<Texture2D>& texture, float tilingFactor,
			const glm::vec4& color, int gameObjectHandle, const glm::vec2& spriteTexCoord, const glm::vec2& spriteTexSize);

		void SubmitLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color);
		void SubmitRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		void SubmitRect(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		void SubmitRect(const glm::mat4& transform, const glm::vec2& size, const glm::vec4& color);
		void SubmitPolygon(const std::vector<glm::vec3>& positions, const glm::vec4& color);

	private:
		glm::vec4 m_QuadVertexPositions[4];

		RenderBatch m_QuadBatch{};
		RenderBatch m_LineBatch{};

		static constexpr uint32_t s_MaxTextureSlots = 32;
		std::array<std::shared_ptr<Texture2D>, s_MaxTextureSlots> m_TextureSlots;
		uint32_t m_TextureSlotIndex = 1; // 0 = white texture
	};
}