#include "Tools/TemplateProcessor.h"

#include <cctype>
#include <utility>

using namespace BoonEditor;

void TemplateContext::Set(std::string key, std::string value)
{
    m_Values[std::move(key)] = std::move(value);
}

bool TemplateContext::Contains(std::string_view key) const
{
    return m_Values.find(std::string(key)) != m_Values.end();
}

const std::string* TemplateContext::TryGet(std::string_view key) const
{
    auto it = m_Values.find(std::string(key));
    if (it == m_Values.end())
        return nullptr;

    return &it->second;
}

void TemplateContext::SetResolver(Resolver resolver)
{
    m_Resolver = std::move(resolver);
}

std::optional<std::string> TemplateContext::Resolve(std::string_view key) const
{
    if (const std::string* pValue = TryGet(key))
        return *pValue;

    if (m_Resolver)
        return m_Resolver(key);

    return std::nullopt;
}

std::string TemplateProcessor::Process(std::string_view input, const TemplateContext& context, const TemplateProcessorOptions& options)
{
    std::string output;
    output.reserve(input.size());

    const std::string_view open = options.OpenDelimiter;
    const std::string_view close = options.CloseDelimiter;

    size_t cursor = 0;

    while (cursor < input.size())
    {
        const size_t openPos = input.find(open, cursor);
        if (openPos == std::string_view::npos)
        {
            output.append(input.substr(cursor));
            break;
        }

        output.append(input.substr(cursor, openPos - cursor));

        const size_t tokenStart = openPos + open.size();
        const size_t closePos = input.find(close, tokenStart);

        if (closePos == std::string_view::npos)
        {
            output.append(input.substr(openPos));
            break;
        }

        std::string key = options.bTrimPlaceholderWhitespace
            ? TrimCopy(input.substr(tokenStart, closePos - tokenStart))
            : std::string(input.substr(tokenStart, closePos - tokenStart));

        std::optional<std::string> resolved = context.Resolve(key);
        if (resolved.has_value())
        {
            output.append(*resolved);
        }
        else if (options.bKeepUnknownPlaceholders)
        {
            output.append(open);
            output.append(input.substr(tokenStart, closePos - tokenStart));
            output.append(close);
        }

        cursor = closePos + close.size();
    }

    return output;
}

void TemplateProcessor::ProcessInPlace(std::string& input, const TemplateContext& context, const TemplateProcessorOptions& options)
{
    input = Process(input, context, options);
}

bool TemplateProcessor::ParseTemplateBlocks(std::string_view input, std::string_view fileToken, const FileCallback& onFile)
{
    if (input.empty() || fileToken.empty() || !onFile)
        return false;

    size_t cursor = 0;
    std::filesystem::path currentPath;
    size_t currentContentStart = std::string_view::npos;
    bool hasOpenBlock = false;

    while (cursor < input.size())
    {
        const size_t lineStart = cursor;
        size_t lineEnd = input.find('\n', cursor);
        if (lineEnd == std::string_view::npos)
            lineEnd = input.size();

        std::string_view line = input.substr(lineStart, lineEnd - lineStart);
        std::string_view trimmedLine = Trim(line);

        if (trimmedLine.starts_with(fileToken))
        {
            if (hasOpenBlock)
            {
                const size_t contentEnd = lineStart;
                std::string_view content = input.substr(currentContentStart, contentEnd - currentContentStart);

                if (!content.empty() && content.back() == '\r')
                    content.remove_suffix(1);

                if (!onFile(currentPath, content))
                    return false;
            }

            std::string_view remainder = Trim(trimmedLine.substr(fileToken.size()));
            if (remainder.empty())
                return false;

            currentPath = std::filesystem::path(std::string(remainder));
            currentContentStart = (lineEnd < input.size()) ? lineEnd + 1 : lineEnd;
            hasOpenBlock = true;
        }

        cursor = (lineEnd < input.size()) ? lineEnd + 1 : lineEnd;
    }

    if (!hasOpenBlock)
        return false;

    std::string_view content = input.substr(currentContentStart);
    if (!content.empty() && content.back() == '\r')
        content.remove_suffix(1);

    return onFile(currentPath, content);
}

bool TemplateProcessor::ProcessTemplateBlocks(std::string_view input, std::string_view fileToken, const TemplateContext& context,
    const FileCallback& onFile, const TemplateProcessorOptions& options)
{
    return ParseTemplateBlocks(
        input,
        fileToken,
        [&](const std::filesystem::path& path, std::string_view content) -> bool
        {
            const std::string processedPath = Process(path.string(), context, options);
            const std::string processedContent = Process(content, context, options);
            return onFile(std::filesystem::path(processedPath), processedContent);
        });
}

std::string TemplateProcessor::TrimCopy(std::string_view text)
{
    return std::string(Trim(text));
}

std::string_view TemplateProcessor::TrimLeft(std::string_view text)
{
    size_t i = 0;
    while (i < text.size() && std::isspace(static_cast<unsigned char>(text[i])))
        ++i;

    return text.substr(i);
}

std::string_view TemplateProcessor::TrimRight(std::string_view text)
{
    size_t i = text.size();
    while (i > 0 && std::isspace(static_cast<unsigned char>(text[i - 1])))
        --i;

    return text.substr(0, i);
}

std::string_view TemplateProcessor::Trim(std::string_view text)
{
    return TrimRight(TrimLeft(text));
}