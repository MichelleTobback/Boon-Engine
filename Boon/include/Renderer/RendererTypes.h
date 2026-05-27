#pragma once

#include <cstdint>

namespace Boon
{
	enum class BlendMode
	{
		None,
		Alpha,
		Additive
	};

	enum class DepthMode
	{
		Disabled,
		Read,
		ReadWrite
	};

	enum class CullMode
	{
		None,
		Back,
		Front
	};

	enum class PrimitiveType
	{
		Triangles,
		Lines
	};

	using RenderPhaseID = int32_t;

	namespace RenderPhases
	{
		constexpr RenderPhaseID Background = 1000;
		constexpr RenderPhaseID Opaque = 2000;
		constexpr RenderPhaseID Transparent = 3000;
		constexpr RenderPhaseID Overlay = 4000;
		constexpr RenderPhaseID UI = 5000;
		constexpr RenderPhaseID Debug = 6000;
	}
}