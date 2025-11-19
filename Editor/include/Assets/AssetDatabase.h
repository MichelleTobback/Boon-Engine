#pragma once
#include "Asset/Importer/AssetImporterRegistry.h"
#include "Asset/AssetRef.h"

using namespace Boon;

namespace BoonEditor
{
	class AssetDatabase final
	{
	public:
		static AssetDatabase& Get();

		void RegisterAsset(const std::string& path, AssetHandle handle);
		AssetHandle GetHandle(const std::string& path) const;
		const std::string& GetPath(AssetHandle handle) const;

		template<typename TAsset>
		AssetRef<TAsset> Load(const std::string& path)
		{
			AssetImporterRegistry& reg = AssetImporterRegistry::Get();
			AssetImporterRegistry::Imported<TAsset> result = reg.ImportAndLoad<TAsset>(path);
			return AssetRef<TAsset>(result.meta.uuid);
		}

		bool Exists(AssetHandle handle) const;
		bool Exists(const std::string& path) const;

		void Clear();

	private:
		std::unordered_map<AssetHandle, std::string> m_HandleToPath;
		std::unordered_map<std::string, AssetHandle> m_PathToHandle;
	};
}