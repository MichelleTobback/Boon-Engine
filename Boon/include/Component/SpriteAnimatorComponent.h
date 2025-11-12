#pragma once
#include "Core/Boon.h"
#include "Asset/SpriteAtlasAsset.h"

#include <vector>

namespace Boon
{
	struct SpriteRendererComponent;
	BCLASS(Category="Components", Name="Sprite animator")
	struct SpriteAnimatorComponent
	{
		SpriteAnimatorComponent() = default;

		inline SpriteAnimClip& GetClip() const { return Atlas->GetClip(Clip); }

		void Awake(GameObject);
		void Update(GameObject);

		void SetClip(int clip, bool restart = true);

		BPROPERTY(RangeMin="0", RangeMax="10", Slider)
		int Clip;
		std::shared_ptr<SpriteAtlas> Atlas;
		SpriteRendererComponent* pRenderer;
		
	private:
		int m_Current = 0;
		float m_Timer = 0.f;
	};
}