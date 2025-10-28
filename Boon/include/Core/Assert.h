#pragma once

#ifdef BN_DEBUG
	#define NV_ENABLE_ASSERT
#endif

#ifdef BN_ENABLE_ASSERT
#include <string>
#include <cassert>
	namespace Boon
	{
		void BnAssert(bool condition, const std::string& message, const char* file, int line);
	}

	#define BN_ASSERT(condition, ...) assert(condition && #__VA_ARGS__) 
	//{NvAssert(condition, #__VA_ARGS__, __FILE__, __LINE__);}
#else
	#define BN_ASSERT(condition, ...)
#endif
