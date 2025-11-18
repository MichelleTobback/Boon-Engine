#pragma once
#include <string>
#include <cstdint>
#include <cstring>
#include <type_traits>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_precision.hpp>

#include "Core/UUID.h"

namespace Boon
{
    enum class VariantType
    {
        None,
        Bool,
        Int,
        UInt,
        Int64,
        UInt64,
        Float,
        Double,
        String,
        Pointer,

        Vec2,
        Vec3,
        Vec4,
        IVec2,
        IVec3,
        IVec4,

        UUID
    };

    class Variant
    {
    public:
        Variant() : m_Type(VariantType::None) {}

        // ---------------------------
        // Standard constructors
        // ---------------------------
        Variant(bool v) { Set(v); }
        Variant(int v) { Set(v); }
        Variant(uint32_t v) { Set(v); }
        Variant(int64_t v) { Set(v); }
        Variant(uint64_t v) { Set(v); }
        Variant(float v) { Set(v); }
        Variant(double v) { Set(v); }
        Variant(const char* v) { Set(std::string(v)); }
        Variant(const std::string& v) { Set(v); }
        Variant(void* ptr) { Set(ptr); }

        Variant(const glm::vec2& v) { Set(v); }
        Variant(const glm::vec3& v) { Set(v); }
        Variant(const glm::vec4& v) { Set(v); }
        Variant(const glm::ivec2& v) { Set(v); }
        Variant(const glm::ivec3& v) { Set(v); }
        Variant(const glm::ivec4& v) { Set(v); }

        Variant(const UUID& id) { Set(id); }

        VariantType GetType() const { return m_Type; }

        // ---------------------------
        // Getter API
        // ---------------------------
        bool AsBool() const { return m_Bool; }
        int AsInt() const { return m_Int; }
        uint32_t AsUInt() const { return m_UInt; }
        int64_t AsInt64() const { return m_Int64; }
        uint64_t AsUInt64() const { return m_UInt64; }
        float AsFloat() const { return m_Float; }
        double AsDouble() const { return m_Double; }
        const std::string& AsString() const { return m_String; }
        void* AsPointer() const { return m_Pointer; }

        glm::vec2 AsVec2() const { return m_Vec2; }
        glm::vec3 AsVec3() const { return m_Vec3; }
        glm::vec4 AsVec4() const { return m_Vec4; }

        glm::ivec2 AsIVec2() const { return m_IVec2; }
        glm::ivec3 AsIVec3() const { return m_IVec3; }
        glm::ivec4 AsIVec4() const { return m_IVec4; }

        UUID AsUUID() const { return (UUID)m_UInt64; }

        // ---------------------------
        // Setter API
        // ---------------------------
        void Set(bool v) { m_Type = VariantType::Bool;   m_Bool = v; }
        void Set(int v) { m_Type = VariantType::Int;    m_Int = v; }
        void Set(uint32_t v) { m_Type = VariantType::UInt;   m_UInt = v; }
        void Set(int64_t v) { m_Type = VariantType::Int64;  m_Int64 = v; }
        void Set(uint64_t v) { m_Type = VariantType::UInt64; m_UInt64 = v; }
        void Set(float v) { m_Type = VariantType::Float;  m_Float = v; }
        void Set(double v) { m_Type = VariantType::Double; m_Double = v; }

        void Set(const std::string& v)
        {
            m_Type = VariantType::String;
            m_String = v;
        }

        void Set(void* ptr)
        {
            m_Type = VariantType::Pointer;
            m_Pointer = ptr;
        }

        void Set(const glm::vec2& v) { m_Type = VariantType::Vec2;  m_Vec2 = v; }
        void Set(const glm::vec3& v) { m_Type = VariantType::Vec3;  m_Vec3 = v; }
        void Set(const glm::vec4& v) { m_Type = VariantType::Vec4;  m_Vec4 = v; }
        void Set(const glm::ivec2& v) { m_Type = VariantType::IVec2; m_IVec2 = v; }
        void Set(const glm::ivec3& v) { m_Type = VariantType::IVec3; m_IVec3 = v; }
        void Set(const glm::ivec4& v) { m_Type = VariantType::IVec4; m_IVec4 = v; }

