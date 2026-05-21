#pragma once

#include "Asset/AssetRef.h"
#include "Asset/TextureAsset.h"
#include "Core/ServiceLocator.h"
#include "Asset/AssetLibrary.h"
#include "Assets/Importer/AssetImporterRegistry.h"

#include <functional>
#include <string>
#include <unordered_map>

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
			return ServiceLocator::Get<AssetLibrary>().Load<TAsset>(path);
		}

		template<typename TAsset>
		bool Export(AssetHandle handle)
		{
			const std::string& path = m_HandleToPath.at(handle);
			return ServiceLocator::Get<AssetImporterRegistry>().Export<TAsset>(path, handle);
		}

		bool Exists(AssetHandle handle) const;
		bool Exists(const std::string& path) const;

		void Clear();

		void ForEachEntry(const std::function<void(AssetHandle, const std::string&)>& fn);

		bool IsDirty() const { return m_Dirty; }
		void ClearDirty() { m_Dirty = false; }

		AssetRef<Texture2DAsset> GetThumbnail(AssetHandle handle) const;

	private:
		AssetDatabase();
		void Init();

		std::unordered_map<AssetHandle, std::string> m_HandleToPath;
		std::unordered_map<std::string, AssetHandle> m_PathToHandle;
		bool m_Dirty = false;

		mutable std::unordered_map<AssetType, AssetRef<Texture2DAsset>> m_DefaultTextures;
	};
}