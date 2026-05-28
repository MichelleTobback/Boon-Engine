#include "Module/ModuleLibrary.h"
#include "Module/ModuleManifestLoader.h"
#include "Module/ModuleInstance.h"
#include "Core/DynamicLibrary.h"

#include <algorithm>
#include <filesystem>
#include <iostream>

namespace Boon
{
    ModuleLibrary::ModuleLibrary(const std::filesystem::path& projectRoot)
        : m_ProjectRoot{projectRoot}
    {
    }

    ModuleLibrary::~ModuleLibrary()
    {
    }

    bool ModuleLibrary::LoadManifest(const std::filesystem::path& manifestPath)
    {
        if (!ModuleManifestLoader::Load(manifestPath, m_Manifest))
        {
            std::cerr << "Failed to load module manifest: "
                << manifestPath << "\n";
            return false;
        }

        return true;
    }

    bool ModuleLibrary::LoadStartupModules(ModuleContext& ctx)
    {
        bool result = true;

        if (m_Manifest.GameModule.LoadOnStartup)
        {
            if (!LoadModule(m_Manifest.GameModule.Name, ctx))
                result = false;
        }

        for (const ModuleManifestEntry& entry : m_Manifest.DynamicModules)
        {
            if (!entry.LoadOnStartup)
                continue;

            if (!LoadModule(entry.Name, ctx))
                result = false;
        }

        return result;
    }

    bool ModuleLibrary::LoadModule(const std::string& name, ModuleContext& ctx)
    {
        const ModuleManifestEntry* entry = FindManifestEntry(name);

        if (!entry)
        {
            std::cerr << "Module not found in manifest: " << name << "\n";
            return false;
        }

        return LoadModuleFromEntry(*entry, ctx);
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
        {
            std::cerr << "Cannot reload unloaded module: " << name << "\n";
            return false;
        }

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
            it->UnregisterFn(&ctx, it->Instance);

        it->Instance = nullptr;

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
                it->UnregisterFn(&ctx, it->Instance);

            it->Instance = nullptr;

            if (it->Library)
                it->Library->Unload();

            DeleteLoadedCopy(it->LoadedPath);
        }

        m_Modules.clear();
    }

    const ModuleManifestEntry* ModuleLibrary::FindManifestEntry(const std::string& name) const
    {
        if (m_Manifest.GameModule.Name == name)
            return &m_Manifest.GameModule;

        auto dynamicIt = std::find_if(
            m_Manifest.DynamicModules.begin(),
            m_Manifest.DynamicModules.end(),
            [&](const ModuleManifestEntry& entry)
            {
                return entry.Name == name;
            });

        if (dynamicIt != m_Manifest.DynamicModules.end())
            return &(*dynamicIt);

        auto staticIt = std::find_if(
            m_Manifest.StaticModules.begin(),
            m_Manifest.StaticModules.end(),
            [&](const ModuleManifestEntry& entry)
            {
                return entry.Name == name;
            });

        if (staticIt != m_Manifest.StaticModules.end())
            return &(*staticIt);

        return nullptr;
    }

    bool ModuleLibrary::LoadModuleFromEntry(const ModuleManifestEntry& entry, ModuleContext& ctx)
    {
        if (entry.Type != ModuleBinaryType::Shared)
        {
            std::cerr << "Cannot dynamically load static module: "
                << entry.Name << "\n";
            return false;
        }

        const std::filesystem::path path = ResolveModulePath(entry);

        ++m_ReloadCounter;
        return LoadModuleInternal(path, ctx, m_ReloadCounter);
    }

    std::filesystem::path ModuleLibrary::ResolveModulePath(const ModuleManifestEntry& entry) const
    {
#ifdef NDEBUG
        const std::string config = "Release";
#else
        const std::string config = "Debug";
#endif

#ifdef _WIN32
        const std::string extension = ".dll";
#elif __APPLE__
        const std::string extension = ".dylib";
#else
        const std::string extension = ".so";
#endif

        return m_ProjectRoot / entry.Directory / config / entry.Subdirectory / (entry.Name + extension);
    }

    bool ModuleLibrary::LoadModuleInternal(
        const std::filesystem::path& originalPath,
        ModuleContext& ctx,
        uint64_t reloadGeneration)
    {
        if (!std::filesystem::exists(originalPath))
        {
            std::cerr << "Module DLL does not exist: "
                << originalPath << "\n";
            return false;
        }

        const std::filesystem::path loadedPath =
            MakeHotReloadCopyPath(originalPath, reloadGeneration);

        if (!CopyModuleForHotReload(originalPath, loadedPath))
        {
            std::cerr << "Failed to copy module for hot reload: "
                << originalPath << "\n";
            return false;
        }

        auto lib = std::make_unique<DynamicLibrary>();

        if (!lib->Load(loadedPath))
        {
            std::cerr << "Failed to load dynamic library: "
                << loadedPath << "\n";

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
            std::cerr << "Module missing required symbols: "
                << loadedPath << "\n";

            lib->Unload();
            DeleteLoadedCopy(loadedPath);
            return false;
        }

        const ModuleInfo* info = getInfo();

        if (!info || !info->Name || std::string(info->Name).empty())
        {
            std::cerr << "Module returned invalid ModuleInfo: "
                << loadedPath << "\n";

            lib->Unload();
            DeleteLoadedCopy(loadedPath);
            return false;
        }

        const std::string moduleName = info->Name;

        auto existingIt = std::find_if(
            m_Modules.begin(),
            m_Modules.end(),
            [&](const LoadedModule& module)
            {
                return module.Name == moduleName;
            });

        if (existingIt != m_Modules.end())
        {
            std::cerr << "Module already loaded: "
                << moduleName << "\n";

            lib->Unload();
            DeleteLoadedCopy(loadedPath);
            return false;
        }

        const ModuleRegistration registration = reg(&ctx);

        if (!registration.Success)
        {
            std::cerr << "Module registration failed: "
                << moduleName << "\n";

            lib->Unload();
            DeleteLoadedCopy(loadedPath);
            return false;
        }

        LoadedModule mod{};
        mod.Name = moduleName;
        mod.OriginalPath = originalPath;
        mod.LoadedPath = loadedPath;
        mod.Library = std::move(lib);
        mod.GetInfoFn = getInfo;
        mod.RegisterFn = reg;
        mod.UnregisterFn = unreg;
        mod.Instance = registration.Instance;
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