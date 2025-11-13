#pragma once
#include <string>
#include <cstdint>
#include <cstring>
#include <type_traits>

namespace Boon
{
    enum class VariantType
    {
        None,
        Bool,
        Int,
        UInt,
        Float,
        Double,
        String,
        Pointer,     // for components, objects, etc.
    };

    class Variant
    {
    public:
        Variant() : m_Type(VariantType::None) {}

        Variant(bool v) { Set(v); }
        Variant(int v) { Set(v); }
        Variant(uint32_t v) { Set(v); }
        Variant(float v) { Set(v); }
        Variant(double v) { Set(v); }
        Variant(const char* v) { Set(std::string(v)); }
        Variant(const std::string& v) { Set(v); }
        Variant(void* ptr) { Set(ptr); }

        VariantType GetType() const { return m_Type; }

        // ---------------------------
        // Getters
        // ---------------------------
        bool AsBool() const { return m_Bool; }
        int AsInt() const { return m_Int; }
        uint32_t AsUInt() const { return m_UInt; }
        float AsFloat() const { return m_Float; }
        double AsDouble() const { return m_Double; }
        const std::string& AsString() const { return m_String; }
        void* AsPointer() const { return m_Pointer; }

        // ---------------------------
        // Setters
        // ---------------------------
        void Set(bool v) { m_Type = VariantType::Bool;   m_Bool = v; }
        void Set(int v) { m_Type = VariantType::Int;    m_Int = v; }
        void Set(uint32_t v) { m_Type = VariantType::UInt;   m_UInt = v; }
        void Set(float v) { m_Type = VariantType::Float;  m_Float = v; }
        void Set(double v) { m_Type = VariantType::Double; m_Double = v; }
        void Set(const std::string& v) { m_Type = VariantType::String; m_String = v; }
        void Set(void* ptr) { m_Type = VariantType::Pointer; m_Pointer = ptr; }

    private:
        VariantType m_Type;

        union
        {
            bool m_Bool;
            int m_Int;
            uint32_t m_UInt;
            float m_Float;
            double m_Double;
            void* m_Pointer;
        };

        std::string m_String;
    };
}
