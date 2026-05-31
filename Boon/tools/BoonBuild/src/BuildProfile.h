#pragma once
#include "BoonBuild/BuildPlatform.h"

#include <string>

namespace BoonBuild
{
    struct BuildProfile
    {
        std::string Name = "Windows-Debug";
        Boon::BuildPlatform Platform = Boon::BuildPlatform::Windows;
        std::string Configuration = "Debug";
        std::string Generator = "Ninja";
    };
}