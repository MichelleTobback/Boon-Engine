#include "Module/ModuleLibrary.h"
#include "Core/DynamicLibrary.h"

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
        auto lib = std::make_unique<DynamicLibrary>();
        if (!lib->Load(dllPath))
            return false;

        auto getInfo = reinterpret_cast<const ModuleInfo * (*)()>(lib->GetSymbol("Boon_GetModuleInfo"));
        auto reg = reinterpret_cast<bool (*)(ModuleContext*)>(lib->GetSymbol("Boon_RegisterModule"));
        auto unreg = reinterpret_cast<void (*)(ModuleContext*)>(lib->GetSymbol("Boon_UnregisterModule"));

        if (!getInfo || !reg || !unreg)
            return false;

        if (!reg(&ctx))
            return false;

        LoadedModule mod;
        mod.Path = dllPath;
        mod.Library = std::move(lib);
        mod.GetInfoFn = getInfo;
        mod.RegisterFn = reg;
        mod.UnregisterFn = unreg;
        mod.Name = getInfo()->Name;

        m_Modules.push_back(std::move(mod));
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
        }

        m_Modules.clear();
    }
}