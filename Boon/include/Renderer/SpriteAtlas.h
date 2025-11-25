#pragma once
#include "Asset/AssetRef.h"
#include "Asset/TextureAsset.h"

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <queue>

namespace Boon
{
    // ---------------------------
    // SpriteFrame data
    // ---------------------------
    struct SpriteFrame
    {
        glm::vec2 UV{0.f, 0.f};
        glm::vec2 Size{1.f, 1.f};
        float FrameTime;
    };

    class SpriteAtlas;

    // ---------------------------
    // Animation clip referencing
    // stable frame IDs
    // ---------------------------
    struct SpriteAnimClip
    {
        float Speed = 1.0f;
        SpriteAtlas* pAtlas = nullptr;

        // Stores STABLE IDs, NOT vector indices
        std::vector<int> Frames;
    };

    // ---------------------------
    // Internal storage: each frame
    // also carries a stable ID
    // ---------------------------
    struct FrameEntry
    {
        int stableId{-1};   // never changes during lifetime
        SpriteFrame frame;  // actual data
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

    public:
        // ---------------------------
        // Texture
        // ---------------------------
        inline void SetTexture(const AssetRef<Texture2DAsset>& tex) { m_pTexture = tex; }
        inline const AssetRef<Texture2DAsset>& GetTexture() const { return m_pTexture; }

        // ---------------------------
        // Frame Management (Stable IDs)
        // ---------------------------

        // Add frame, return stableId
        inline int AddSpriteFrame(const SpriteFrame& frame)
        {
            int id;
            if (!m_FreeIds.empty())
            {
                id = m_FreeIds.front();
                m_FreeIds.pop();
            }
            else
            {
                id = m_NextId++;
                m_IdToIndex.resize(m_NextId, -1);
            }

            int index = (int)m_Frames.size();

            // Store frame
            m_Frames.push_back(FrameEntry{ id, frame });

            // Map stable ID → index
            m_IdToIndex[id] = index;

            return id;
        }

        // Remove frame by stable ID
        inline void RemoveSpriteFrame(int stableId)
        {
            int index = m_IdToIndex[stableId];
            if (index < 0 || index >= (int)m_Frames.size())
                return;

            int lastIndex = (int)m_Frames.size() - 1;

            // swap with last
            std::swap(m_Frames[index], m_Frames[lastIndex]);

            // fix moved element's index
            int movedId = m_Frames[index].stableId;
            m_IdToIndex[movedId] = index;

            // shrink vector
            m_Frames.pop_back();

            // free the ID
            m_IdToIndex[stableId] = -1;
            m_FreeIds.push(stableId);
        }

        inline bool Exists(int stableId) const
        {
            if (stableId >= m_IdToIndex.size())
                return false;

            return m_IdToIndex[stableId] > -1;
        }

        // Modify existing frame
        inline void SetSpriteFrame(int stableId, const SpriteFrame& frame)
        {
            int index = m_IdToIndex[stableId];
            m_Frames[index].stableId = stableId;
            m_Frames[index].frame = frame;
        }

        // Access frame by stable ID
        inline const SpriteFrame& GetSpriteFrame(int stableId) const
        {
            int index = m_IdToIndex.at(stableId);
            return m_Frames[index].frame;
        }

        // Access raw entries (if needed)
        std::vector<FrameEntry>& GetFrameEntries() { return m_Frames; }
        const std::vector<FrameEntry>& GetFrameEntries() const { return m_Frames; }

        // Get all valid stable IDs
        inline std::vector<int> GetAllFrameIDs() const
        {
            std::vector<int> ids;
            ids.reserve(m_Frames.size());
            for (const auto& entry : m_Frames)
                ids.push_back(entry.stableId);
            return ids;
        }

        // ---------------------------
        // Animation Clip Management
        // ---------------------------
        inline void AddClip(const SpriteAnimClip& clip) { m_Clips.push_back(clip); }
        inline void RemoveClip(int index) { m_Clips.erase(m_Clips.begin() + index); }
        inline const std::vector<SpriteAnimClip>& GetClips() const { return m_Clips; }
        inline SpriteAnimClip& GetClip(int index) { return m_Clips[index]; }

    private:
        // ---------------------------
        // Members
        // ---------------------------
        AssetRef<Texture2DAsset> m_pTexture;

        // Vector of frames (packed)
        std::vector<FrameEntry> m_Frames;

        // stableId → vector index (size grows when needed)
        std::vector<int> m_IdToIndex;

        // IDs that can be reused
        std::queue<int> m_FreeIds;

        // Next ID to assign
        int m_NextId = 0;

        // Animation clips
        std::vector<SpriteAnimClip> m_Clips;
    };
}
