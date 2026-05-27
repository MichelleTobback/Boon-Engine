#include "Renderer/ShaderCompiler/ShaderPreprocessor.h"

#include <fstream>
#include <sstream>

namespace Boon
{
    static std::string TrimLeft(std::string value)
    {
        const size_t first = value.find_first_not_of(" \t\r\n");
        if (first == std::string::npos)
            return {};

        return value.substr(first);
    }

    std::string ShaderPreprocessor::ResolveIncludes(
        const std::string& source,
        const std::filesystem::path& sourcePath)
    {
        std::unordered_set<std::filesystem::path> includeStack;
        return ResolveIncludes(source, sourcePath, includeStack);
    }

    std::string ShaderPreprocessor::ResolveIncludes(
        const std::string& source,
        const std::filesystem::path& currentDirectory,
        std::unordered_set<std::filesystem::path>& includeStack)
    {
        std::istringstream input(source);
        std::ostringstream output;

        std::string line;
        while (std::getline(input, line))
        {
            std::string includePathString;
            if (!TryParseInclude(line, includePathString))
            {
                output << line << '\n';
                continue;
            }

            const std::filesystem::path includePath = ResolveIncludePath(includePathString, currentDirectory);
            const std::filesystem::path normalizedPath = std::filesystem::weakly_canonical(includePath);

            if (includeStack.contains(normalizedPath))
            {
                output << "// Skipped recursive shader include: " << includePathString << '\n';
                continue;
            }

            std::ifstream includeFile(includePath, std::ios::in);
            if (!includeFile)
            {
                output << "// Failed to include shader file: " << includePathString << '\n';
                continue;
            }

            std::stringstream includeBuffer;
            includeBuffer << includeFile.rdbuf();

            includeStack.insert(normalizedPath);
            output << "// Begin include: " << includePathString << '\n';
            output << ResolveIncludes(includeBuffer.str(), includePath.parent_path(), includeStack);
            output << "// End include: " << includePathString << '\n';
            includeStack.erase(normalizedPath);
        }

        return output.str();
    }

    bool ShaderPreprocessor::TryParseInclude(const std::string& line, std::string& includePath)
    {
        const std::string trimmed = TrimLeft(line);
        constexpr const char* includeToken = "#include";

        if (trimmed.rfind(includeToken, 0) != 0)
            return false;

        const size_t firstQuote = trimmed.find('"');
        const size_t lastQuote = trimmed.find_last_of('"');

        if (firstQuote == std::string::npos || lastQuote == std::string::npos || firstQuote == lastQuote)
            return false;

        includePath = trimmed.substr(firstQuote + 1, lastQuote - firstQuote - 1);
        return !includePath.empty();
    }

    std::filesystem::path ShaderPreprocessor::ResolveIncludePath(
        const std::filesystem::path& includePath,
        const std::filesystem::path& currentDirectory)
    {
        if (includePath.is_absolute())
            return includePath;

        const std::filesystem::path localPath = currentDirectory / includePath;
        if (std::filesystem::exists(localPath))
            return localPath;

        // Engine include fallback. From Assets/shaders/Foo.glsl, this resolves
        // #include "Boon/SpriteBatch.glsl" to Assets/shaders/Boon/SpriteBatch.glsl.
        const std::filesystem::path engineShaderPath = currentDirectory / "Boon" / includePath.filename();
        if (std::filesystem::exists(engineShaderPath))
            return engineShaderPath;

        return localPath;
    }
}
