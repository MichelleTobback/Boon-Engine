#pragma once

#ifdef _WIN32
#ifdef SandboxGame_EXPORTS
#define SANDBOX_API __declspec(dllexport)
#else
#define SANDBOX_API __declspec(dllimport)
#endif
#else
#define SANDBOX_API
#endif