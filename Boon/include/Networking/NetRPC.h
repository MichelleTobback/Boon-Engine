#pragma once
#include "Core/Variant.h"
#include "NetPacket.h"
#include "Core/UUID.h"
#include <vector>
#include <string>

namespace Boon
{
	class NetScene;

	class NetRPC final
	{
	public:
		explicit NetRPC(NetScene* scene);

		// Reflection typed RPC calls
		void CallServer(uint32_t classId, uint32_t fnId, const UUID& uuid, const std::vector<Variant>& args);

		void CallClient(uint64_t clientId, uint32_t classId, uint32_t fnId, const UUID& uuid, const std::vector<Variant>& args);

		// Called by NetScene after receiving an ENetPacketType::RPC
		void Process(const NetPacket& pkt, bool isServerSide);

	private:
		NetScene* m_Scene;
	};
}
