#pragma once
#include <Core/Delegate.h>

using namespace Boon;

namespace BoonEditor
{
	template <typename T>
	class EditorContext final
	{
	public:
		using Signature = void(T&);
		using Callback = std::function<Signature>;

		EditorContext() = default;
		EditorContext(const T& context);
		~EditorContext() = default;

		T& Get();
		const T& Get() const;
		void Set(const T& context);
		void AddOnContextChangedCallback(const Callback& fn);

		T& overator();
		const T& overator() const;

	private:
		T m_pContext{};
		Delegate<Signature> m_OnContextChanged;
	};

	template<typename T>
	inline EditorContext<T>::EditorContext(const T& context)
		: m_pContext{ context }, m_OnContextChanged{}
	{
	}
	template<typename T>
	inline T& EditorContext<T>::Get()
	{
		return m_pContext;
	}
	template<typename T>
	inline const T& EditorContext<T>::Get() const
	{
		return m_pContext;
	}
	template<typename T>
	inline void EditorContext<T>::Set(const T& context)
	{
		m_pContext = context;
		m_OnContextChanged.Invoke(m_pContext);
	}
	template<typename T>
	inline void EditorContext<T>::AddOnContextChangedCallback(const Callback& fn)
	{
		m_OnContextChanged += fn;
	}
	template<typename T>
	inline T& EditorContext<T>::overator()
	{
		return m_pContext;
	}
	template<typename T>
	inline const T& EditorContext<T>::overator() const
	{
		return m_pContext;
	}
}