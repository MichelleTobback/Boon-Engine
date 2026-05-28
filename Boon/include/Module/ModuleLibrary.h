#pragma once

#include "Module/ModuleAPI.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace Boon
{
	class DynamicLibrary;

	class ModuleLibrary
	{
	public:
		struct LoadedModule
		{
			std::string Name;

			std::filesystem::path OriginalPath;
			std::filesystem::path LoadedPath;

			std::unique_ptr<DynamicLibrary> Library;

			GetModuleInfoFn GetInfoFn = nullptr;
			RegisterModuleFn RegisterFn = nullptr;
			UnregisterModuleFn UnregisterFn = nullptr;

			uint64_t ReloadGeneration = 0;
		};

	public:
		ModuleLibrary();
		~ModuleLibrary();

		bool LoadModule(const std::filesystem::path& dllPath, ModuleContext& ctx);
		bool UnloadModule(const std::string& name, ModuleContext& ctx);
		bool ReloadModule(const std::string& name, ModuleContext& ctx);
		bool ReloadModule(const std::filesystem::path& dllPath, ModuleContext& ctx);

		void UnloadAll(ModuleContext& ctx);

		const std::vector<LoadedModule>& GetModules() const { return m_Modules; }

	private:
		bool LoadModuleInternal(
			const std::filesystem::path& originalPath,
			ModuleContext& ctx,
			uint64_t reloadGeneration);

		std::filesystem::path MakeHotReloadCopyPath(
			const std::filesystem::path& originalPath,
			uint64_t generation) const;

		bool CopyModuleForHotReload(
			const std::filesystem::path& originalPath,
			const std::filesystem::path& copyPath) const;

		void DeleteLoadedCopy(const std::filesystem::path& path) const;

	private:
		std::vector<LoadedModule> m_Modules;
		uint64_t m_ReloadCounter = 0;
	};
}