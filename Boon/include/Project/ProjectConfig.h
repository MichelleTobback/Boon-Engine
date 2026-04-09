#pragma once
#include "RuntimeConfig.h"

namespace Boon
{
	struct ProjectConfig
	{
		std::string Name;
		uint32_t Version = 1;

		RuntimeConfig Runtime{};

		struct EditorConfig
		{
			std::filesystem::path EditorResourcesRoot;
		} Editor;
	};
}