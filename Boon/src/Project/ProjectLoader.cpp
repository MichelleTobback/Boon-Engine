#include "Project/ProjectLoader.h"
#include "BoonDebug/Logger.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

#include <windows.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Boon
{

    static std::filesystem::path s_exepath{};

    void from_json(const json& j, ENetDriverMode& mode)
    {
        const std::string value = j.get<std::string>();

        if (value == "Standalone")
            mode = ENetDriverMode::Standalone;
        else if (value == "Client")
            mode = ENetDriverMode::Client;
        else if (value == "ListenServer")
            mode = ENetDriverMode::ListenServer;
        else if (value == "DedicatedServer")
            mode = ENetDriverMode::DedicatedServer;
        else
            throw std::runtime_error("Invalid DriverMode: " + value);
    }
    void to_json(json& j, const ENetDriverMode& mode)
    {
        switch (mode)
        {
        case ENetDriverMode::Standalone:      j = "Standalone"; break;
        case ENetDriverMode::Client:          j = "Client"; break;
        case ENetDriverMode::ListenServer:    j = "ListenServer"; break;
        case ENetDriverMode::DedicatedServer: j = "DedicatedServer"; break;
        default:                              j = "Standalone"; break;
        }
    }

    void from_json(const json& j, RuntimeConfig::WindowSettings& w)
    {
        if (j.contains("Title"))        j.at("Title").get_to(w.Title);
        if (j.contains("Width"))        j.at("Width").get_to(w.Width);
        if (j.contains("Height"))       j.at("Height").get_to(w.Height);
        if (j.contains("bFullscreen"))  j.at("bFullscreen").get_to(w.bFullscreen);
        if (j.contains("bResizable"))   j.at("bResizable").get_to(w.bResizable);
        if (j.contains("bBorderless"))  j.at("bBorderless").get_to(w.bBorderless);
    }
    void to_json(json& j, const RuntimeConfig::WindowSettings& w)
    {
        j = json{
            { "Title",        w.Title },
            { "Width",        w.Width },
            { "Height",       w.Height },
            { "bFullscreen",  w.bFullscreen },
            { "bResizable",   w.bResizable },
            { "bBorderless",  w.bBorderless }
        };
    }

    void from_json(const json& j, RuntimeConfig::RenderSettings& r)
    {
        if (j.contains("bVSync"))   j.at("bVSync").get_to(r.bVSync);
        if (j.contains("Renderer")) j.at("Renderer").get_to(r.Renderer);
    }
    void to_json(json& j, const RuntimeConfig::RenderSettings& r)
    {
        j = json{
            { "bVSync",   r.bVSync },
            { "Renderer", r.Renderer }
        };
    }

    void from_json(const json& j, NetworkSettings& n)
    {
        if (j.contains("DriverMode")) j.at("DriverMode").get_to(n.NetMode);
    }
    void to_json(json& j, const NetworkSettings& n)
    {
        j = json{
            { "DriverMode", n.NetMode }
        };
    }

    void from_json(const json& j, RuntimeConfig& r)
    {
        //if (j.contains("ProjectRoot"))       j.at("ProjectRoot").get_to(r.ProjectRoot);
        if (j.contains("AssetsRoot"))        j.at("AssetsRoot").get_to(r.AssetsRoot);
        //if (j.contains("IntermediateRoot"))  j.at("IntermediateRoot").get_to(r.IntermediateRoot);
        //if (j.contains("SavedRoot"))         j.at("SavedRoot").get_to(r.SavedRoot);

        if (j.contains("StartupScene")) j.at("StartupScene").get_to(r.StartupScene);
        if (j.contains("GameModule"))   j.at("GameModule").get_to(r.GameModule);

        if (j.contains("EnabledModules"))
            j.at("EnabledModules").get_to(r.EnabledModules);

        if (j.contains("Window"))
            j.at("Window").get_to(r.Window);

        if (j.contains("Render"))
            j.at("Render").get_to(r.Render);

        if (j.contains("Network"))
            j.at("Network").get_to(r.Network);
    }
    void to_json(json& j, const RuntimeConfig& r)
    {
        j = json{
            //{ "ProjectRoot",      r.ProjectRoot.string() },
            { "AssetsRoot",       r.AssetsRoot.string() },
            //{ "IntermediateRoot", r.IntermediateRoot.string() },
            //{ "SavedRoot",        r.SavedRoot.string() },
            { "StartupScene",     r.StartupScene },
            { "GameModule",       r.GameModule },
            { "EnabledModules",   r.EnabledModules },
            { "Window",           r.Window },
            { "Render",           r.Render },
            { "Network",          r.Network }
        };
    }

    void from_json(const json&, ProjectConfig::EditorConfig&)
    {
        
    }
    void to_json(json&, const ProjectConfig::EditorConfig&)
    {
       
    }

    void from_json(const json& j, ProjectConfig& p)
    {
        if (j.contains("Name"))    j.at("Name").get_to(p.Name);
        if (j.contains("Version")) j.at("Version").get_to(p.Version);

        if (j.contains("Runtime"))
            j.at("Runtime").get_to(p.Runtime);

        if (j.contains("Editor"))
            j.at("Editor").get_to(p.Editor);
    }
    void to_json(json& j, const ProjectConfig& p)
    {
        j = json{
            { "Name",    p.Name },
            { "Version", p.Version },
            { "Runtime", p.Runtime },
            { "Editor",  p.Editor }
        };
    }

    std::filesystem::path GetExecutablePath()
    {
        wchar_t buffer[MAX_PATH];
        GetModuleFileNameW(nullptr, buffer, MAX_PATH);
        return std::filesystem::path(buffer);
    }

    TResult<ProjectConfig> ProjectLoader::LoadFromFile(const std::filesystem::path& projectFilePath)
    {
        TResult<ProjectConfig> result{};

        std::filesystem::path p = projectFilePath.lexically_normal();

        static bool sIsInitialized = false;
        if (!sIsInitialized)
        {
            s_exepath = GetExecutablePath().parent_path();
            std::filesystem::current_path(s_exepath);
            sIsInitialized = true;
        }

        if (p.empty())
        {
            ProjectLoadError err;
            err.Code = EProjectLoadErrorCode::InvalidConfig;
            err.Message = "Project file path is empty.";
            result.Error = err;
            BOON_LOG_ERROR("{}", err.Message);
            return result;
        }

        if (!std::filesystem::exists(p))
        {
            ProjectLoadError err;
            err.Code = EProjectLoadErrorCode::FileNotFound;
            err.Message = "Project file does not exist: " + p.string();
            result.Error = err;
            BOON_LOG_ERROR("{}", err.Message);
            return result;
        }

        std::string fileText;
        if (!ReadTextFile(p, fileText))
        {
            ProjectLoadError err;
            err.Code = EProjectLoadErrorCode::FileReadFailed;
            err.Message = "Failed to read project file: " + p.string();
            result.Error = err;
            BOON_LOG_ERROR("{}", err.Message);
            return result;
        }

        ProjectConfig config{};
        ApplyDefaults(config);

        std::string parseError;
        if (!DeserializeProjectConfig(fileText, config, parseError))
        {
            ProjectLoadError err;
            err.Code = EProjectLoadErrorCode::ParseFailed;
            err.Message = "Failed to parse project file '" + p.string() + "': " + parseError;
            result.Error = err;
            BOON_LOG_ERROR("{}", err.Message);
            return result;
        }

        ApplyDefaults(config);
        ResolvePaths(config, p);

        ProjectLoadError validationError{};
        if (!Validate(config, validationError))
        {
            result.Error = validationError;
            BOON_LOG_ERROR("{}", validationError.Message);
            return result;
        }

        result.Value = std::move(config);

        std::filesystem::path absolute = std::filesystem::absolute(p.parent_path());
        std::filesystem::current_path(absolute);

        return result;
    }

    std::filesystem::path GetBinRelativeToExe()
    {
        auto exe = GetExecutablePath();

        auto configDir = exe.parent_path().parent_path();   // Debug/
        auto binDir = configDir.parent_path();              // bin/

        return binDir.filename() / configDir.filename();    // "bin/Debug"
    }

    void ProjectLoader::ApplyDefaults(ProjectConfig& config)
    {
        if (config.Name.empty())
            config.Name = "Boon Project";

        if (config.Version == 0)
            config.Version = 1;

        if (config.Runtime.StartupScene.empty())
            config.Runtime.StartupScene = "Scenes/Main.scene";

        if (config.Runtime.AssetsRoot.empty())
            config.Runtime.AssetsRoot = "Assets";

        if (config.Runtime.IntermediateRoot.empty())
            config.Runtime.IntermediateRoot = GetBinRelativeToExe();

        if (config.Runtime.SavedRoot.empty())
            config.Runtime.SavedRoot = "Saved";

        if (config.Runtime.Window.Title.empty())
            config.Runtime.Window.Title = config.Name;

        config.Runtime.EngineRoot = s_exepath / "../Boon";
        config.Editor.EditorResourcesRoot = s_exepath;
    }

    bool ProjectLoader::SaveToFile(const std::filesystem::path& location, const ProjectConfig& projectConfig)
    {
        if (location.empty())
            return false;

        const std::filesystem::path filePath = location / (projectConfig.Name + ".bproj");

        try
        {
            json j = projectConfig;

            std::ofstream file(filePath, std::ios::out | std::ios::binary | std::ios::trunc);
            if (!file.is_open())
                return false;

            file << j.dump(4);

            if (file.fail())
            {
                BOON_LOG_ERROR("Failed to save project file: {}", filePath.string());
                return false;
            }

            BOON_LOG("Project .bproj file saved at: {}", filePath.string());
            return true;
        }
        catch (const std::exception&)
        {
            BOON_LOG_ERROR("Failed to save project file: {}", filePath.string());
            return false;
        }
    }

    void ProjectLoader::ResolvePaths(ProjectConfig& config, const std::filesystem::path& projectFilePath)
    {
        const std::filesystem::path projectRoot = projectFilePath.parent_path();
        config.Runtime.ProjectRoot = projectRoot;

        auto ResolveRelativeToProject = [&projectRoot](std::filesystem::path& path)
            {
                if (path.empty())
                    return;

                if (path.is_relative())
                    path = projectRoot / path;

                path = path.lexically_normal();
            };

        //ResolveRelativeToProject(config.Runtime.AssetsRoot);
        //ResolveRelativeToProject(config.Runtime.EngineRoot);
        //ResolveRelativeToProject(config.Runtime.IntermediateRoot);
        //ResolveRelativeToProject(config.Runtime.SavedRoot);
        //ResolveRelativeToProject(config.Editor.EditorResourcesRoot);
    }

    bool ProjectLoader::Validate(const ProjectConfig& config, ProjectLoadError& outError)
    {
        if (config.Version == 0)
        {
            outError.Code = EProjectLoadErrorCode::UnsupportedVersion;
            outError.Message = "Project version 0 is invalid.";
            return false;
        }

        if (config.Version > 1)
        {
            outError.Code = EProjectLoadErrorCode::UnsupportedVersion;
            outError.Message = "Unsupported project version: " + std::to_string(config.Version);
            return false;
        }

        if (config.Name.empty())
        {
            outError.Code = EProjectLoadErrorCode::InvalidConfig;
            outError.Message = "Project name must not be empty.";
            return false;
        }

        if (config.Runtime.Window.Width == 0 || config.Runtime.Window.Height == 0)
        {
            outError.Code = EProjectLoadErrorCode::InvalidConfig;
            outError.Message = "Window width and height must be greater than zero.";
            return false;
        }

        if (config.Runtime.GameModule.empty())
        {
            outError.Code = EProjectLoadErrorCode::InvalidConfig;
            outError.Message = "Runtime.GameModule must not be empty.";
            return false;
        }

        if (config.Runtime.StartupScene.empty())
        {
            outError.Code = EProjectLoadErrorCode::InvalidConfig;
            outError.Message = "Runtime.StartupScene must not be empty.";
            return false;
        }

        return true;
    }

    bool ProjectLoader::ReadTextFile(const std::filesystem::path& path, std::string& outText)
    {
        std::ifstream file(path, std::ios::in | std::ios::binary);
        if (!file.is_open())
            return false;

        std::ostringstream buffer;
        buffer << file.rdbuf();

        if (file.fail() && !file.eof())
            return false;

        outText = buffer.str();
        return true;
    }

    bool ProjectLoader::DeserializeProjectConfig(
        const std::string& text,
        ProjectConfig& outConfig,
        std::string& outErrorMessage)
    {
        if (text.empty())
        {
            outErrorMessage = "File is empty.";
            return false;
        }

        try
        {
            const json j = json::parse(text);
            j.get_to(outConfig);
            return true;
        }
        catch (const std::exception& e)
        {
            outErrorMessage = e.what();
            return false;
        }
    }
}