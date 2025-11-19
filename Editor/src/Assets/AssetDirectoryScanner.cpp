#include "Assets/AssetDirectoryScanner.h"
#include "Assets/AssetDatabase.h"

#include <Core/Time.h>

namespace BoonEditor
{
    AssetDirectoryScanner::AssetDirectoryScanner(const std::string& root, float interval)
        : m_Root(root), m_Interval(interval)
    {
    }

    void AssetDirectoryScanner::Update()
    {
        m_Accumulator += Time::Get().GetDeltaTime();

        if (m_Accumulator >= m_Interval)
        {
            m_Accumulator = 0.f;
            Scan();
        }
    }

    void AssetDirectoryScanner::Scan()
    {
        for (auto& entry : fs::recursive_directory_iterator(m_Root))
        {
            if (!entry.is_regular_file())
                continue;

            ProcessFile(entry.path());
        }
    }

    void AssetDirectoryScanner::ProcessFile(const fs::path& path)
    {
        auto& database = AssetDatabase::Get();
        // Skip .meta files
        if (path.extension() == ".meta")
            return;

        if (database.Exists(path.string()))
            return;

        // Ask importer registry if this extension maps to an asset type
        auto ext = path.extension().string();
        auto& registry = Boon::AssetImporterRegistry::Get();
        if (!registry.HasExtension(ext))
            return;

        auto imported = registry.Import(path.string());
        database.RegisterAsset(path.string(), imported.meta.uuid);
    }
}
