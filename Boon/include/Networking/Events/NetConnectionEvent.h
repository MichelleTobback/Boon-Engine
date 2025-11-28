#pragma once
#include "Event/Event.h"
#include "Networking/NetConnection.h"

namespace Boon
{
	struct NetConnectionEvent : public IEvent
	{
		NetConnectionEvent(uint64_t connectionId, ENetConnectionState state)
			: ConnectionId(connectionId), State(state) { }
		virtual ~NetConnectionEvent() = default;

		virtual EventCategory GetCategory() const override { return EventCategory::None; }

		uint64_t ConnectionId;
		ENetConnectionState State;
	};
}