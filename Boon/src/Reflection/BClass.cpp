#include "Reflection/BClass.h"

Boon::BClassRegistry* Boon::BClassRegistry::s_Instance{};

Boon::Variant Boon::BClass::GetValue(const void* instance, const char* propName) const
{
    const BProperty* prop = FindProperty(propName);
    if (!prop)
        throw std::runtime_error("Property not found: " + std::string(propName));

    const uint8_t* base = reinterpret_cast<const uint8_t*>(instance);
    const void* pValue = base + prop->offset;

    Variant result;

    switch (prop->typeId)
    {
    case BTypeId::Int:
        result.Set(*reinterpret_cast<const int32_t*>(pValue));
        break;

    case BTypeId::Int64:
        result.Set(*reinterpret_cast<const int64_t*>(pValue));
        break;

    case BTypeId::Uint:
        result.Set(*reinterpret_cast<const uint32_t*>(pValue));
        break;

    case BTypeId::Uint64:
        result.Set(*reinterpret_cast<const uint64_t*>(pValue));
        break;

    case BTypeId::Float:
        result.Set(*reinterpret_cast<const float*>(pValue));
        break;

    case BTypeId::Bool:
        result.Set(*reinterpret_cast<const bool*>(pValue));
        break;

    case BTypeId::String:
        result.Set(*reinterpret_cast<const std::string*>(pValue));
        break;

    case BTypeId::Float2:
        result.Set(*reinterpret_cast<const glm::vec2*>(pValue));
        break;

    case BTypeId::Float3:
        result.Set(*reinterpret_cast<const glm::vec3*>(pValue));
        break;

    case BTypeId::Float4:
        result.Set(*reinterpret_cast<const glm::vec4*>(pValue));
        break;

    case BTypeId::Int2:
        result.Set(*reinterpret_cast<const glm::ivec2*>(pValue));
        break;

    case BTypeId::Int3:
        result.Set(*reinterpret_cast<const glm::ivec3*>(pValue));
        break;

    case BTypeId::Int4:
        result.Set(*reinterpret_cast<const glm::ivec4*>(pValue));
        break;

    default:
        throw std::runtime_error("Unsupported property type for Variant get: " + std::string(propName));
    }

    return result;
}

void Boon::BClass::SetValue(void* instance, const char* propName, const Variant& value) const
{
    const BProperty* prop = FindProperty(propName);
    if (!prop)
        throw std::runtime_error("Property not found: " + std::string(propName));

    uint8_t* base = reinterpret_cast<uint8_t*>(instance);
    void* pValue = base + prop->offset;

    switch (prop->typeId)
    {
    case BTypeId::Int:
        *reinterpret_cast<int32_t*>(pValue) = value.As<int32_t>();
        break;

    case BTypeId::Int64:
        *reinterpret_cast<int64_t*>(pValue) = value.As<int64_t>();
        break;

    case BTypeId::Uint:
        *reinterpret_cast<uint32_t*>(pValue) = value.As<uint32_t>();
        break;

    case BTypeId::Uint64:
        *reinterpret_cast<uint64_t*>(pValue) = value.As<uint64_t>();
        break;

    case BTypeId::Float:
        *reinterpret_cast<float*>(pValue) = value.As<float>();
        break;

    case BTypeId::Bool:
        *reinterpret_cast<bool*>(pValue) = value.As<bool>();
        break;

    case BTypeId::String:
        *reinterpret_cast<std::string*>(pValue) = value.As<std::string>();
        break;

    case BTypeId::Float2:
        *reinterpret_cast<glm::vec2*>(pValue) = value.As<glm::vec2>();
        break;

    case BTypeId::Float3:
        *reinterpret_cast<glm::vec3*>(pValue) = value.As<glm::vec3>();
        break;

    case BTypeId::Float4:
        *reinterpret_cast<glm::vec4*>(pValue) = value.As<glm::vec4>();
        break;

    case BTypeId::Int2:
        *reinterpret_cast<glm::ivec2*>(pValue) = value.As<glm::ivec2>();
        break;

    case BTypeId::Int3:
        *reinterpret_cast<glm::ivec3*>(pValue) = value.As<glm::ivec3>();
        break;

    case BTypeId::Int4:
        *reinterpret_cast<glm::ivec4*>(pValue) = value.As<glm::ivec4>();
        break;

    default:
        throw std::runtime_error("Unsupported property type for Variant set: " + std::string(propName));
    }
}

bool Boon::BProperty::IsVariant() const
{
    switch (typeId)
    {
    case BTypeId::Int:
    case BTypeId::Int64:
    case BTypeId::Uint:
    case BTypeId::Uint64:
    case BTypeId::Float:
    case BTypeId::Bool:
    case BTypeId::String:
    case BTypeId::Float2:
    case BTypeId::Float3:
    case BTypeId::Float4:
    case BTypeId::Int2:
    case BTypeId::Int3:
    case BTypeId::Int4:
        return true;
    default:
        return false;
    }
}