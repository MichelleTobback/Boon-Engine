#pragma once

#include "Asset/Asset.h"
#include "Asset/AssetMeta.h"
#include "Asset/AssetRef.h"
#include "Asset/AssetSerializer.h"
#include "Asset/AssetTraits.h"
#include "Asset/TextureAsset.h"
#include "Core/Memory/Buffer.h"
#include "Renderer/SpriteAtlas.h"
#include "Renderer/Material.h"

#include <memory>

namespace Boon
{
    class SpriteAtlasAsset : public Asset
    {
    public:
        using Type = SpriteAtlas;

        SpriteAtlasAsset(AssetHandle handle, const std::shared_ptr<SpriteAtlas>& atlas)
            : Asset(handle), m_pAtlas(atlas)
        {
        }

        std::shared_ptr<SpriteAtlas> GetInstance() const
        {
            return m_pAtlas;
        }

        void SetDefaultMaterial(const std::shared_ptr<Material>& material)
        {
            m_DefaultMaterial = material;
        }

        std::shared_ptr<Material> GetDefaultMaterial() const
        {
            return m_DefaultMaterial;
        }

    private:
        std::shared_ptr<SpriteAtlas> m_pAtlas = nullptr;
        std::shared_ptr<Material> m_DefaultMaterial = nullptr;

        friend class SpriteAtlasImporter;
        friend struct AssetSerializer<SpriteAtlasAsset>;
    };

    template<>
    struct AssetTraits<SpriteAtlasAsset>
    {
        static constexpr AssetType Type = AssetType::SpriteAtlas;
        static constexpr const char* Name = "SpriteAtlas";
    };

    template<>
    struct AssetSerializer<SpriteAtlasAsset>
    {
        static SpriteAtlasAsset* Load(Buffer& buffer, const AssetMeta& meta)
        {
            size_t cursor = 0;

            const uint64_t textureRaw = buffer.Read<uint64_t>(cursor);
            AssetHandle textureHandle(textureRaw);

            std::shared_ptr<SpriteAtlas> atlas = std::make_shared<SpriteAtlas>();
            atlas->SetTexture(AssetRef<Texture2DAsset>(textureHandle));

            const uint32_t frameCount = buffer.Read<uint32_t>(cursor);
            const uint32_t clipCount = buffer.Read<uint32_t>(cursor);

            std::vector<FrameEntry> frames(frameCount);
            buffer.ReadRaw(frames.data(), frameCount * sizeof(FrameEntry), cursor);
            for (uint32_t f = 0; f < frameCount; ++f)
            {
                //const FrameEntry frame = buffer.Read<FrameEntry>(cursor);
                atlas->AddSpriteFrameWithId(frames[f].stableId, frames[f].frame);
            }

            for (uint32_t i = 0; i < clipCount; ++i)
            {
                SpriteAnimClip clip{};

                const uint32_t nameLength = buffer.Read<uint32_t>(cursor);
                if (nameLength > 0)
                {
                    clip.Name.resize(nameLength);
                    buffer.ReadRaw(clip.Name.data(), nameLength, cursor);
                }

                clip.FPS = buffer.Read<float>(cursor);
                clip.Speed = buffer.Read<float>(cursor);

                const uint32_t count = buffer.Read<uint32_t>(cursor);
                clip.Frames.resize(count);
                buffer.ReadRaw(clip.Frames.data(), count * sizeof(int), cursor);

                clip.pAtlas = atlas.get();
                atlas->AddClip(clip);
            }

            return new SpriteAtlasAsset(meta.uuid, atlas);
        }

        static Buffer Serialize(SpriteAtlasAsset* asset)
        {
            Buffer out;

            SpriteAtlas* atlas = asset->GetInstance().get();
            if (!atlas)
                return out;

            out.Write<uint64_t>(static_cast<uint64_t>(atlas->GetTexture().Handle()));

            const auto& frames = atlas->GetFrameEntries();
            const auto& clips = atlas->GetClips();
            out.Write<uint32_t>(static_cast<uint32_t>(frames.size()));
            out.Write<uint32_t>(static_cast<uint32_t>(clips.size()));

            out.WriteRaw(frames.data(), frames.size() * sizeof(FrameEntry));

            for (const auto& clip : clips)
            {
                out.Write<uint32_t>(static_cast<uint32_t>(clip.Name.size()));

                if (!clip.Name.empty())
                    out.WriteRaw(clip.Name.data(), clip.Name.size());

                out.Write<float>(clip.FPS);
                out.Write<float>(clip.Speed);
                out.Write<uint32_t>(static_cast<uint32_t>(clip.Frames.size()));
                out.WriteRaw(clip.Frames.data(), clip.Frames.size() * sizeof(int));
            }

            return out;
        }
    };
}
