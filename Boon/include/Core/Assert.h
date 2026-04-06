#pragma once

#include <string>
#include <cstdlib>

#ifdef _DEBUG
#define BN_ENABLE_ASSERT
#endif

namespace Boon
{
    /**
     * @brief Assertion failure hook.
     *
     * This function is called when an assertion (BN_ASSERT) fails. The default
     * implementation triggers a debug break. Consumers may replace or extend
     * this behavior by providing their own implementation.
     *
     * @param expr Textual representation of the failed expression.
     * @param msg Optional message provided to the assert macro.
     * @param file Source file where the assertion failed.
     * @param line Line number of the failure.
     */
    inline void BnAssertFail(const char* /*expr*/, const char* /*msg*/, const char* /*file*/, int /*line*/)
    {
        // You can replace this with your logger:
        // printf("ASSERT FAILED: %s | %s | %s:%d\n", expr, msg, file, line);
        __debugbreak(); // halt in debugger
    }
}

#ifdef BN_ENABLE_ASSERT

/**
 * @brief Assertion macro that invokes BnAssertFail on failure in debug builds.
 *
 * In release builds the macro is a no-op. The optional variadic parameter can
 * be used to supply an additional message forwarded to the failure hook.
 */
#define BN_ASSERT(condition, ...) \
        do { \
            if(!(condition)) { \
                ::Boon::BnAssertFail(#condition, ##__VA_ARGS__, __FILE__, __LINE__); \
            } \
        } while(0)

#else

#define BN_ASSERT(condition, ...) do {} while(0)

#endif
