#pragma once
#include <vector>
#include <functional>
#include <memory>

namespace Boon
{
    template<typename Signature>
    class Delegate;

    template<typename T, typename... Args>
    class Delegate<T(Args...)>
    {
    public:
        void operator+=(const std::function<T(Args...)>& func)
        {
            m_Functions.emplace_back(func);
        }

        void operator-=(const std::function<T(Args...)>& func)
        {
            auto it{ std::find_if(m_Functions.begin(), m_Functions.end(), [&func](const auto& f)
                {
                    return f.target_type() == func.target_type();
                }) };

            if (it != m_Functions.end())
            {
                m_Functions.erase(it);
            }
        }

        void Invoke(Args... args) const
        {
            for (const auto& func : m_Functions)
            {
                func(args...);
            }
        }

        void Clear();

    private:
        std::vector<std::function<T(Args...)>> m_Functions;
    };

    template<typename T, typename ...Args>
    inline void Delegate<T(Args...)>::Clear()
    {
        m_Functions.clear();
    }
}
