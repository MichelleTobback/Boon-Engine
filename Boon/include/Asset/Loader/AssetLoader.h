#pragma once

#include "Asset/Asset.h"
#include "Asset/AssetMeta.h"
#include "Asset/AssetSerializer.h"
#include "Asset/AssetTraits.h"
#include "Core/Memory/Buffer.h"

namespace Boon
{
    // Compatibility wrapper. New code should use AssetLoaderRegistry instead.
    class AssetLoader final
    {
    public:
        template<typename T>
        static T* Load(Buffer& payload, const AssetMeta& meta)
        {
            if (meta.type != AssetTraits<T>::Type)
                return nullptr;

            return AssetSerializer<T>::Load(payload, meta);
        }
    };
}
