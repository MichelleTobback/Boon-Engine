#pragma once
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace BoonEditor
{
    class TemplateContext
    {
    public:
        using ValueMap = std::unordered_map<std::string, std::string>;
        using Resolver = std::function<std::optional<std::string>(std::string_view key)>;

    public:
        void Set(std::string key, std::string value);
        bool Contains(std::string_view key) const;
        const std::string* TryGet(std::string_view key) const;

        void SetResolver(Resolver resolver);
        std::optional<std::string> Resolve(std::string_view key) const;

    private:
        ValueMap m_Values;
        Resolver m_Resolver;
    };

    struct TemplateProcessorOptions
    {
        std::string OpenDelimiter = "{{";
        std::string CloseDelimiter = "}}";
        bool bKeepUnknownPlaceholders = true;
        bool bTrimPlaceholderWhitespace = true;
    };

    class TemplateProcessor
    {
    public:
        using FileCallback = std::function<bool(const std::filesystem::path& path, std::string_view content)>;

    public:
        static std::string Process(std::string_view input, const TemplateContext& context, const TemplateProcessorOptions& options = {});

        static void ProcessInPlace(std::string& input, const TemplateContext& context, const TemplateProcessorOptions& options = {});

        static bool ParseTemplateBlocks(std::string_view input, std::string_view fileToken, const FileCallback& onFile);

        static bool ProcessTemplateBlocks(std::string_view input, std::string_view fileToken, const TemplateContext& context,
            const FileCallback& onFile, const TemplateProcessorOptions& options = {});

    private:
        static std::string TrimCopy(std::string_view text);
        static std::string_view TrimLeft(std::string_view text);
        static std::string_view TrimRight(std::string_view text);
        static std::string_view Trim(std::string_view text);
    };
}