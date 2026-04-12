#pragma once
#include "Command/EditorCommand.h"
#include "Reflection/BProperty.h"
#include <Scene/GameObject.h>
#include <Core/Variant.h>

#include <memory>

using namespace Boon;

namespace BoonEditor
{
    template<typename T>
    class SetTypedComponentPropertyCommand final : public EditorCommand
    {
    public:
        SetTypedComponentPropertyCommand(GameObject obj, BClass* cls, BProperty* property, const T& newValue)
            : m_Obj(obj)
            , m_pCls(cls)
            , m_pProp(property)
            , m_OldValue()
            , m_NewValue(newValue)
            , m_OldSet(false)
        {
        }

        SetTypedComponentPropertyCommand(GameObject obj, BClass* cls, BProperty* property, const T& oldValue, const T& newValue)
            : m_Obj(obj)
            , m_pCls(cls)
            , m_pProp(property)
            , m_OldValue(oldValue)
            , m_NewValue(newValue)
            , m_OldSet(true)
        {
        }

        virtual ~SetTypedComponentPropertyCommand() = default;

        void Execute() override
        {
            if (!m_Obj.IsValid())
                return;

            if (!m_pCls || !m_pProp)
                return;

            void* pInstance = m_Obj.GetComponentByClass(m_pCls);

            if (!pInstance)
                return;

            T& ref = m_pCls->GetValueRef<T>(pInstance, m_pProp->name);

            if (!m_OldSet)
            {
                m_OldValue = ref;
                m_OldSet = true;
            }

            ref = m_NewValue;
        }

        void Undo() override
        {
            if (!m_Obj.IsValid())
                return;

            if (!m_pCls || !m_pProp)
                return;

            void* pInstance = m_Obj.GetComponentByClass(m_pCls);

            if (!pInstance)
                return;

            m_pCls->GetValueRef<T>(pInstance, m_pProp->name) = m_OldValue;
        }

    private:
        GameObject m_Obj;
        BClass* m_pCls;
        BProperty* m_pProp;
        T m_OldValue;
        T m_NewValue;
        bool m_OldSet;
    };

    class SetComponentPropertyCommand final : public EditorCommand
    {
    public:
        SetComponentPropertyCommand(GameObject obj, const BClass* cls, const BProperty* prop, const Variant& newValue)
            : m_Obj(obj)
            , m_pCls(cls)
            , m_pProp(prop)
            , m_NewValue(newValue)
            , m_OldSet(false)
        {
        }

        SetComponentPropertyCommand(GameObject obj, const BClass* cls, const BProperty* prop, const Variant& oldValue, const Variant& newValue)
            : m_Obj(obj)
            , m_pCls(cls)
            , m_pProp(prop)
            , m_OldValue(oldValue)
            , m_NewValue(newValue)
            , m_OldSet(true)
        {
        }

        ~SetComponentPropertyCommand() override = default;

        void Execute() override
        {
            Apply(true);
        }

        void Undo() override
        {
            if (!m_OldSet)
                return;

            Apply(false);
        }

    private:
        void Apply(bool useNewValue)
        {
            if (!m_Obj.IsValid() || !m_pCls || !m_pProp)
                return;

            void* pInstance = m_Obj.GetComponentByClass(m_pCls);
            if (!pInstance)
                return;

            switch (m_pProp->typeId)
            {
            case BTypeId::Int:    ApplyByType<int32_t>(pInstance, useNewValue); break;
            case BTypeId::Int64:  ApplyByType<int64_t>(pInstance, useNewValue); break;
            case BTypeId::Uint:   ApplyByType<uint32_t>(pInstance, useNewValue); break;
            case BTypeId::Uint64: ApplyByType<uint64_t>(pInstance, useNewValue); break;
            case BTypeId::Float:  ApplyByType<float>(pInstance, useNewValue); break;
            case BTypeId::Bool:   ApplyByType<bool>(pInstance, useNewValue); break;
            case BTypeId::Float2: ApplyByType<glm::vec2>(pInstance, useNewValue); break;
            case BTypeId::Float3: ApplyByType<glm::vec3>(pInstance, useNewValue); break;
            case BTypeId::Float4: ApplyByType<glm::vec4>(pInstance, useNewValue); break;
            case BTypeId::Int2:   ApplyByType<glm::ivec2>(pInstance, useNewValue); break;
            case BTypeId::Int3:   ApplyByType<glm::ivec3>(pInstance, useNewValue); break;
            case BTypeId::Int4:   ApplyByType<glm::ivec4>(pInstance, useNewValue); break;
            case BTypeId::String: ApplyByType<std::string>(pInstance, useNewValue); break;
            default:
                break;
            }
        }

        template <typename T>
        void ApplyByType(void* pInstance, bool useNewValue)
        {
            T& ref = m_pCls->GetValueRef<T>(pInstance, m_pProp->name);

            if (useNewValue)
            {
                if (!m_OldSet)
                {
                    m_OldValue.Set(ref);
                    m_OldSet = true;
                }

                ref = m_NewValue.As<T>();
            }
            else
            {
                ref = m_OldValue.As<T>();
            }
        }

    private:
        GameObject m_Obj;
        const BClass* m_pCls{};
        const BProperty* m_pProp{};
        Variant m_OldValue{};
        Variant m_NewValue{};
        bool m_OldSet{};
    };
}