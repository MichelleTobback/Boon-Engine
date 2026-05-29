#include "Process/ProcessRunner.h"

#if defined(_WIN32)
	#include "Platform/Windows/WindowsProcessRunner.inl"

#elif defined(__linux__)
	#include "Platform/Linux/LinuxProcessRunner.inl"

#else
	#error Unsupported platform
#endif