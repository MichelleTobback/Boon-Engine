#include "Core/Boon.h"

#include <Component/NameComponent.h>
#include <Component/SpriteAnimatorComponent.h>
#include <Component/SpriteRendererComponent.h>
#include <Component/Rigidbody2D.h>
#include <Component/BoxCollider2D.h>
#include <Networking/Components/NetRigidbody2D.h>
#include <Networking/Events/NetConnectionEvent.h>
#include <Networking/NetScene.h>
#include <Event/EventBus.h>
#include <Core/ServiceLocator.h>
#include "Game/PlayerController.h"
#include <Asset/Assets.h>

namespace Boon
{
	BCLASS(Name = "Player Start", Category = "Gameplay")
		class PlayerSpawn final
	{
		BCLASS_BODY()

	public:

		void Awake(GameObject obj)
		{
			if (obj.HasComponent<NetIdentity>() && !obj.GetComponent<NetIdentity>().IsAuthority())
			{
				return;
			}

			m_pScene = obj.GetComponent<NetIdentity>().pScene;

			EventBus& bus = ServiceLocator::Get<EventBus>();
			bus.Subscribe<NetConnectionEvent>([this](const NetConnectionEvent& e)
				{
					if (e.State == ENetConnectionState::Connected)
						SpawnPlayerServer(e.ConnectionId);
					else if (e.State == ENetConnectionState::Disconnected)
					{
						m_Instances[e.ConnectionId].Destroy();
						m_Instances.erase(e.ConnectionId);
					}
				});
		}

		void SpawnPlayer()
		{
			if (!m_pScene)
				return;

			SpawnPlayerServer(m_pScene->GetLocalConnectionID());
		}

		GameObject SpawnPlayerServer(uint64_t connectionId)
		{
			if (!m_pScene)
				return GameObject();

			GameObject player = m_pScene->InstantiateGameObject(connectionId);

			AssetLibrary& assetLib = Assets::Get();
			AssetRef<SpriteAtlasAsset> atlas = assetLib.Import<SpriteAtlasAsset>("game/Blue_witch/B_witch_atlas_compact.bsa");

			SpriteRendererComponent& sprite = player.AddComponent<SpriteRendererComponent>();
			sprite.SpriteAtlasHandle = atlas->GetHandle();
			sprite.Sprite = 0;

			SpriteAnimatorComponent& animator = player.AddComponent<SpriteAnimatorComponent>();
			animator.Clip = 0;
			animator.Atlas = atlas->GetInstance();
			animator.pRenderer = player;

			BoxCollider2D& col = player.AddComponent<BoxCollider2D>();
			col.Size = { 0.8f, 1.f };

			Rigidbody2D& rb = player.AddComponent<Rigidbody2D>();
			rb.Type = (int)Boon::Rigidbody2D::BodyType::Dynamic;
			rb.GravityScale = 0.f;
			rb.FixedRotation = true;

			player.AddComponent<PlayerController>();
			player.GetComponent<NameComponent>().Name = "Player";

			player.GetOrAddComponent<NetIdentity>();
			player.AddComponent<NetRigidbody2D>();

			m_Instances[connectionId] = player;

			return player;
		}

	private:
		NetScene* m_pScene{nullptr};
		std::unordered_map<uint64_t, GameObject> m_Instances;
	};
}