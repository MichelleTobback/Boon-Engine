#pragma once

#include "SpriteRendererComponent.h"
#include "Asset/SpriteAtlasAsset.h"
#include "Asset/AssetLibrary.h"

#include "Core/ServiceLocator.h"
#include "Core/Time.h"

#include <vector>

namespace Boon
{
	struct SpriteAnimatorComponent final
	{
		SpriteAnimatorComponent() = default;

		inline SpriteAnimClip& GetClip() const { return Atlas->GetClip(Clip); }

		void Update()
		{
			SpriteAnimClip& clip = GetClip();

			if (clip.Frames.empty())
				return;

			if (m_Current >= clip.Frames.size())
				m_Current = clip.Frames.size() - 1;

			float maxTime = Atlas->GetSpriteFrame(clip.Frames[m_Current]).FrameTime;

			pRenderer->Sprite = clip.Frames[m_Current];

			m_Timer += Time::Get().GetDeltaTime() * clip.Speed;
			if (m_Timer >= maxTime)
			{
				m_Timer = 0.f;

				if (++m_Current == clip.Frames.size())
					m_Current = 0;
			}
		}

		int Clip;
		std::shared_ptr<SpriteAtlas> Atlas;
		SpriteRendererComponent* pRenderer;
		
	private:
		int m_Current = 0;
		float m_Timer = 0.f;
	};
}