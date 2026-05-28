#include "Module/ModuleLibrary.h"

#include "Core/DynamicLibrary.h"

#include <algorithm>
#include <chrono>
#include <filesystem>

namespace Boon
{
	ModuleLibrary::ModuleLibrary()
	{
	}

	ModuleLibrary::~ModuleLibrary()
	{
	}

	bool ModuleLibrary::LoadModule(const std::filesystem::path& dllPath, ModuleContext& ctx)
	{
		++m_ReloadCounter;
		return LoadModuleInternal(dllPath, ctx, m_ReloadCounter);
	}

	bool ModuleLibrary::ReloadModule(const std::filesystem::path& dllPath, ModuleContext& ctx)
	{
		auto it = std::find_if(
			m_Modules.begin(),
			m_Modules.end(),
			[&](const LoadedModule& module)
			{
				return std::filesystem::equivalent(module.OriginalPath, dllPath);
			});

		if (it != m_Modules.end())
			return ReloadModule(it->Name, ctx);

		return LoadModule(dllPath, ctx);
	}

	bool ModuleLibrary::ReloadModule(const std::string& name, ModuleContext& ctx)
	{
		auto it = std::find_if(
			m_Modules.begin(),
			m_Modules.end(),
			[&](const LoadedModule& module)
			{
				return module.Name == name;
			});

		if (it == m_Modules.end())
			return false;

		const std::filesystem::path originalPath = it->OriginalPath;

		if (!UnloadModule(name, ctx))
			return false;

		++m_ReloadCounter;
		return LoadModuleInternal(originalPath, ctx, m_ReloadCounter);
	}

	bool ModuleLibrary::UnloadModule(const std::string& name, ModuleContext& ctx)
	{
		auto it = std::find_if(
			m_Modules.begin(),
			m_Modules.end(),
			[&](const LoadedModule& module)
			{
				return module.Name == name;
			});

		if (it == m_Modules.end())
			return false;

		if (it->UnregisterFn)
			it->UnregisterFn(&ctx);

		if (it->Library)
			it->Library->Unload();

		DeleteLoadedCopy(it->LoadedPath);

		m_Modules.erase(it);
		return true;
	}

	void ModuleLibrary::UnloadAll(ModuleContext& ctx)
	{
		for (auto it = m_Modules.rbegin(); it != m_Modules.rend(); ++it)
		{
			if (it->UnregisterFn)
				it->UnregisterFn(&ctx);

			if (it->Library)
				it->Library->Unload();

			DeleteLoadedCopy(it->LoadedPath);
		}

		m_Modules.clear();
	}

	bool ModuleLibrary::LoadModuleInternal(
		const std::filesystem::path& originalPath,
		ModuleContext& ctx,
		uint64_t reloadGeneration)
	{
		if (!std::filesystem::exists(originalPath))
			return false;

		const std::filesystem::path loadedPath =
			MakeHotReloadCopyPath(originalPath, reloadGeneration);

		if (!CopyModuleForHotReload(originalPath, loadedPath))
			return false;

		auto lib = std::make_unique<DynamicLibrary>();

		if (!lib->Load(loadedPath))
		{
			DeleteLoadedCopy(loadedPath);
			return false;
		}

		auto getInfo =
			reinterpret_cast<GetModuleInfoFn>(
				lib->GetSymbol("Boon_GetModuleInfo"));

		auto reg =
			reinterpret_cast<RegisterModuleFn>(
				lib->GetSymbol("Boon_RegisterModule"));

		auto unreg =
			reinterpret_cast<UnregisterModuleFn>(
				lib->GetSymbol("Boon_UnregisterModule"));

		if (!getInfo || !reg || !unreg)
		{
			lib->Unload();
			DeleteLoadedCopy(loadedPath);
			return false;
		}

		const ModuleInfo* info = getInfo();

		if (!info || !info->Name || std::string(info->Name).empty())
		{
			lib->Unload();
			DeleteLoadedCopy(loadedPath);
			return false;
		}

		if (!reg(&ctx))
		{
			lib->Unload();
			DeleteLoadedCopy(loadedPath);
			return false;
		}

		LoadedModule mod{};
		mod.Name = info->Name;
		mod.OriginalPath = originalPath;
		mod.LoadedPath = loadedPath;
		mod.Library = std::move(lib);
		mod.GetInfoFn = getInfo;
		mod.RegisterFn = reg;
		mod.UnregisterFn = unreg;
		mod.ReloadGeneration = reloadGeneration;

		m_Modules.push_back(std::move(mod));
		return true;
	}

	std::filesystem::path ModuleLibrary::MakeHotReloadCopyPath(
		const std::filesystem::path& originalPath,
		uint64_t generation) const
	{
		const std::filesystem::path dir =
			originalPath.parent_path() / ".hotreload";

		const std::string stem = originalPath.stem().string();
		const std::string ext = originalPath.extension().string();

		return dir / (stem + ".loaded." + std::to_string(generation) + ext);
	}

	bool ModuleLibrary::CopyModuleForHotReload(
		const std::filesystem::path& originalPath,
		const std::filesystem::path& copyPath) const
	{
		std::error_code ec;

		std::filesystem::create_directories(copyPath.parent_path(), ec);
		if (ec)
			return false;

		std::filesystem::copy_file(
			originalPath,
			copyPath,
			std::filesystem::copy_options::overwrite_existing,
			ec);

		return !ec;
	}

	void ModuleLibrary::DeleteLoadedCopy(const std::filesystem::path& path) const
	{
		if (path.empty())
			return;

		std::error_code ec;
		std::filesystem::remove(path, ec);
	}
}