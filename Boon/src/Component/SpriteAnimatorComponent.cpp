#include "Component/SpriteAnimatorComponent.h"
#include "Component/SpriteRendererComponent.h"

#include "Asset/AssetLibrary.h"

#include "Core/ServiceLocator.h"
#include "Core/Time.h"

using namespace Boon;

void Boon::SpriteAnimatorComponent::Awake(GameObject obj)
{
	pRenderer = &obj.GetComponent<SpriteRendererComponent>();
}

void Boon::SpriteAnimatorComponent::Update(GameObject obj)
{
	if (!pRenderer)
		return;

	Atlas = pRenderer->SpriteAtlasHandle.Instance();
	if (!Atlas)
		return;

	if (Atlas->GetClips().size() <= Clip)
		return;

	SpriteAnimClip& clip = GetClip();

	if (clip.Frames.empty())
		return;

	if (m_Current >= clip.Frames.size())
		m_Current = clip.Frames.size() - 1;

	float maxTime = Atlas->GetSpriteFrame(clip.Frames[m_Current]).FrameTime;

	m_Timer += Time::Get().GetDeltaTime() * clip.Speed;
	if (m_Timer >= maxTime)
	{
		m_Timer = 0.f;

		if (++m_Current == clip.Frames.size())
			m_Current = 0;

		pRenderer->Sprite = clip.Frames[m_Current];
	}
}

void Boon::SpriteAnimatorComponent::SetClip(int clip, bool restart)
{
	if (Clip != clip && restart)
	{
		m_Timer = 0.f;
		m_Current = 0;
	}

	Clip = clip;
}