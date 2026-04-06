#pragma once

namespace Boon
{
	/**
	 * @brief Utilities for manipulating bitflag-style enum values.
	 *
	 * These helpers perform simple integer operations on enum-backed bitfields.
	 */
	class BitFlag final
	{
	public:
		/**
		 * @brief Test whether a specific flag bit is set.
		 *
		 * @tparam T Enum or integer type representing flags.
		 * @param bitFlags Current flag value.
		 * @param flag Flag bit to test.
		 * @return true if the bit is set (non-zero), false otherwise.
		 */
		template<typename T>
		static bool IsSet(T bitFlags, T flag);

		/**
		 * @brief Set or clear a specific flag bit.
		 *
		 * @param bitFlags Reference to the flag value to modify.
		 * @param flag Flag bit to set or clear.
		 * @param set If true the flag bit is set; if false it is cleared.
		 */
		template<typename T>
		static void Set(T& bitFlags, T flag, bool set);

		/**
		 * @brief Clear all bits in the provided flag value.
		 */
		template<typename T>
		static void ClearAll(T& flags);

		/**
		 * @brief Set all bits in the provided flag value.
		 */
		template<typename T>
		static void SetAll(T& flags);

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
	template<typename T>
	static void ClearAll(T& flags)
	{
		flags = 0;
	}

	template<typename T>
	static void SetAll(T& flags)
	{
		flags = 0xFFFFFFFFu;
	}
}