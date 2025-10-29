#pragma once
#include "Event.h"

namespace Boon
{
	struct WindowResizeEvent : public IEvent
	{
		WindowResizeEvent(int width, int height) 
			: Width(width), Height(height){}
		virtual ~WindowResizeEvent() = default;

		virtual EventCategory GetCategory() const override
		{
			return EventCategory::Application | EventCategory::Window;
		}

		int Width{}, Height{};
	};

	struct WindowCloseEvent : public IEvent
	{
		WindowCloseEvent() = default;
		virtual ~WindowCloseEvent() = default;

		virtual EventCategory GetCategory() const override
		{
			return EventCategory::Application | EventCategory::Window;
		}
	};
}