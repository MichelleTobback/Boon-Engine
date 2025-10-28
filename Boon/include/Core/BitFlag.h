#pragma once

namespace Boon
{
	class BitFlag final
	{
	public:
		template<typename T>
		static bool IsSet(T bitFlags, T flag);
		template<typename T>
		static void Set(T& bitFlags, T flag, bool set);

	private:
		BitFlag() = default;
	};
	template<typename T>
	inline bool BitFlag::IsSet(T bitFlags, T flag)
	{
		return (static_cast<int>(bitFlags) & static_cast<int>(flag));
	}
	template<typename T>
	inline void BitFlag::Set(T& bitFlags, T flag, bool set)
	{
		bitFlags = (set)
			? static_cast<T>(static_cast<int>(bitFlags) | static_cast<int>(flag))
			: static_cast<T>(static_cast<int>(bitFlags) & ~static_cast<int>(flag));
	}
}