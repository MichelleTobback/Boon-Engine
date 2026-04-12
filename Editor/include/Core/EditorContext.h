#pragma once
#include <Project/ProjectConfig.h>
#include <Panels/EditorWidget.h>

#include <memory>
#include <type_traits>
#include <vector>
#include <unordered_map>

using namespace Boon;

namespace BoonEditor
{
	class EditorCommandQueue;
	class EditorContext final
	{
	public:
		EditorContext();

		template <typename T, typename ...TArgs>
		T& CreateObject(TArgs&& ... args)
		{
			static_assert(std::is_base_of<EditorObject, T>::value, "T must derive from Object");

			auto pInstance = std::make_unique<T>(std::forward<TArgs>(args)...);
			T& ref = *pInstance;
			m_Objects.push_back(std::move(pInstance));
			return ref;
		}

		template <typename T, typename... TArgs>
		T& CreateWidget(const std::string& name, TArgs&&... args)
		{
			static_assert(std::is_base_of_v<EditorWidget, T>, "T must derive from EditorWidget");

			auto it = m_Widgets.find(name);
			if (it != m_Widgets.end())
				throw std::runtime_error("Widget with name already exists: " + name);

			T& ref = CreateObject<T>(name, this, std::forward<TArgs>(args)...);
			m_Widgets.emplace(name, &ref);
			return ref;
		}

		template <typename T>
		T& GetWidget(const std::string& name)
		{
			static_assert(std::is_base_of_v<EditorWidget, T>, "T must derive from EditorWidget");

			auto it = m_Widgets.find(name);
			if (it == m_Widgets.end())
				throw std::runtime_error("Widget not found: " + name);

			T* pWidget = dynamic_cast<T*>(it->second);
			if (!pWidget)
				throw std::runtime_error("Widget type mismatch for: " + name);

			return *pWidget;
		}

		template <typename T>
		T* TryGetWidget(const std::string& name)
		{
			static_assert(std::is_base_of_v<EditorWidget, T>, "T must derive from EditorWidget");

			auto it = m_Widgets.find(name);
			if (it == m_Widgets.end())
				return nullptr;

			return dynamic_cast<T*>(it->second);
		}

		inline EditorCommandQueue* GetCommandQueue() { return m_CommandQueue.get(); }
		inline const ProjectConfig& GetCurrentProjectConfig() const { return m_CurrentProject; }

	private:
		friend class EditorState;
		ProjectConfig m_CurrentProject{};

		std::vector<std::unique_ptr<EditorObject>> m_Objects;
		std::unordered_map<std::string, EditorWidget*> m_Widgets;
		std::unique_ptr<EditorCommandQueue> m_CommandQueue;
	};
}