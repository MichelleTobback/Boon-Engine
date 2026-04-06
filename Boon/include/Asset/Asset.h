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
    /**
     * @brief Base class for all asset types used by the engine.
     *
     * Stores a handle identifying the asset. Concrete asset types derive from this class.
     */
    class Asset
    {
    public:
        Asset(AssetHandle handle)
            : m_Handle(handle){ }
        virtual ~Asset() = default;

        /**
         * @brief Get the UUID handle for this asset.
         *
         * @return AssetHandle value assigned to this asset.
         */
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
