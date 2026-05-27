#pragma once

#include "Renderer/RenderContext.h"

namespace Boon
{
	class RenderPass
	{
	public:
		virtual ~RenderPass() = default;

		virtual RenderPhaseID GetPhase() const
		{
			return RenderPhases::Opaque;
		}

		virtual int GetOrder() const
		{
			return 0;
		}

		virtual void Execute(RenderContext& context) = 0;
	};
}