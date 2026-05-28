#pragma once
#include <Core/SubsystemRegistry.h>

namespace Boon
{
	class Window;
	class Input;
	class AssetLibrary;
	class SceneManager;
	class EventBus;
	class Time;

	struct EngineContext
	{
		Window* Window = nullptr;
		Input* Input = nullptr;
		AssetLibrary* AssetLib = nullptr;
		SceneManager* Scenes = nullptr;
		EventBus* EventBus = nullptr;
		Time* Time = nullptr;

		SubsystemRegistry* Subsystems = nullptr;

		template<typename T>
		T& GetSubsystem()
		{
			return Subsystems->Get<T>();
		}

		template<typename T>
		T* TryGetSubsystem()
		{
			return Subsystems->TryGet<T>();
		}
	};
}