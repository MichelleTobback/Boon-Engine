#pragma once

namespace Boon
{
	class Window;
	class Input;
	class AssetLibrary;
	class SceneManager;
	class EventBus;

	struct EngineContext
	{
		Window* Window = nullptr;
		Input* Input = nullptr;
		AssetLibrary* AssetLib = nullptr;
		SceneManager* Scenes = nullptr;
		EventBus* EventBus = nullptr;
	};
}