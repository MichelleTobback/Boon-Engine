#pragma once
#include <cstdint>

namespace Boon
{
	using GamepadButtonCode = uint16_t;
	using GamepadAxisCode = uint16_t;

	namespace Gamepad
	{
		enum : GamepadButtonCode
		{
			ButtonA = 0,
			ButtonB = 1,
			ButtonX = 2,
			ButtonY = 3,
			ButtonLeftBumper = 4,
			ButtonRightBumper = 5,
			ButtonBack = 6,
			ButtonStart = 7,
			ButtonGuide = 8,
			ButtonLeftThumb = 9,
			ButtonRightThumb = 10,
			ButtonDPadUp = 11,
			ButtonDPadRight = 12,
			ButtonDPadDown = 13,
			ButtonDPadLeft = 14,

			ButtonLast = ButtonDPadLeft
		};

		enum : GamepadAxisCode
		{
			AxisLeftX = 0,
			AxisLeftY = 1,
			AxisRightX = 2,
			AxisRightY = 3,
			AxisLeftTrigger = 4,
			AxisRightTrigger = 5,

			AxisLast = AxisRightTrigger
		};
	}
}