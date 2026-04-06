#pragma once
#include <cstdint>
#include <memory>
#include "Asset/Asset.h"
#include "Asset/AssetRegistry.h"
#include "Asset/AssetTraits.h"
#include "Asset/AssetRef.h"

namespace Boon
{
    class AssetPackReader;

    class AssetLoader
    {
    public:
        explicit AssetLoader(AssetPackReader* reader)
            : m_Reader(reader) {
        }

        /**
         * @brief Load an asset of type T from the asset pack using its handle.
         *
         * @tparam T Asset type to load.
         * @param handle Handle of the asset to load.
         * @return Pointer to the loaded asset of type T, or nullptr on failure.
         */
        template<typename T>
        T* Load(AssetHandle handle);

        /**
         * @brief Load an asset as the base Asset type.
         *
         * @param handle Handle of the asset to load.
         * @return Pointer to the loaded Asset, or nullptr on failure.
         */
        Asset* LoadGeneric(AssetHandle handle);

    private:
        AssetPackReader* m_Reader = nullptr;
    };
}
