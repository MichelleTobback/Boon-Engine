#pragma once
#include <Core/Delegate.h>

using namespace Boon;

namespace BoonEditor
{
	template <typename T>
	class ObjectContext final
	{
	public:
		using Signature = void(T&);
		using Callback = std::function<Signature>;

		ObjectContext() = default;
		ObjectContext(const T& context);
		~ObjectContext() = default;

		T& Get();
		const T& Get() const;
		void Set(const T& context);
		void AddOnContextChangedCallback(const Callback& fn);

		bool IsValid() const;

		T& overator();
		const T& overator() const;

	private:
		T m_pContext{};
		Delegate<Signature> m_OnContextChanged;
	};

	template<typename T>
	inline ObjectContext<T>::ObjectContext(const T& context)
		: m_pContext{ context }, m_OnContextChanged{}
	{
	}
	template<typename T>
	inline T& ObjectContext<T>::Get()
	{
		return m_pContext;
	}
	template<typename T>
	inline const T& ObjectContext<T>::Get() const
	{
		return m_pContext;
	}
	template<typename T>
	inline void ObjectContext<T>::Set(const T& context)
	{
		m_pContext = context;
		m_OnContextChanged.Invoke(m_pContext);
	}
	template<typename T>
	inline void ObjectContext<T>::AddOnContextChangedCallback(const Callback& fn)
	{
		m_OnContextChanged += fn;
	}
	template<typename T>
	inline T& ObjectContext<T>::overator()
	{
		return m_pContext;
	}
	template<typename T>
	inline const T& ObjectContext<T>::overator() const
	{
		return m_pContext;
	}

	template<typename T>
	bool ObjectContext<T>::IsValid() const
	{
		return m_pContext;
	}
}