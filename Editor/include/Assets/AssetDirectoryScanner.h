#pragma once
#include "Core/EditorObject.h"
#include <filesystem>
#include <vector>
#include <string>
#include "Core/UUID.h"
#include "Asset/AssetLibrary.h"
#include "Assets/Importer/AssetImporterRegistry.h"

namespace fs = std::filesystem;

using namespace Boon;

namespace BoonEditor
{
    class AssetDirectoryScanner final : public EditorObject
    {
    public:
        AssetDirectoryScanner(EditorContext* context, size_t assetRootIndex, float interval);

        void Update();

        void Scan();

    private:
        void ProcessFile(const std::filesystem::path& path);

    private:
        size_t m_AssetRootIndex;
        float m_Interval;
        float m_Accumulator{};
    };
}