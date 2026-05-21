#pragma once

#include "Asset/Asset.h"
#include "Asset/AssetMeta.h"
#include "Core/Memory/Buffer.h"

namespace Boon
{
    template<typename T>
    struct AssetSerializer
    {
        static T* Load(Buffer&, const AssetMeta&)
        {
            static_assert(sizeof(T) == 0, "No AssetSerializer<T>::Load specialization registered for this asset type.");
            return nullptr;
        }

        static Buffer Serialize(T*)
        {
            static_assert(sizeof(T) == 0, "No AssetSerializer<T>::Serialize specialization registered for this asset type.");
            return {};
        }
    };
}
