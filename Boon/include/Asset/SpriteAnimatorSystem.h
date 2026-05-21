#pragma once
#include <Component/IECSSystem.h>
#include <Component/SpriteAnimatorComponent.h>
#include <Component/RenderComponents.h>

namespace Boon
{
    //class SpriteAnimatorSystem : public IECSSystem
    //{
    //public:
    //    virtual void OnUpdate(Scene& scene) override
    //    {
    //        auto group = scene.GetAllGameObjectsWith<SpriteAnimatorComponent, SpriteRendererComponent>();
    //        for (auto gameObject : group)
    //        {
    //            auto [animator, renderer] = group.get<SpriteAnimatorComponent, SpriteRendererComponent>(gameObject);
    //
    //            if (!animator.bPlaying)
    //                continue;
    //
    //            if (!animator.Atlas)
    //                continue;
    //
    //            if (animator.GetClipId() < 0 || animator.GetClipId() >= animator.Atlas->GetClips().size())
    //                continue;
    //
    //            SpriteAnimClip& clip = animator.GetClip();
    //            if (clip.Frames.empty())
    //                continue;
    //
    //            if (animator.CurrentFrame >= clip.Frames.size())
    //                animator.CurrentFrame = 0;
    //
    //            const int frameId = clip.Frames[animator.CurrentFrame];
    //            const float frameTime = animator.Atlas->GetSpriteFrame(frameId).FrameTime;
    //
    //            animator.Time += Time::Get().GetDeltaTime() * clip.Speed * animator.Speed;
    //
    //            if (animator.Time >= frameTime)
    //            {
    //                animator.Time = 0.0f;
    //                animator.CurrentFrame++;
    //
    //                if (animator.CurrentFrame >= clip.Frames.size())
    //                {
    //                    animator.CurrentFrame = animator.bLooping ? 0 : static_cast<int>(clip.Frames.size()) - 1;
    //                    animator.bPlaying = animator.bLooping;
    //                }
    //
    //                animator.bDirty = true;
    //            }
    //
    //            if (animator.bDirty)
    //            {
    //                renderer.Frame = clip.Frames[animator.CurrentFrame];
    //                renderer.pAtlas = animator.Atlas;
    //                animator.bDirty = false;
    //            }
    //        }
    //    }
    //};
}