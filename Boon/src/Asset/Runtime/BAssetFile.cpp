#include "Asset/Runtime/BAssetFile.h"

#include <fstream>

namespace Boon
{
    bool BAssetFile::Write(const std::filesystem::path& path, const AssetMeta& meta, const Buffer& payload)
    {
        if (!meta.IsValid())
            return false;

        std::filesystem::create_directories(path.parent_path());

        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        if (!file)
            return false;

        Header header{};
        header.type = static_cast<uint32_t>(meta.type);
        header.uuid = static_cast<uint64_t>(meta.uuid);
        header.payloadSize = static_cast<uint64_t>(payload.Size());

        file.write(reinterpret_cast<const char*>(&header), sizeof(Header));

        if (!payload.Empty())
            file.write(reinterpret_cast<const char*>(payload.Data()), static_cast<std::streamsize>(payload.Size()));

        return static_cast<bool>(file);
    }

    bool BAssetFile::Read(const std::filesystem::path& path, AssetMeta& outMeta, Buffer& outPayload)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file)
            return false;

        Header header{};
        file.read(reinterpret_cast<char*>(&header), sizeof(Header));

        if (!file || header.magic != Magic || header.version != Version)
            return false;

        outMeta = {};
        outMeta.uuid = AssetHandle(header.uuid);
        outMeta.type = static_cast<AssetType>(header.type);
        outMeta.runtimePath = path.lexically_normal();

        outPayload.Clear();
        outPayload.Resize(static_cast<size_t>(header.payloadSize));

        if (header.payloadSize > 0)
            file.read(reinterpret_cast<char*>(outPayload.Data()), static_cast<std::streamsize>(header.payloadSize));

        return static_cast<bool>(file);
    }
}
