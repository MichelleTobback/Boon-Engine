#pragma once
#include "NetPacket.h"
#include "Core/Variant.h"

#include <vector>
#include <string>

namespace Boon
{
	class NetScene;
	class NetRPC final
	{
	public:
		explicit NetRPC(NetScene* scene);

		void CallServer(const std::string& fn, const std::vector<Variant>& args);
		void CallClient(uint64_t clientId, const std::string& fn, const std::vector<Variant>& args);

		void Process(const NetPacket& pkt);

	private:
		NetScene* m_Scene;
	};
}