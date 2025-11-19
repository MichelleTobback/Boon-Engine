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

        template<typename T>
        T* Load(AssetHandle handle);
        Asset* LoadGeneric(AssetHandle handle);

    private:
        AssetPackReader* m_Reader = nullptr;
    };
}
