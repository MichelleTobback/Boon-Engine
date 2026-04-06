#pragma once
#include "Asset/AssetMeta.h"
#include <unordered_map>
#include <string>
#include "Core/UUID.h"
#include "Asset.h"

namespace Boon
{
    class AssetRegistry
    {
    public:
        /**
         * @brief Get metadata for the given asset id.
         *
         * @param id UUID handle of the asset.
         * @return Pointer to the AssetMeta if present, otherwise nullptr.
         */
        const AssetMeta* Get(UUID id) const
        {
            auto it = m_Metadata.find(id);
            return it != m_Metadata.end() ? &it->second : nullptr;
        }

        /**
         * @brief Add or replace metadata for an asset.
         *
         * @param meta Metadata to store in the registry.
         */
        void Add(const AssetMeta& meta)
        {
            m_Metadata[meta.uuid] = meta;
        }

        /**
         * @brief Get the full metadata map.
         *
         * @return Const reference to the internal metadata map.
         */
        const auto& GetAll() const { return m_Metadata; }

        /**
         * @brief Clear all registered metadata entries.
         */
        void Clear() { m_Metadata.clear(); }

    private:
        std::unordered_map<UUID, AssetMeta> m_Metadata;
    };
}
