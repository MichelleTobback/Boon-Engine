#pragma once
#include <string>
#include <cstdint>
#include <memory>
#include <vector>
#include <typeindex>

#include "Core/UUID.h"
#include "Asset/AssetType.h"
#include "Asset/AssetTraits.h"

namespace Boon
{
    using AssetHandle = UUID;
    // Base class for all assets.
    class Asset
    {
    public:
        Asset(AssetHandle handle)
            : m_Handle(handle){ }
        virtual ~Asset() = default;

        AssetHandle GetHandle() const { return m_Handle; }

        template <typename T>
        static AssetType GetType()
        {
            return AssetTraits<T>::Type;
        }

    private:
        AssetHandle m_Handle{};
    };
}
