#pragma once
#include <unordered_map>
#include <memory>
#include "Core/UUID.h"
#include "Asset/Asset.h"
#include "Asset/AssetRef.h"

namespace Boon
{
    class AssetCache
    {
    public:
        template<typename T>
        T* Find(AssetHandle handle)
        {
            auto it = m_Assets.find(handle);
            if (it == m_Assets.end())
                return nullptr;

            T* casted = dynamic_cast<T*>(it->second.get());
            return casted;
        }

        template<typename T>
        void Store(T* asset)
        {
            m_Assets[asset->GetHandle()] = std::unique_ptr<T>(asset);
        }

        void Clear()
        {
            m_Assets.clear();
        }

    private:
        std::unordered_map<AssetHandle, std::unique_ptr<Asset>> m_Assets;
    };
}
