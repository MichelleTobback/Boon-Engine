#pragma once
#include "ObjectContext.h"

#include <Scene/Scene.h>
#include <Scene/GameObject.h>
#include <Asset/Asset.h>

using namespace Boon;

namespace BoonEditor
{
	using SceneContext = ObjectContext<Scene*>;
	using GameObjectContext = ObjectContext<GameObject>;
	using AssetContext = ObjectContext<AssetHandle>;

	enum class EditorPlayState
	{
		Edit, Play, Pause
	};
}