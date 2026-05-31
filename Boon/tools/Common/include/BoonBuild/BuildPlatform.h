#pragma once

#include <string>

namespace Boon
{
    enum class BuildPlatform
    {
        Windows,
        Linux,
        Web,
        Android
    };

    inline std::string ToString(BuildPlatform platform)
    {
        switch (platform)
        {
        case BuildPlatform::Windows: return "Windows";
        case BuildPlatform::Linux:   return "Linux";
        case BuildPlatform::Web:     return "Web";
        case BuildPlatform::Android: return "Android";
        }

        return "Windows";
    }

    inline std::string ToCompileDefinition(BuildPlatform platform)
    {
        switch (platform)
        {
        case BuildPlatform::Windows: return "BOON_PLATFORM_WINDOWS=1";
        case BuildPlatform::Linux:   return "BOON_PLATFORM_LINUX=1";
        case BuildPlatform::Web:     return "BOON_PLATFORM_WEB=1";
        case BuildPlatform::Android: return "BOON_PLATFORM_ANDROID=1";
        }

        return "BOON_PLATFORM_WINDOWS=1";
    }

    inline bool TryParsePlatform(const std::string& value, BuildPlatform& outPlatform)
    {
        if (value == "Windows") { outPlatform = BuildPlatform::Windows; return true; }
        if (value == "Linux") { outPlatform = BuildPlatform::Linux; return true; }
        if (value == "Web") { outPlatform = BuildPlatform::Web; return true; }
        if (value == "Android") { outPlatform = BuildPlatform::Android; return true; }

        return false;
    }

    inline BuildPlatform ToPlatform(const std::string& value)
    {
        if (value == "Windows") { return BuildPlatform::Windows; }
        if (value == "Linux") { return BuildPlatform::Linux; }
        if (value == "Web") { return BuildPlatform::Web; }
        if (value == "Android") { return BuildPlatform::Android; }

        return BuildPlatform::Windows;
    }
}