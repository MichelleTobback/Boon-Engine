#pragma once

#include "Asset/Asset.h"
#include "Asset/AssetMeta.h"
#include "Asset/AssetSerializer.h"
#include "Asset/AssetTraits.h"
#include "Core/Memory/Buffer.h"

#include <memory>
#include <unordered_map>

namespace Boon
{
    class IAssetLoader
    {
    public:
        virtual ~IAssetLoader() = default;

        virtual AssetType GetType() const = 0;
        virtual std::unique_ptr<Asset> Load(Buffer& payload, const AssetMeta& meta) = 0;
    };

    template<typename T>
    class AssetLoaderT final : public IAssetLoader
    {
    public:
        AssetType GetType() const override
        {
            return AssetTraits<T>::Type;
        }

        std::unique_ptr<Asset> Load(Buffer& payload, const AssetMeta& meta) override
        {
            if (meta.type != AssetTraits<T>::Type)
                return nullptr;

            return std::unique_ptr<Asset>(AssetSerializer<T>::Load(payload, meta));
        }
    };

    class AssetLoaderRegistry final
    {
    public:
        template<typename T>
        void Register()
        {
            static_assert(std::is_base_of_v<Asset, T>);
            m_Loaders[AssetTraits<T>::Type] = std::make_unique<AssetLoaderT<T>>();
        }

        bool IsRegistered(AssetType type) const
        {
            return m_Loaders.find(type) != m_Loaders.end();
        }

        IAssetLoader* Get(AssetType type)
        {
            auto it = m_Loaders.find(type);
            return it == m_Loaders.end() ? nullptr : it->second.get();
        }

        const IAssetLoader* Get(AssetType type) const
        {
            auto it = m_Loaders.find(type);
            return it == m_Loaders.end() ? nullptr : it->second.get();
        }

        void Clear()
        {
            m_Loaders.clear();
        }

    private:
        std::unordered_map<AssetType, std::unique_ptr<IAssetLoader>> m_Loaders;
    };
}
