#pragma once
#include "EditorContext.h"

#include <Scene/Scene.h>
#include <Scene/GameObject.h>

using namespace Boon;

namespace BoonEditor
{
	using SceneContext = EditorContext<Scene*>;
	using GameObjectContext = EditorContext<GameObject>;

	enum class EditorPlayState
	{
		Edit, Play, Pause
	};
}