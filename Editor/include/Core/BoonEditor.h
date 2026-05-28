#pragma once
#include "ObjectContext.h"

#include <Scene/Scene.h>
#include <Scene/GameObject.h>
#include <Asset/Asset.h>

namespace BoonEditor
{
	using SceneContext = ObjectContext<Boon::Scene*>;
	using GameObjectContext = ObjectContext<Boon::GameObject>;
	using AssetContext = ObjectContext<Boon::AssetHandle>;

	enum class EditorPlayState
	{
		Edit, Play, Pause
	};
}