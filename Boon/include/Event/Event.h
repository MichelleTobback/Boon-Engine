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
        Scene = 1 << 3,
        Last = Scene,
        Custom = Last << 1
    };

    /**
     * @brief Bitmask categories used to classify events.
     */

    // combine categories with | operator
    inline EventCategory operator|(EventCategory a, EventCategory b) 
    {
        return static_cast<EventCategory>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    namespace Event
    {
        /**
         * @brief Check whether a category bit is set on a category value.
         *
         * @param value Category value to test.
         * @param category Category flag to check for.
         * @return true if the category bit is present.
         */
        inline bool HasCategory(EventCategory value, EventCategory category) 
        { 
            return (static_cast<uint32_t>(value) & static_cast<uint32_t>(category)) != 0; 
        }
    }

    /**
     * @brief Base interface for events in the system.
     *
     * Implementations must provide a category via GetCategory().
     */
    struct IEvent 
    {
        virtual ~IEvent() = default;
        virtual EventCategory GetCategory() const = 0;
    };

    using EventListenerID = uint64_t;
}