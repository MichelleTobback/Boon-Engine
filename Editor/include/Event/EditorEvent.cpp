#include <Event/Event.h>
#include <Event/EventBus.h>
#include <Core/ServiceLocator.h>

#include "EditorState.h"

namespace BoonEditor
{
	struct EditorPlayStateChangeEvent final : public Event
	{
		EditorPlayStateChangeEvent(EditorState state)
			: State{state}{}
		virtual ~IEvent() = default;

		virtual EventCategory GetCategory() const override { return EventCategory::Custom; }

		EditorState State;
	}
}