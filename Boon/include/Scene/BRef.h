#pragma once
#include "Scene/GameObject.h"
#include "Reflection/BClass.h"

namespace Boon
{
    class BRefBase
    {
    public:
        BRefBase(GameObject owner, const BClass* cls)
            : m_Object(owner), m_Type(cls) { }
        virtual ~BRefBase() = default;

        const BClass* Class() const { return m_Type; }
        GameObject Owner() const { return m_Object; }
        void Set(GameObject owner) { m_Object = owner; }

        BRefBase& operator=(const BRefBase& other)
        {
            if (this == &other)
                return *this;

            m_Object = other.m_Object;
            m_Type = other.m_Type;
            return *this;
        }

    protected:
        GameObject m_Object;
        const BClass* m_Type;
    };

    template<typename T>
	class BRef final : public BRefBase
	{
    public:
        BRef() 
            : BRefBase(GameObject(), BClassRegistry::Get().Find<T>()){}
        BRef(GameObject owner)
            : BRefBase(owner, BClassRegistry::Get().Find<T>()) { }
        virtual ~BRef() = default;

        BRef<T>& operator=(const BRef<T>& other)
        {
            if (this == &other)
                return *this;

            m_Object = other.m_Object;
            m_Type = other.m_Type;
            return *this;
        }

        T* operator->()
        {
            return Get();
        }

        T* Get()
        {
            if (!m_Object.IsValid() || !m_Type)
                return nullptr;

            return reinterpret_cast<T*>(m_Type->getComponent(m_Object));
        }

        void* Raw() const
        {
            if (!m_Object.IsValid() || !m_Type) return nullptr;
            if (!m_Type->hasComponent(m_Object)) return nullptr;
            return m_Type->getComponent(m_Object);
        }

        bool IsValid()
        {
            return m_Object.IsValid() && m_Type && m_Object.HasComponentByClass(m_Type);
        }
	};
}