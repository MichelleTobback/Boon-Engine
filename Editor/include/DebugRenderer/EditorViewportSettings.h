#pragma once
#include "DebugRenderLayers.h"

namespace BoonEditor
{
	struct EditorViewportSettings
	{
		DebugRenderLayer DebugRenderLayers{ DebugRenderLayer::All };
	};
}