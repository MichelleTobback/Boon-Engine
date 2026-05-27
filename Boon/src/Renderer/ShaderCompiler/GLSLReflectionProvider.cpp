#include "Renderer/ShaderCompiler/GLSLReflectionProvider.h"

#include <sstream>

namespace Boon
{
    static ShaderDataType ParseShaderDataType(const std::string& type)
    {
        if (type == "float") return ShaderDataType::Float;
        if (type == "vec2")  return ShaderDataType::Float2;
        if (type == "vec3")  return ShaderDataType::Float3;
        if (type == "vec4")  return ShaderDataType::Float4;
        if (type == "mat3")  return ShaderDataType::Mat3;
        if (type == "mat4")  return ShaderDataType::Mat4;
        if (type == "int")   return ShaderDataType::Int;
        if (type == "ivec2") return ShaderDataType::Int2;
        if (type == "ivec3") return ShaderDataType::Int3;
        if (type == "ivec4") return ShaderDataType::Int4;
        if (type == "bool")  return ShaderDataType::Bool;

        return ShaderDataType::None;
    }

    static MaterialParameterType ParseMaterialParameterType(const std::string& type)
    {
        if (type == "float") return MaterialParameterType::Float;
        if (type == "vec2")  return MaterialParameterType::Float2;
        if (type == "vec3")  return MaterialParameterType::Float3;
        if (type == "vec4")  return MaterialParameterType::Float4;
        if (type == "mat3")  return MaterialParameterType::Mat3;
        if (type == "mat4")  return MaterialParameterType::Mat4;
        if (type == "int")   return MaterialParameterType::Int;
        if (type == "ivec2") return MaterialParameterType::Int2;
        if (type == "ivec3") return MaterialParameterType::Int3;
        if (type == "ivec4") return MaterialParameterType::Int4;
        if (type == "bool")  return MaterialParameterType::Bool;

        return MaterialParameterType::None;
    }

    static uint32_t MaterialParameterTypeSize(MaterialParameterType type)
    {
        switch (type)
        {
        case MaterialParameterType::Float:  return 4;
        case MaterialParameterType::Float2: return 8;
        case MaterialParameterType::Float3: return 12;
        case MaterialParameterType::Float4: return 16;
        case MaterialParameterType::Int:    return 4;
        case MaterialParameterType::Int2:   return 8;
        case MaterialParameterType::Int3:   return 12;
        case MaterialParameterType::Int4:   return 16;
        case MaterialParameterType::Mat3:   return 36;
        case MaterialParameterType::Mat4:   return 64;
        case MaterialParameterType::Bool:   return 1;
        default: return 0;
        }
    }

    static void ParseLine(const std::string& line, ShaderReflection& reflection)
    {
        std::istringstream stream(line);

        std::string comment;
        std::string tag;
        stream >> comment >> tag;

        if (comment != "//")
            return;

        if (tag == "@vertex")
        {
            std::string type;
            std::string name;

            stream >> type >> name;

            const ShaderDataType dataType = ParseShaderDataType(type);
            if (dataType != ShaderDataType::None && !name.empty())
            {
                auto elements = reflection.VertexLayout.GetElements();
                elements.emplace_back(dataType, name);

                reflection.VertexLayout = VertexBufferLayout(elements);
            }

            return;
        }

        if (tag == "@material")
        {
            std::string type;
            std::string name;
            uint32_t offset = 0;

            stream >> type >> name >> offset;

            const MaterialParameterType parameterType = ParseMaterialParameterType(type);
            const uint32_t size = MaterialParameterTypeSize(parameterType);

            if (parameterType != MaterialParameterType::None && !name.empty())
            {
                reflection.MaterialLayout.Parameters.push_back({ name, parameterType, offset, size });

                reflection.MaterialLayout.UniformBufferSize =
                    std::max(reflection.MaterialLayout.UniformBufferSize, offset + size);
            }

            return;
        }

        if (tag == "@texture")
        {
            std::string name;
            uint32_t slot = 0;

            stream >> name >> slot;

            if (!name.empty())
                reflection.MaterialLayout.Textures.push_back({ name, slot, 1, false });

            return;
        }

        if (tag == "@texture_array")
        {
            std::string name;
            uint32_t slot = 0;
            uint32_t count = 1;

            stream >> name >> slot >> count;

            if (!name.empty())
                reflection.MaterialLayout.Textures.push_back({ name, slot, count, true });

            return;
        }

        if (tag == "@material_binding")
        {
            uint32_t binding = 2;
            stream >> binding;
            reflection.MaterialLayout.UniformBinding = binding;
            return;
        }
    }

    ShaderReflection GLSLReflectionProvider::Reflect(const std::string& vertexSource, const std::string& fragmentSource)
    {
        ShaderReflection reflection{};

        std::istringstream vertexStream(vertexSource);
        std::istringstream fragmentStream(fragmentSource);

        std::string line;

        while (std::getline(vertexStream, line))
            ParseLine(line, reflection);

        while (std::getline(fragmentStream, line))
            ParseLine(line, reflection);

        return reflection;
    }
}