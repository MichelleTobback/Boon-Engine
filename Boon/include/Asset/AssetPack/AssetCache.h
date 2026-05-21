#pragma once

#include "Asset/Asset.h"
#include "Core/UUID.h"

#include <memory>
#include <type_traits>
#include <unordered_map>

namespace Boon
{
    class AssetCache
    {
    public:
        template<typename T>
        T* Find(AssetHandle handle)
        {
            static_assert(std::is_base_of_v<Asset, T>);

            Asset* asset = FindUntyped(handle);
            return dynamic_cast<T*>(asset);
        }

        template<typename T>
        const T* Find(AssetHandle handle) const
        {
            static_assert(std::is_base_of_v<Asset, T>);

            const Asset* asset = FindUntyped(handle);
            return dynamic_cast<const T*>(asset);
        }

        Asset* FindUntyped(AssetHandle handle)
        {
            auto it = m_Assets.find(handle);
            return it == m_Assets.end() ? nullptr : it->second.get();
        }

        const Asset* FindUntyped(AssetHandle handle) const
        {
            auto it = m_Assets.find(handle);
            return it == m_Assets.end() ? nullptr : it->second.get();
        }

        template<typename T>
        T* Store(std::unique_ptr<T> asset)
        {
            static_assert(std::is_base_of_v<Asset, T>);

            if (!asset)
                return nullptr;

            AssetHandle handle = asset->GetHandle();
            T* raw = asset.get();
            m_Assets[handle] = std::move(asset);
            return raw;
        }

        template<typename T>
        T* Store(T* asset)
        {
            return Store(std::unique_ptr<T>(asset));
        }

        Asset* StoreUntyped(std::unique_ptr<Asset> asset)
        {
            if (!asset)
                return nullptr;

            AssetHandle handle = asset->GetHandle();
            Asset* raw = asset.get();
            m_Assets[handle] = std::move(asset);
            return raw;
        }

        bool Contains(AssetHandle handle) const
        {
            return m_Assets.find(handle) != m_Assets.end();
        }

        void Remove(AssetHandle handle)
        {
            m_Assets.erase(handle);
        }

        void Clear()
        {
            m_Assets.clear();
        }

    private:
        std::unordered_map<AssetHandle, std::unique_ptr<Asset>> m_Assets;
    };
}
