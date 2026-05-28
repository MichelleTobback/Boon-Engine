#pragma once

#include "Tools/TemplateProcessor.h"

#include <filesystem>

namespace BoonBuild
{
    class TemplateFileGenerator
    {
    public:
        bool Generate(
            const std::filesystem::path& templateFile,
            const std::filesystem::path& outputDir,
            const Boon::TemplateContext& context) const;

    private:
        static std::string ReadFile(const std::filesystem::path& path);
        static bool WriteFile(const std::filesystem::path& path, std::string_view content);
    };
}