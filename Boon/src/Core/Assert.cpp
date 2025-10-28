#include "Assert.h"

#ifdef BN_ENABLE_ASSERT
    #include <iostream>
    #include <stdexcept>
    void Boon::BnAssert(bool condition, const std::string& message, const char* file, int line)
    {
        if (!condition) 
        {
            std::cerr << "Assertion failed in " << file << " at line " << line << ": " << message << "\n";
            throw std::runtime_error("Assertion failed");
        }
    }
#endif