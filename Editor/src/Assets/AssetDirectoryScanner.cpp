#include "Assets/AssetDirectoryScanner.h"
#include "Assets/AssetDatabase.h"
#include "Core/EditorContext.h"

#include <Core/Time.h>

namespace BoonEditor
{
    AssetDirectoryScanner::AssetDirectoryScanner(EditorContext* context, size_t assetRootIndex, float interval)
        : EditorObject(context)
        , m_AssetRootIndex(assetRootIndex)
        , m_Interval(interval)
    {
        Scan();
    }

    void AssetDirectoryScanner::Update()
    {
        Time& time = *GetContext().GetEngineContext().Time;
        m_Accumulator += time.GetDeltaTime();

        if (m_Accumulator >= m_Interval)
        {
            m_Accumulator = 0.f;
            Scan();
        }
    }

    void AssetDirectoryScanner::Scan()
    {
        auto& registry = Boon::ServiceLocator::Get<Boon::AssetImporterRegistry>();
        const auto& roots = registry.GetAssetRoots();

        if (m_AssetRootIndex >= roots.size())
            return;

        const fs::path& sourceRoot = roots[m_AssetRootIndex].sourceRoot;
        if (!fs::exists(sourceRoot))
            return;

        for (auto& entry : fs::recursive_directory_iterator(sourceRoot))
        {
            if (!entry.is_regular_file())
                continue;

            ProcessFile(entry.path());
        }
    }

    void AssetDirectoryScanner::ProcessFile(const fs::path& path)
    {
        if (path.extension() == ".meta" || path.extension() == ".basset")
            return;

        auto& registry = Boon::ServiceLocator::Get<Boon::AssetImporterRegistry>();
        if (!registry.HasExtension(path.extension().string()))
            return;

        if (AssetDatabase::Get().Exists(std::filesystem::relative(path, registry.GetAssetRoots()[m_AssetRootIndex].sourceRoot).string()))
            return;

        Boon::AssetMeta meta = registry.ImportFromRoot(m_AssetRootIndex, path);
        if (!meta.IsValid())
            return;

        AssetDatabase::Get().RegisterAsset(meta.sourcePath.generic_string(), meta.uuid, m_AssetRootIndex);
    }
}
