#pragma once

#include "Project/ProjectConfig.h"

#include <filesystem>
#include <optional>
#include <string>

namespace Boon
{
    enum class EProjectLoadErrorCode
    {
        None = 0,
        FileNotFound,
        FileReadFailed,
        ParseFailed,
        UnsupportedVersion,
        InvalidConfig
    };

    struct ProjectLoadError
    {
        EProjectLoadErrorCode Code = EProjectLoadErrorCode::None;
        std::string Message;
    };

    template<typename T>
    struct TResult
    {
        T Value{};
        std::optional<ProjectLoadError> Error;

        [[nodiscard]] bool Succeeded() const
        {
            return !Error.has_value();
        }

        [[nodiscard]] explicit operator bool() const
        {
            return Succeeded();
        }
    };

    class ProjectLoader final
    {
    public:
        [[nodiscard]]
        static TResult<ProjectConfig> LoadFromFile(const std::filesystem::path& projectFilePath);
        static void ApplyDefaults(ProjectConfig& config);

        static bool SaveToFile(const std::filesystem::path& location, const ProjectConfig& projectConfig);

    private:
        static void ResolvePaths(ProjectConfig& config, const std::filesystem::path& projectFilePath);
        static bool Validate(const ProjectConfig& config, ProjectLoadError& outError);

        static bool ReadTextFile(const std::filesystem::path& path, std::string& outText);

        static bool DeserializeProjectConfig(
            const std::string& text,
            ProjectConfig& outConfig,
            std::string& outErrorMessage);
    };
}