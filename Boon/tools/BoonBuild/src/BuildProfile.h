#pragma once
#include "BuildPlatform.h"

#include <string>

namespace BoonBuild
{
    struct BuildProfile
    {
        std::string Name = "Windows-Debug";
        BuildPlatform Platform = BuildPlatform::Windows;
        std::string Configuration = "Debug";
        std::string Generator = "Ninja";
    };
}