#pragma once
#include <functional>
#include <memory>

namespace Boon
{
	class Command
	{
	public:
		virtual ~Command() = default;
		virtual void Execute() = 0;
	};

	class ActionCommand final : public Command
	{
	public:
		explicit ActionCommand(const std::function<void()>& fnAction);
		virtual ~ActionCommand() override = default;

		virtual void Execute() override;

		template<typename Obj, typename Func> 
		inline static std::shared_ptr<ActionCommand> Create(Obj* t, Func&& fn) 
		{
			return std::make_shared<ActionCommand>( std::bind(fn, t) );
		}

	private:
		std::function<void()> m_FnAction;
	};

	template <typename T>
	class ValueCommand final : public Command
	{
	public:
		explicit ValueCommand(const std::function<void(T)>& fnAction)
			: m_FnAction{ fnAction }
		{
		}
		virtual ~ValueCommand() override = default;

		inline virtual void Execute() override
		{
			if (m_FnAction)
				m_FnAction(m_Value);
		}

		template<typename Obj, typename Func>
		inline static std::shared_ptr<ValueCommand<T>> Create(Obj* pObj, Func&& fn)
		{
			return std::make_shared<ValueCommand<T>>( std::bind(fn, pObj, std::placeholders::_1));
		}

		inline void SetValue(const T& value) { m_Value = value; }

	private:
		std::function<void(T)> m_FnAction;
		T m_Value{};
	};
}