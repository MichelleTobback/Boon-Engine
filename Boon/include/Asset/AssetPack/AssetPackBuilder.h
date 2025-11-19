#pragma once
#include <string>
#include <vector>

#include "Core/Memory/Buffer.h"
#include "AssetPack.h"
#include "Asset/AssetRegistry.h"
#include "Asset/Asset.h"

namespace Boon
{
    class AssetPackBuilder
    {
    public:
        explicit AssetPackBuilder(const std::string& outputPath);

        void AddAsset(UUID id, AssetType type, const Buffer& data);

        bool Build();

    private:
        std::string m_OutputPath;
        struct Entry
        {
            UUID id;
            AssetType type;
            Buffer data;
        };
        std::vector<Entry> m_Entries;
    };
}
