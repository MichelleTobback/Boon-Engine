#pragma once
namespace Boon
{
	template <typename T>
	class Singleton
	{
	public:
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