#pragma once

#include "Module/ModuleManifest.h"

#include <filesystem>

namespace Boon
{
    class ModuleManifestLoader
    {
    public:
        static bool Load(const std::filesystem::path& path, ModuleManifest& outManifest);
    };
}