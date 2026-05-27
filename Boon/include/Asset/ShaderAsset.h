#pragma once

#include "Asset/Asset.h"
#include "Asset/AssetMeta.h"
#include "Asset/AssetSerializer.h"
#include "Asset/AssetTraits.h"
#include "Core/Memory/Buffer.h"
#include "Renderer/Shader.h"

#include "Renderer/ShaderCompiler/ShaderReflection.h"
#include "Renderer/ShaderCompiler/GLSLReflectionProvider.h"

#include <memory>
#include <string>

namespace Boon
{
    class ShaderAsset : public Asset
    {
    public:
        using Type = Shader;

        explicit ShaderAsset(AssetHandle handle)
            : Asset(handle)
        {
        }

        std::shared_ptr<Shader> GetInstance()
        {
            if (!m_RuntimeShader)
                m_RuntimeShader = Shader::Create(m_VertexSource, m_FragmentSource);

            return m_RuntimeShader;
        }

        const std::string& GetVertexSource() const { return m_VertexSource; }
        const std::string& GetFragmentSource() const { return m_FragmentSource; }

        const ShaderReflection& GetReflection() const { return m_Reflection; }
        const VertexBufferLayout& GetVertexLayout() const { return m_Reflection.VertexLayout; }
        const MaterialLayout& GetMaterialLayout() const { return m_Reflection.MaterialLayout; }

    private:
        std::string m_VertexSource;
        std::string m_FragmentSource;
        mutable std::shared_ptr<Shader> m_RuntimeShader = nullptr;
        ShaderReflection m_Reflection;

        friend class ShaderImporter;
        friend struct AssetSerializer<ShaderAsset>;
    };

    template<>
    struct AssetTraits<ShaderAsset>
    {
        static constexpr AssetType Type = AssetType::Shader;
        static constexpr const char* Name = "Shader";
    };

    template<>
    struct AssetSerializer<ShaderAsset>
    {
        static ShaderAsset* Load(Buffer& buffer, const AssetMeta& meta)
        {
            size_t cursor = 0;

            auto* asset = new ShaderAsset(meta.uuid);
            asset->m_VertexSource = buffer.ReadString(cursor);
            asset->m_FragmentSource = buffer.ReadString(cursor);

            GLSLReflectionProvider provider;
            asset->m_Reflection = provider.Reflect(
                asset->m_VertexSource,
                asset->m_FragmentSource);

            return asset;
        }

        static Buffer Serialize(ShaderAsset* asset)
        {
            Buffer out;
            out.WriteString(asset->GetVertexSource());
            out.WriteString(asset->GetFragmentSource());
            return out;
        }
    };
}
