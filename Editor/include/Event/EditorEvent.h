#include <Event/Event.h>
#include <Event/EventBus.h>
#include <Core/ServiceLocator.h>

#include "Core/BoonEditor.h"

using namespace Boon;

namespace BoonEditor
{
	struct EditorPlayStateChangeEvent final : public IEvent
	{
		EditorPlayStateChangeEvent(EditorPlayState state)
			: State{ state } {}
		virtual ~EditorPlayStateChangeEvent() = default;

		virtual EventCategory GetCategory() const override { return EventCategory::Custom; }

		EditorPlayState State;
	};
}