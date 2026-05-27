#pragma once

#include <filesystem>
#include <string>
#include <unordered_set>

namespace Boon
{
    class ShaderPreprocessor final
    {
    public:
        static std::string ResolveIncludes(
            const std::string& source,
            const std::filesystem::path& sourcePath);

        static std::string ResolveIncludes(
            const std::string& source,
            const std::filesystem::path& currentDirectory,
            std::unordered_set<std::filesystem::path>& includeStack);

    private:
        static bool TryParseInclude(const std::string& line, std::string& includePath);
        static std::filesystem::path ResolveIncludePath(
            const std::filesystem::path& includePath,
            const std::filesystem::path& currentDirectory);
    };
}
