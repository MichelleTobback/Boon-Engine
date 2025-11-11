#pragma once
#include <type_traits>

namespace BoonEditor
{
    enum DebugRenderLayer : uint32_t
    {
        None = 0,
        Default = 1 << 0,
        Collision = 1 << 1,
        Gizmos = 1 << 2,
        Navigation = 1 << 3,

        Disabled = 1u << 31,

        All = 0x7FFFFFFF
    };

    inline DebugRenderLayer operator|(DebugRenderLayer lhs, DebugRenderLayer rhs)
    {
        using T = std::underlying_type_t<DebugRenderLayer>;
        return static_cast<DebugRenderLayer>(static_cast<T>(lhs) | static_cast<T>(rhs));
    }

    inline DebugRenderLayer operator&(DebugRenderLayer lhs, DebugRenderLayer rhs)
    {
        using T = std::underlying_type_t<DebugRenderLayer>;
        return static_cast<DebugRenderLayer>(static_cast<T>(lhs) & static_cast<T>(rhs));
    }

    inline DebugRenderLayer operator~(DebugRenderLayer value)
    {
        using T = std::underlying_type_t<DebugRenderLayer>;
        return static_cast<DebugRenderLayer>(~static_cast<T>(value));
    }

    inline DebugRenderLayer& operator|=(DebugRenderLayer& lhs, DebugRenderLayer rhs)
    {
        lhs = lhs | rhs;
        return lhs;
    }

    inline DebugRenderLayer& operator&=(DebugRenderLayer& lhs, DebugRenderLayer rhs)
    {
        lhs = lhs & rhs;
        return lhs;
    }
}