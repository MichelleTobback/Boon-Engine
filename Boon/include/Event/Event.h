#pragma once
#include <stdint.h>

namespace Boon
{
    enum class EventCategory : uint32_t 
    {
        None = 0,
        Application = 1 << 0,
        Window = 1 << 1,
        Input = 1 << 2,
        Scene = 1 << 3
    };

    // combine categories with | operator
    inline EventCategory operator|(EventCategory a, EventCategory b) 
    {
        return static_cast<EventCategory>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    namespace Event
    {
        inline bool HasCategory(EventCategory value, EventCategory category) 
        { 
            return (static_cast<uint32_t>(value) & static_cast<uint32_t>(category)) != 0; 
        }
    }

	struct IEvent 
	{
		virtual ~IEvent() = default;
		virtual EventCategory GetCategory() const = 0;
	};

    using EventListenerID = uint64_t;
}