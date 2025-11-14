#pragma once

#include <string>
#include <cstdlib>

#ifdef _DEBUG
#define BN_ENABLE_ASSERT
#endif

namespace Boon
{
    // Optional logging hook if you want printed output:
    inline void BnAssertFail(const char* /*expr*/, const char* /*msg*/, const char* /*file*/, int /*line*/)
    {
        // You can replace this with your logger:
        // printf("ASSERT FAILED: %s | %s | %s:%d\n", expr, msg, file, line);
        __debugbreak(); // halt in debugger
    }
}

#ifdef BN_ENABLE_ASSERT

#define BN_ASSERT(condition, ...) \
        do { \
            if(!(condition)) { \
                ::Boon::BnAssertFail(#condition, ##__VA_ARGS__, __FILE__, __LINE__); \
            } \
        } while(0)

#else

#define BN_ASSERT(condition, ...) do {} while(0)

#endif
