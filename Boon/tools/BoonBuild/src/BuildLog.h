#pragma once

#include <filesystem>
#include <iostream>
#include <string>

namespace BoonBuild
{
    enum class LogLevel
    {
        Info,
        Success,
        Warning,
        Error
    };

    inline const char* ToString(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::Info:    return "[INFO] ";
        case LogLevel::Success: return "[ OK ] ";
        case LogLevel::Warning: return "[WARN] ";
        case LogLevel::Error:   return "[ERR ] ";
        }

        return "[LOG ] ";
    }

    inline void Log(LogLevel level, const std::string& message)
    {
        std::ostream& out = level == LogLevel::Error ? std::cerr : std::cout;
        out << ToString(level) << message << '\n';
    }

    inline void Info(const std::string& message)
    {
        Log(LogLevel::Info, message);
    }

    inline void Success(const std::string& message)
    {
        Log(LogLevel::Success, message);
    }

    inline void Warning(const std::string& message)
    {
        Log(LogLevel::Warning, message);
    }

    inline void Error(const std::string& message)
    {
        Log(LogLevel::Error, message);
    }

    inline std::string PathString(const std::filesystem::path& path)
    {
        return path.lexically_normal().string();
    }
}