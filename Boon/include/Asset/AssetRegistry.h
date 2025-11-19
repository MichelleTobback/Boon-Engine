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
        const AssetMeta* Get(UUID id) const
        {
            auto it = m_Metadata.find(id);
            return it != m_Metadata.end() ? &it->second : nullptr;
        }

        void Add(const AssetMeta& meta)
        {
            m_Metadata[meta.uuid] = meta;
        }

        const auto& GetAll() const { return m_Metadata; }

        void Clear() { m_Metadata.clear(); }

    private:
        std::unordered_map<UUID, AssetMeta> m_Metadata;
    };
}
