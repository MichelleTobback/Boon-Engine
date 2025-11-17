#pragma once
#include "Core/Variant.h"

#include <string>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <optional>
#include <utility>
#include <functional>

namespace Boon
{
    // ---------------------------------------------------------------------
    // Stable FNV-1a 32-bit hash for function IDs and name lookup
    // ---------------------------------------------------------------------
    constexpr uint32_t FNV1A32_OFFSET = 0x811C9DC5u;
    constexpr uint32_t FNV1A32_PRIME = 0x01000193u;

    inline uint32_t FNV1a32(const char* str)
    {
        uint32_t hash = FNV1A32_OFFSET;
        while (*str)
        {
            hash ^= static_cast<uint8_t>(*str++);
            hash *= FNV1A32_PRIME;
        }
        return hash;
    }

    inline uint32_t FNV1a32(const std::string& s)
    {
        return FNV1a32(s.c_str());
    }

    // ---------------------------------------------------------------------
    // Metadata and parameter info for functions
    // ---------------------------------------------------------------------
    struct BFunctionMeta
    {
        std::string key;
        std::string value;
    };

    struct BFunctionParam
    {
        std::string typeName;   // textual C++ type
        std::string name;       // identifier
        BTypeId typeId;         // inferred from typeName
    };

    struct BFunction
    {
        using ThunkFn = void(*)(void* /*instance*/, Variant* /*args*/, size_t /*argCount*/);

        uint32_t id = 0;                              // FNV1a hash of function name
        std::vector<BFunctionParam> params;           // reflected parameter info
        std::vector<BFunctionMeta> meta;              // function metadata
        ThunkFn thunk = nullptr;                      // invocation wrapper

        bool IsValid() const { return thunk != nullptr; }
    };

} // namespace Boon
