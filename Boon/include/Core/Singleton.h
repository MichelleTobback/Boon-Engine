#pragma once
namespace Boon
{
	template <typename T>
	class Singleton
	{
	public:
		/**
		 * @brief Access the singleton instance for type T.
		 *
		 * Returns a reference to a function-local static instance. The instance
		 * is constructed on first use.
		 *
		 * @return Reference to the singleton instance of T.
		 */
		static T& Get()
		{
			static T pInstance{};
			return pInstance;
		}

		virtual ~Singleton() = default;
		Singleton(const Singleton& other) = delete;
		Singleton(Singleton&& other) = delete;
		Singleton& operator=(const Singleton& other) = delete;
		Singleton& operator=(Singleton&& other) = delete;

	protected:
		Singleton() = default;
	};
}