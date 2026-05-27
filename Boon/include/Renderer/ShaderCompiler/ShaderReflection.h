#pragma once

#include "Renderer/VertexBufferLayout.h"

#include <optional>
#include <string>
#include <vector>

namespace Boon
{
    enum class MaterialParameterType
    {
        None = 0,
        Float, Float2, Float3, Float4,
        Int, Int2, Int3, Int4,
        Mat3, Mat4,
        Bool
    };

    struct MaterialParameter
    {
        std::string Name;
        MaterialParameterType Type = MaterialParameterType::None;
        uint32_t Offset = 0;
        uint32_t Size = 0;
    };

    struct MaterialTextureSlot
    {
        std::string Name;
        uint32_t Slot = 0;
        uint32_t Count = 1;
        bool IsArray = false;
    };

    struct MaterialLayout
    {
        uint32_t UniformBufferSize = 0;
        uint32_t UniformBinding = 2;

        std::vector<MaterialParameter> Parameters;
        std::vector<MaterialTextureSlot> Textures;

        const MaterialParameter* FindParameter(const std::string& name) const
        {
            for (const auto& parameter : Parameters)
            {
                if (parameter.Name == name)
                    return &parameter;
            }

            return nullptr;
        }

        const MaterialTextureSlot* FindTexture(const std::string& name) const
        {
            for (const auto& texture : Textures)
            {
                if (texture.Name == name)
                    return &texture;
            }

            return nullptr;
        }

        std::optional<uint32_t> FindTextureSlot(const std::string& name) const
        {
            const MaterialTextureSlot* texture = FindTexture(name);
            if (!texture)
                return std::nullopt;

            return texture->Slot;
        }
    };

    struct ShaderReflection
    {
        VertexBufferLayout VertexLayout;
        MaterialLayout MaterialLayout;
    };

    class IShaderReflectionProvider
    {
    public:
        virtual ~IShaderReflectionProvider() = default;

        virtual ShaderReflection Reflect(
            const std::string& vertexSource,
            const std::string& fragmentSource) = 0;
    };
}
