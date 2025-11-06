#pragma once
#include "Event.h"
#include "Scene/Scene.h"

namespace Boon
{
	struct SceneChangedEvent : public IEvent
	{
		SceneChangedEvent(SceneID id)
			: ID(id) {}
		virtual ~SceneChangedEvent() = default;

		virtual EventCategory GetCategory() const override
		{
			return EventCategory::Scene;
		}

		SceneID ID;
	};
}