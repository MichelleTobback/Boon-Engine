#pragma once

#include "Asset/Runtime/BAssetFile.h"
#include "Asset/Source/IAssetSource.h"

#include <filesystem>

namespace Boon
{
    class FolderAssetSource final : public IAssetSource
    {
    public:
        explicit FolderAssetSource(std::filesystem::path root)
            : m_Root(std::move(root).lexically_normal())
        {
        }

        bool Read(const std::filesystem::path& runtimePath, Buffer& outPayload, AssetMeta& outMeta) override
        {
            const std::filesystem::path fullPath = Resolve(runtimePath);
            if (fullPath.empty())
                return false;

            if (!BAssetFile::Read(fullPath, outMeta, outPayload))
                return false;

            outMeta.runtimePath = runtimePath.lexically_normal();
            return true;
        }

        bool Read(AssetHandle, Buffer&, AssetMeta&) override
        {
            // Loose runtime files are path-addressed. Handle loading is done through AssetManifest.
            return false;
        }

        const std::filesystem::path& GetRoot() const { return m_Root; }

    private:
        std::filesystem::path Resolve(const std::filesystem::path& path) const
        {
            if (path.empty())
                return {};

            if (path.is_absolute())
                return std::filesystem::exists(path) ? path.lexically_normal() : std::filesystem::path{};

            const std::filesystem::path candidate = (m_Root / path).lexically_normal();
            return std::filesystem::exists(candidate) ? candidate : std::filesystem::path{};
        }

    private:
        std::filesystem::path m_Root;
    };
}
