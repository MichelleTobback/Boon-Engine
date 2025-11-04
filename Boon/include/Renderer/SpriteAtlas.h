#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>
#include <queue>
#include <vector>

namespace Boon
{
	struct SpriteFrame
	{
		glm::vec2 UV;
		glm::vec2 Size;
		float FrameTime;
	};

	class SpriteAtlas;
	struct SpriteAnimClip
	{
		float Speed;
		SpriteAtlas* pAtlas;
		std::vector<int> Frames;
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

		inline void AddSpriteFrame(const SpriteFrame& uv)
		{
			int id = (int)m_Sprites.size();
			if (!m_FreeIds.empty())
			{
				id = m_FreeIds.front();
				m_FreeIds.pop();
			}
			m_Sprites[id] = uv;
		}

		inline void RemoveSpriteFrame(int index) { m_Sprites.erase(index); m_FreeIds.push(index); }
		inline void SetSpriteFrame(const SpriteFrame& uv, int index = 0) { m_Sprites[index] = uv; }
		inline const SpriteFrame& GetSpriteFrame(int index) const { return m_Sprites.at(index); }

		inline void AddClip(const SpriteAnimClip& clip) { m_Clips.push_back(clip); }
		inline void RemoveClip(int index) { m_Clips.erase(m_Clips.begin() + index); }
		inline const std::vector<SpriteAnimClip>& GetClips() const { return m_Clips; }
		inline SpriteAnimClip& GetClip(int index) { return m_Clips[index]; }

	private:
		std::shared_ptr<Texture2D> m_pTexture;
		std::unordered_map<int, SpriteFrame> m_Sprites;
		std::queue<int> m_FreeIds;

		std::vector<SpriteAnimClip> m_Clips;
	};
}