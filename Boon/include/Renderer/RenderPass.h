#pragma once

#include "Renderer/RenderContext.h"

namespace Boon
{
	class RenderPass
	{
	public:
		virtual ~RenderPass() = default;

		virtual void Execute(RenderContext& context) = 0;
	};
}