#pragma once
#include "EditorContext.h"

#include <Scene/Scene.h>
#include <Scene/GameObject.h>
#include <Asset/Asset.h>

using namespace Boon;

namespace BoonEditor
{
	using SceneContext = EditorContext<Scene*>;
	using GameObjectContext = EditorContext<GameObject>;
	using AssetContext = EditorContext<AssetHandle>;

	enum class EditorPlayState
	{
		Edit, Play, Pause
	};
}