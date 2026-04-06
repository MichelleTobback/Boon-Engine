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
		/**
		 * @brief Issue an RPC from client to server targeting the object identified by uuid.
		 *
		 * @param classId Reflected class id.
		 * @param fnId Reflected function id.
		 * @param uuid Target object uuid.
		 * @param args Arguments for the RPC encoded as Variants.
		 */
		void CallServer(uint32_t classId, uint32_t fnId, const UUID& uuid, const std::vector<Variant>& args);

		/**
		 * @brief Issue an RPC from server to a particular client.
		 */
		void CallClient(uint64_t clientId, uint32_t classId, uint32_t fnId, const UUID& uuid, const std::vector<Variant>& args);

		/**
		 * @brief Process an incoming RPC packet.
		 *
		 * @param pkt Packet containing RPC data.
		 * @param isServerSide true when processing on the server side.
		 */
		void Process(const NetPacket& pkt, bool isServerSide);

	private:
		NetScene* m_Scene;
	};
}
