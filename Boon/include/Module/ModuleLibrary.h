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
            std::filesystem::path Path;
            std::unique_ptr<DynamicLibrary> Library;

            const ModuleInfo* (*GetInfoFn)() = nullptr;
            bool (*RegisterFn)(ModuleContext*) = nullptr;
            void (*UnregisterFn)(ModuleContext*) = nullptr;
        };

        ModuleLibrary();
        ~ModuleLibrary();

        bool LoadModule(const std::filesystem::path& dllPath, ModuleContext& ctx);
        void UnloadAll(ModuleContext& ctx);

        const std::vector<LoadedModule>& GetModules() const { return m_Modules; }

    private:
        std::vector<LoadedModule> m_Modules;
    };
}