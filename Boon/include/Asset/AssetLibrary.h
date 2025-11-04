#pragma once
#include "AssetLoader.h"
#include "Core/Assert.h"

#include <vector>
#include <unordered_map>
#include <filesystem>

namespace Boon
{
	class AssetLibrary final
	{
	public:
		AssetLibrary(const std::string& assetDirectory);
		~AssetLibrary();

		AssetLibrary(const AssetLibrary& other) = delete;
		AssetLibrary(AssetLibrary&& other) = delete;
		AssetLibrary& operator=(const AssetLibrary& other) = delete;
		AssetLibrary& operator=(AssetLibrary&& other) = delete;

		template <typename T>
		AssetHandle Load(const std::string& path, uint32_t location = 0);
		template <typename T, typename TType = T::Type>
		std::shared_ptr<TType> GetAsset(AssetHandle handle);

		void AddDirectory(const std::string& directory);

		template <typename T>
		void RegisterLoader();

		bool IsValidAsset(AssetHandle handle) const;

	private:
		std::vector<std::unique_ptr<AssetLoader>> m_AssetLoaders;
		std::unordered_map<std::string, AssetLoader*> m_ExtensionsToLoaders;

		std::unordered_map<AssetHandle, std::unique_ptr<Asset>> m_Assets;
		std::unordered_map<AssetTypeID, std::vector<AssetHandle>> m_TypesToHandles;
		std::unordered_map<std::string, AssetHandle> m_Paths;

		std::vector<std::string> m_Dirs{};
	};

	template<typename T>
	inline AssetHandle AssetLibrary::Load(const std::string& path, uint32_t location)
	{
		std::filesystem::path fullPath = std::filesystem::path(m_Dirs[location]) / path;

		auto it{ m_Paths.find(fullPath.string()) };
		if (it != m_Paths.end())
			return it->second;

		std::string extension{ fullPath.extension().string() };
		if (!extension.empty() && extension[0] == '.')
			extension = extension.substr(1);

		auto extIt = m_ExtensionsToLoaders.find(extension);
		if (extIt == m_ExtensionsToLoaders.end())
			return 0u;
		AssetLoader* pLoader = extIt->second;

		if (pLoader)
		{
			auto pAsset{ pLoader->Load(fullPath.string()) };
			AssetHandle handle{ pAsset->GetHandle() };
			m_Assets[handle] = std::move(pAsset);
			m_TypesToHandles[typeid(T).hash_code()].push_back(handle);
			m_Paths[fullPath.string()] = handle;
			return handle;
		}
		return 0u;
	}

	template<typename T, typename TType>
	inline std::shared_ptr<TType> AssetLibrary::GetAsset(AssetHandle handle)
	{
		BN_ASSERT(m_Assets.find(handle) != m_Assets.end(), "asset does not exist");
		T* pAsset{ dynamic_cast<T*>(m_Assets[handle].get()) };
		BN_ASSERT(pAsset, "asset is not valid");

		return pAsset->GetInstance();
	}

	template<typename T>
	inline void AssetLibrary::RegisterLoader()
	{
		auto& pLoader{ m_AssetLoaders.emplace_back(std::move(T::Create())) };
		const std::vector<std::string>& extensions{ pLoader->GetExtensions() };
		for (const std::string& ext : extensions)
		{
			m_ExtensionsToLoaders[ext] = pLoader.get();
		}
	}
}