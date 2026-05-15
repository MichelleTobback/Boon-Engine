#pragma once
#include "EditorCommand.h"
#include "Scene/Scene.h"
#include "Scene/GameObject.h"

using namespace Boon;

namespace BoonEditor
{
	template<typename TComp, typename TValue>
	class ComponentValueCommand final : public EditorCommand
	{
	public:
		using Func = std::function<void(TComp&, const TValue&)>;
		explicit ComponentValueCommand(GameObject obj, const TValue& originalValue, const TValue& newValue, Func fnSetter)
			: m_GameObject{ obj }, m_OriginalValue{ originalValue }, m_NewValue{ newValue }, m_FnSetter{fnSetter} { }
		virtual ~ComponentValueCommand() override = default;

		virtual void Execute() override
		{
			if (!m_GameObject.IsValid())
				return;

			if (!m_GameObject.HasComponent<TComp>())
				return;

			if (m_FnSetter)
				m_FnSetter(m_GameObject.GetComponent<TComp>(), m_NewValue);
		}

		virtual void Undo() override
		{
			if (!m_GameObject.IsValid())
				return;

			if (!m_GameObject.HasComponent<TComp>())
				return;

			if (m_FnSetter)
				m_FnSetter(m_GameObject.GetComponent<TComp>(), m_OriginalValue);
		}

	private:
		GameObject m_GameObject;
		TValue m_OriginalValue;
		TValue m_NewValue;
		Func m_FnSetter;
	};

	typedef ComponentValueCommand<TransformComponent, glm::vec3> TransformValueCommand;
}