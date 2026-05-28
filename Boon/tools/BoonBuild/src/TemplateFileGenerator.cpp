#include "TemplateFileGenerator.h"

#include <fstream>
#include <iostream>
#include <sstream>

namespace BoonBuild
{
    std::string TemplateFileGenerator::ReadFile(const std::filesystem::path& path)
    {
        std::ifstream in(path, std::ios::binary);

        if (!in.is_open())
            return {};

        std::stringstream ss;
        ss << in.rdbuf();

        return ss.str();
    }

    bool TemplateFileGenerator::WriteFile(const std::filesystem::path& path, std::string_view content)
    {
        std::filesystem::create_directories(path.parent_path());

        std::ofstream out(path, std::ios::binary | std::ios::trunc);

        if (!out.is_open())
            return false;

        out << content;

        return true;
    }

    bool TemplateFileGenerator::Generate(
        const std::filesystem::path& templateFile,
        const std::filesystem::path& outputDir,
        const Boon::TemplateContext& context) const
    {
        const std::string templateText = ReadFile(templateFile);

        if (templateText.empty())
        {
            std::cerr << "Template file missing or empty:\n" << templateFile << "\n";
            return false;
        }

        return Boon::TemplateProcessor::ProcessTemplateBlocks(
            templateText,
            "@@",
            context,
            [&](const std::filesystem::path& relativePath, std::string_view content)
            {
                const std::filesystem::path outPath = outputDir / relativePath;

                std::cout << "Generating: " << outPath.string() << "\n";

                return WriteFile(outPath, content);
            });
    }
}