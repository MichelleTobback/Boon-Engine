#include "Delegate.h"

namespace Boon
{
    template<typename T>
    class ObservableType
    {
    public:
        using OnValueChangedDelegate = Delegate<void(T)>;
        ObservableType(T value = T{})
            : m_Value{ value }
            , m_pOnValueChangedDelegate{ std::make_unique<OnValueChangedDelegate>() }
        {

        }

        void operator+=(const T& right)
        {
            m_Value += right;
            m_pOnValueChangedDelegate->Invoke(m_Value);
        }
        void operator-=(const T& right)
        {
            m_Value -= right;
            m_pOnValueChangedDelegate->Invoke(m_Value);
        }
        void operator*=(const T& right)
        {
            m_Value *= right;
            m_pOnValueChangedDelegate->Invoke(m_Value);
        }
        void operator/=(const T& right)
        {
            m_Value /= right;
            m_pOnValueChangedDelegate->Invoke(m_Value);
        }
        void operator=(const T& right)
        {
            m_Value = right;
            m_pOnValueChangedDelegate->Invoke(m_Value);
        }
        T& operator--()
        {
            --m_Value;
            m_pOnValueChangedDelegate->Invoke(m_Value);
            return m_Value;
        }
        T& operator++()
        {
            ++m_Value;
            m_pOnValueChangedDelegate->Invoke(m_Value);
            return m_Value;
        }

        const T& operator()() const { return m_Value; }

        OnValueChangedDelegate& GetOnValueChangedDelegate() { return *m_pOnValueChangedDelegate; }

    private:
        T m_Value{};
        std::unique_ptr<OnValueChangedDelegate> m_pOnValueChangedDelegate{};
    };
}