#pragma once
#include "Scene/GameObject.h"
#include "Asset/SpriteAtlasAsset.h"

#include <vector>

#define BCLASS()

namespace Boon
{
	struct SpriteRendererComponent;
	BCLASS()
	struct SpriteAnimatorComponent final
	{
		SpriteAnimatorComponent() = default;

		inline SpriteAnimClip& GetClip() const { return Atlas->GetClip(Clip); }

		void Update(GameObject);

		int Clip;
		std::shared_ptr<SpriteAtlas> Atlas;
		SpriteRendererComponent* pRenderer;
		
	private:
		int m_Current = 0;
		float m_Timer = 0.f;
	};
}