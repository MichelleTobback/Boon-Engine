#pragma once

namespace Boon
{
	class AppState
	{
	public:
		virtual void OnEnter() = 0;
		virtual void OnUpdate() = 0;
		virtual void OnExit() = 0;
	};
}