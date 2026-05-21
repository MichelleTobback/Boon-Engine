#pragma once

#include "Asset/AssetPack/AssetPackReader.h"
#include "Asset/Source/IAssetSource.h"

#include <memory>

namespace Boon
{
    class PackAssetSource final : public IAssetSource
    {
    public:
        explicit PackAssetSource(std::unique_ptr<AssetPackReader> reader)
            : m_Reader(std::move(reader))
        {
        }

        bool Read(const std::filesystem::path& runtimePath, Buffer& outPayload, AssetMeta& outMeta) override
        {
            return m_Reader && m_Reader->ReadAsset(runtimePath, outPayload, outMeta);
        }

        bool Read(AssetHandle handle, Buffer& outPayload, AssetMeta& outMeta) override
        {
            return m_Reader && m_Reader->ReadAsset(handle, outPayload, outMeta);
        }

    private:
        std::unique_ptr<AssetPackReader> m_Reader;
    };
}