        void Set(const UUID& id)
        {
            m_Type = VariantType::UUID;
            m_UInt64 = (uint64_t)id;
        }

        // ---------------------------
        // Generic templated getter
        // ---------------------------
        template<typename T>
        T As() const
        {
            if constexpr (std::is_same_v<T, bool>) return AsBool();
            else if constexpr (std::is_same_v<T, int>) return AsInt();
            else if constexpr (std::is_same_v<T, uint32_t>) return AsUInt();
            else if constexpr (std::is_same_v<T, int64_t>) return AsInt64();
            else if constexpr (std::is_same_v<T, uint64_t>) return AsUInt64();
            else if constexpr (std::is_same_v<T, float>) return AsFloat();
            else if constexpr (std::is_same_v<T, double>) return AsDouble();
            else if constexpr (std::is_same_v<T, std::string>) return AsString();
            else if constexpr (std::is_same_v<T, void*>) return AsPointer();

            else if constexpr (std::is_same_v<T, glm::vec2>) return AsVec2();
            else if constexpr (std::is_same_v<T, glm::vec3>) return AsVec3();
            else if constexpr (std::is_same_v<T, glm::vec4>) return AsVec4();

            else if constexpr (std::is_same_v<T, glm::ivec2>) return AsIVec2();
            else if constexpr (std::is_same_v<T, glm::ivec3>) return AsIVec3();
            else if constexpr (std::is_same_v<T, glm::ivec4>) return AsIVec4();

            else if constexpr (std::is_same_v<T, UUID>) return AsUUID();

            else
            {
                static_assert(sizeof(T) == 0, "Variant::As<T> - Unsupported type");
            }
        }

        // ---------------------------
        // Generic templated setter
        // ---------------------------
        template<typename T>
        void Set(const T& v)
        {
            if constexpr (std::is_same_v<T, bool>) Set((bool)v);
            else if constexpr (std::is_same_v<T, int>) Set((int)v);
            else if constexpr (std::is_same_v<T, uint32_t>) Set((uint32_t)v);
            else if constexpr (std::is_same_v<T, int64_t>) Set((int64_t)v);
            else if constexpr (std::is_same_v<T, uint64_t>) Set((uint64_t)v);
            else if constexpr (std::is_same_v<T, float>) Set((float)v);
            else if constexpr (std::is_same_v<T, double>) Set((double)v);
            else if constexpr (std::is_same_v<T, std::string>) Set((const std::string&)v);
            else if constexpr (std::is_same_v<T, void*>) Set((void*)v);

            else if constexpr (std::is_same_v<T, glm::vec2>) Set((glm::vec2)v);
            else if constexpr (std::is_same_v<T, glm::vec3>) Set((glm::vec3)v);
            else if constexpr (std::is_same_v<T, glm::vec4>) Set((glm::vec4)v);

            else if constexpr (std::is_same_v<T, glm::ivec2>) Set((glm::ivec2)v);
            else if constexpr (std::is_same_v<T, glm::ivec3>) Set((glm::ivec3)v);
            else if constexpr (std::is_same_v<T, glm::ivec4>) Set((glm::ivec4)v);

            else if constexpr (std::is_same_v<T, UUID>) Set((UUID)v);

            else
            {
                static_assert(sizeof(T) == 0, "Variant::Set<T> - Unsupported type");
            }
        }

    private:
        VariantType m_Type;

        union
        {
            bool        m_Bool;
            int         m_Int;
            uint32_t    m_UInt;
            int64_t     m_Int64;
            uint64_t    m_UInt64;
            float       m_Float;
            double      m_Double;
            void*       m_Pointer;

            glm::vec2   m_Vec2;
            glm::vec3   m_Vec3;
            glm::vec4   m_Vec4;

            glm::ivec2  m_IVec2;
            glm::ivec3  m_IVec3;
            glm::ivec4  m_IVec4;
        };

        std::string m_String;
    };
}
