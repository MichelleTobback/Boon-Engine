#pragma once
#include "Core/Window.h"
#include "Networking/NetworkSettings.h"

#include <string>
#include <filesystem>
#include <vector>

namespace Boon
{
    struct RuntimeConfig
    {
        std::filesystem::path ProjectRoot;
        std::filesystem::path EngineRoot;
        std::filesystem::path AssetsRoot;
        std::filesystem::path IntermediateRoot;
        std::filesystem::path SavedRoot;

        std::string StartupScene;
        std::string GameModule;

        std::vector<std::string> EnabledModules;

        struct WindowSettings
        {
            std::string Title = "Boon";
            std::filesystem::path Icon;
            uint32_t Width = 1280;
            uint32_t Height = 720;

            bool bFullscreen = false;
            bool bResizable = true;
            bool bBorderless = false;
        } Window;

        struct RenderSettings
        {
            bool bVSync = true;
            std::string Renderer = "Default";
        } Render;

        NetworkSettings Network{};
    };
}