#pragma once

#include <xhash>

//https://www.youtube.com/watch?v=O_0nUE4S8T8

namespace Boon
{
	class UUID final
	{
	public:
		UUID();
		UUID(uint64_t uuid);

		bool IsValid() const;

		operator uint64_t() const { return m_Uuid; }

		static const UUID Null;

	private:
		uint64_t m_Uuid{};
	};
}

namespace std
{
	template<>
	struct hash<Boon::UUID>
	{
		std::size_t operator()(const Boon::UUID& uuid) const
		{
			return hash<uint64_t>()(static_cast<uint64_t>(uuid));
		}
	};
}