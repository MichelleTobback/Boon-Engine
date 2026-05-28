#pragma once

#include "Module/ModuleAPI.h"
#include "Module/ModuleManifest.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#include <optional>

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

            ModuleInstance* Instance = nullptr;

            uint64_t ReloadGeneration = 0;
        };

    public:
        ModuleLibrary(const std::filesystem::path& projectRoot);
        ~ModuleLibrary();

        bool LoadManifest(const std::filesystem::path& manifestPath);

        bool LoadStartupModules(ModuleContext& ctx);

        bool LoadModule(const std::string& name, ModuleContext& ctx);
        bool UnloadModule(const std::string& name, ModuleContext& ctx);
        bool ReloadModule(const std::string& name, ModuleContext& ctx);

        void UnloadAll(ModuleContext& ctx);

        const std::vector<LoadedModule>& GetModules() const { return m_Modules; }
        const ModuleManifest& GetManifest() const { return m_Manifest; }

    private:
        const ModuleManifestEntry* FindManifestEntry(const std::string& name) const;

        bool LoadModuleFromEntry(const ModuleManifestEntry& entry, ModuleContext& ctx);

        bool LoadModuleInternal(
            const std::filesystem::path& originalPath,
            ModuleContext& ctx,
            uint64_t reloadGeneration);

        std::filesystem::path ResolveModulePath(const ModuleManifestEntry& entry) const;

        std::filesystem::path MakeHotReloadCopyPath(
            const std::filesystem::path& originalPath,
            uint64_t generation) const;

        bool CopyModuleForHotReload(
            const std::filesystem::path& originalPath,
            const std::filesystem::path& copyPath) const;

        void DeleteLoadedCopy(const std::filesystem::path& path) const;

    private:
        std::filesystem::path m_ProjectRoot;
        ModuleManifest m_Manifest;

        std::vector<LoadedModule> m_Modules;
        uint64_t m_ReloadCounter = 0;
    };
}