#pragma once

#include <xhash>

//https://www.youtube.com/watch?v=O_0nUE4S8T8

namespace Boon
{
	class UUID final
	{
	public:
		/**
		 * @brief Create a new (random) UUID.
		 *
		 * The default constructor generates a new unique identifier.
		 */
		UUID();

		/**
		 * @brief Construct a UUID from an existing 64-bit value.
		 * @param uuid Raw 64-bit identifier value.
		 */
		UUID(uint64_t uuid);

		/**
		 * @brief Check whether the UUID is valid (non-null).
		 * @return True when the UUID contains a non-zero value.
		 */
		bool IsValid() const;

		/**
		 * @brief Convert to the underlying 64-bit representation.
		 */
		operator uint64_t() const { return m_Uuid; }

		/**
		 * @brief A sentinel null UUID value.
		 */
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