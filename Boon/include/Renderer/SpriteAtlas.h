#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>
#include <queue>

namespace Boon
{
	struct SpriteUV
	{
		glm::vec2 UV;
		glm::vec2 Size;
	};

	class Texture2D;
	class SpriteAtlas
	{
	public:
		SpriteAtlas() = default;
		virtual ~SpriteAtlas() = default;

		SpriteAtlas(const SpriteAtlas& other) = delete;
		SpriteAtlas(SpriteAtlas&& other) = delete;
		SpriteAtlas& operator=(const SpriteAtlas& other) = delete;
		SpriteAtlas& operator=(SpriteAtlas&& other) = delete;

		inline void SetTexture(const std::shared_ptr<Texture2D>& pTexture) { m_pTexture = pTexture; }
		inline const std::shared_ptr<Texture2D>& GetTexture() const { return m_pTexture; }

		inline void AddSpriteUV(const SpriteUV& uv)
		{
			int id = (int)m_Sprites.size();
			if (!m_FreeIds.empty())
			{
				id = m_FreeIds.front();
				m_FreeIds.pop();
			}
			m_Sprites[id] = uv;
		}

		inline void RemoveSpriteUV(int index) { m_Sprites.erase(index); m_FreeIds.push(index); }
		inline void SetSpriteUV(const SpriteUV& uv, int index = 0) { m_Sprites[index] = uv; }
		inline const SpriteUV& GetSpriteUV(int index) const { return m_Sprites.at(index); }

	private:
		std::shared_ptr<Texture2D> m_pTexture;
		std::unordered_map<int, SpriteUV> m_Sprites;
		std::queue<int> m_FreeIds;
	};
}