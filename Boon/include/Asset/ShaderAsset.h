#pragma once
#include "Asset/Asset.h"
#include "Asset/AssetTraits.h"
#include "Asset/AssetMeta.h"
#include "Core/Memory/Buffer.h"
#include "Renderer/Shader.h"

#include <memory>

namespace Boon
{
    class ShaderAsset : public Asset
    {
    public:
        using Type = Shader;
        ShaderAsset(AssetHandle handle)
            : Asset(handle) {
        }

        std::shared_ptr<Shader> GetInstance()
        {
            if (!m_RuntimeShader)
            {
                m_RuntimeShader = Shader::Create(m_VertexSource, m_FragmentSource);
            }
            return m_RuntimeShader;
        }

        const std::string& GetVertexSource() const { return m_VertexSource; }
        const std::string& GetFragmentSource() const { return m_FragmentSource; }

    private:
        friend class ShaderImporter;
        friend struct AssetTraits<ShaderAsset>;

        std::string m_VertexSource;
        std::string m_FragmentSource;

        mutable std::shared_ptr<Shader> m_RuntimeShader = nullptr;
    };


    template<>
    struct AssetTraits<ShaderAsset>
    {
        static constexpr AssetType Type = AssetType::Shader;

        static ShaderAsset* Load(Buffer& buffer, const AssetMeta& meta)
        {
            size_t cursor = 0;

            // Read vertex shader
            std::string vert = buffer.ReadString(cursor);

            // Read fragment shader
            std::string frag = buffer.ReadString(cursor);

            ShaderAsset* asset = new ShaderAsset(meta.uuid);
            asset->m_VertexSource = vert;
            asset->m_FragmentSource = frag;

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
