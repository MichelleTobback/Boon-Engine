#pragma once

#include "Asset/AssetRef.h"
#include "Asset/TextureAsset.h"

#include <glm/glm.hpp>

#include <memory>
#include <queue>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>

namespace Boon
{
    struct SpriteFrame
    {
        glm::vec2 UV{ 0.0f, 0.0f };
        glm::vec2 Size{ 1.0f, 1.0f };
    };

    class SpriteAtlas;

    struct SpriteAnimClip
    {
        std::string Name = "Clip";
        float FPS = 12.0f;
        float Speed = 1.0f;

        // Kept for compatibility with older code.
        // Runtime/editor code should not rely on this.
        SpriteAtlas* pAtlas = nullptr;

        // Stable frame IDs, not vector indices.
        std::vector<int> Frames;
    };

    struct FrameEntry
    {
        int stableId = -1;
        SpriteFrame frame;
    };

    class Texture2D;

    class SpriteAtlas
    {
    public:
        SpriteAtlas() = default;
        virtual ~SpriteAtlas() = default;

        SpriteAtlas(const SpriteAtlas&) = delete;
        SpriteAtlas(SpriteAtlas&&) = delete;
        SpriteAtlas& operator=(const SpriteAtlas&) = delete;
        SpriteAtlas& operator=(SpriteAtlas&&) = delete;

        void SetTexture(const AssetRef<Texture2DAsset>& texture)
        {
            m_pTexture = texture;
        }

        const AssetRef<Texture2DAsset>& GetTexture() const
        {
            return m_pTexture;
        }

        int AddSpriteFrame(const SpriteFrame& frame)
        {
            int id = -1;

            if (!m_FreeIds.empty())
            {
                id = m_FreeIds.front();
                m_FreeIds.pop();
            }
            else
            {
                id = m_NextId++;
                m_IdToIndex.resize(static_cast<size_t>(m_NextId), -1);
            }

            const int index = static_cast<int>(m_Frames.size());

            m_Frames.push_back(FrameEntry{ id, frame });
            m_IdToIndex[static_cast<size_t>(id)] = index;

            return id;
        }

        bool AddSpriteFrameWithId(int stableId, const SpriteFrame& frame)
        {
            if (stableId < 0)
                return false;

            if (static_cast<size_t>(stableId) >= m_IdToIndex.size())
                m_IdToIndex.resize(static_cast<size_t>(stableId) + 1, -1);

            if (m_IdToIndex[static_cast<size_t>(stableId)] != -1)
                return false;

            const int index = static_cast<int>(m_Frames.size());

            m_Frames.push_back(FrameEntry{ stableId, frame });
            m_IdToIndex[static_cast<size_t>(stableId)] = index;

            if (stableId >= m_NextId)
                m_NextId = stableId + 1;

            return true;
        }

        void RemoveSpriteFrame(int stableId)
        {
            if (!Exists(stableId))
                return;

            const int index = m_IdToIndex[static_cast<size_t>(stableId)];
            const int lastIndex = static_cast<int>(m_Frames.size()) - 1;

            if (index != lastIndex)
            {
                std::swap(m_Frames[index], m_Frames[lastIndex]);

                const int movedId = m_Frames[index].stableId;
                m_IdToIndex[static_cast<size_t>(movedId)] = index;
            }

            m_Frames.pop_back();

            m_IdToIndex[static_cast<size_t>(stableId)] = -1;
            m_FreeIds.push(stableId);

            // Remove deleted frame references from clips.
            for (SpriteAnimClip& clip : m_Clips)
            {
                clip.Frames.erase(
                    std::remove(clip.Frames.begin(), clip.Frames.end(), stableId),
                    clip.Frames.end()
                );
            }
        }

        bool Exists(int stableId) const
        {
            if (stableId < 0)
                return false;

            if (static_cast<size_t>(stableId) >= m_IdToIndex.size())
                return false;

            return m_IdToIndex[static_cast<size_t>(stableId)] != -1;
        }

        void SetSpriteFrame(int stableId, const SpriteFrame& frame)
        {
            if (!Exists(stableId))
                return;

            const int index = m_IdToIndex[static_cast<size_t>(stableId)];

            m_Frames[index].stableId = stableId;
            m_Frames[index].frame = frame;
        }

        void SetOrAddSpriteFrame(int stableId, const SpriteFrame& frame)
        {
            if (Exists(stableId))
                SetSpriteFrame(stableId, frame);
            else
                AddSpriteFrameWithId(stableId, frame);
        }

        const SpriteFrame& GetSpriteFrame(int stableId) const
        {
            const int index = m_IdToIndex.at(static_cast<size_t>(stableId));
            return m_Frames[index].frame;
        }

        SpriteFrame& GetSpriteFrameMutable(int stableId)
        {
            const int index = m_IdToIndex.at(static_cast<size_t>(stableId));
            return m_Frames[index].frame;
        }

        std::vector<FrameEntry>& GetFrameEntries()
        {
            return m_Frames;
        }

        const std::vector<FrameEntry>& GetFrameEntries() const
        {
            return m_Frames;
        }

        size_t GetFrameCount() const
        {
            return m_Frames.size();
        }

        std::vector<int> GetAllFrameIDs() const
        {
            std::vector<int> ids;
            ids.reserve(m_Frames.size());

            for (const FrameEntry& entry : m_Frames)
                ids.push_back(entry.stableId);

            return ids;
        }

        int AddClip(const SpriteAnimClip& clip)
        {
            SpriteAnimClip copy = clip;
            copy.pAtlas = this;

            m_Clips.push_back(copy);
            return static_cast<int>(m_Clips.size()) - 1;
        }

        void RemoveClip(int index)
        {
            if (!IsValidClip(index))
                return;

            m_Clips.erase(m_Clips.begin() + index);
        }

        bool IsValidClip(int index) const
        {
            return index >= 0 && index < static_cast<int>(m_Clips.size());
        }

        size_t GetClipCount() const
        {
            return m_Clips.size();
        }

        const std::vector<SpriteAnimClip>& GetClips() const
        {
            return m_Clips;
        }

        std::vector<SpriteAnimClip>& GetClips()
        {
            return m_Clips;
        }

        SpriteAnimClip& GetClip(int index)
        {
            return m_Clips[index];
        }

        const SpriteAnimClip& GetClip(int index) const
        {
            return m_Clips[index];
        }

        int FindClipIndex(std::string_view name) const
        {
            for (int i = 0; i < static_cast<int>(m_Clips.size()); ++i)
            {
                if (m_Clips[i].Name == name)
                    return i;
            }

            return -1;
        }

        SpriteAnimClip* FindClip(std::string_view name)
        {
            const int index = FindClipIndex(name);
            return index >= 0 ? &m_Clips[index] : nullptr;
        }

        const SpriteAnimClip* FindClip(std::string_view name) const
        {
            const int index = FindClipIndex(name);
            return index >= 0 ? &m_Clips[index] : nullptr;
        }

    private:
        AssetRef<Texture2DAsset> m_pTexture;

        std::vector<FrameEntry> m_Frames;
        std::vector<int> m_IdToIndex;
        std::queue<int> m_FreeIds;
        int m_NextId = 0;

        std::vector<SpriteAnimClip> m_Clips;
    };
}