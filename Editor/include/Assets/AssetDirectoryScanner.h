#pragma once
#include "Core/EditorObject.h"
#include <filesystem>
#include <vector>
#include <string>
#include "Core/UUID.h"
#include "Asset/AssetLibrary.h"
#include "Asset/Importer/AssetImporterRegistry.h"

#include <filesystem>

namespace fs = std::filesystem;

using namespace Boon;

namespace BoonEditor
{
    class AssetDirectoryScanner final : public EditorObject
    {
    public:
        AssetDirectoryScanner(const std::string& root, float interval = 0.2f);

        void Update();

        void Scan();

    private:
        void ProcessFile(const std::filesystem::path& path);

    private:
        std::string m_Root;

        float m_Interval{};
        float m_Accumulator{};
    };
}