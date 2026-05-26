#pragma once

#include <vector>

#include "Renderer/Material.h"

#include <glm/glm.hpp>

namespace Boon
{
    struct QuadRenderItem2D
    {
        glm::mat4 Transform{ 1.0f };

        glm::vec4 Color{ 1.0f };
        float TilingFactor = 1.0f;

        std::shared_ptr<Texture2D> Texture = nullptr;
        std::shared_ptr<Material> MaterialOverride = nullptr;

        glm::vec2 UV0{ 0.0f };
        glm::vec2 UV1{ 1.0f };

        int EntityID = -1;

        int SortLayer = 0;
        float SortOrder = 0.0f;
    };

    struct LineRenderItem2D
    {
        glm::vec3 P0{ 0.0f };
        glm::vec3 P1{ 0.0f };

        glm::vec4 Color{ 1.0f };
    };

    struct GeometryRenderItem2D
    {
        glm::mat4 Transform{ 1.0f };

        std::shared_ptr<VertexInput> VertexInput = nullptr;
        std::shared_ptr<Material> Material = nullptr;

        int EntityID = -1;

        int SortLayer = 0;
        float SortOrder = 0.0f;
    };

    class RenderQueue2D
    {
    public:
        void Clear()
        {
            m_Quads.clear();
            m_Lines.clear();
            m_Geometry.clear();
        }

        void Submit(const QuadRenderItem2D& item)
        {
            m_Quads.push_back(item);
        }

        void Submit(const LineRenderItem2D& item)
        {
            m_Lines.push_back(item);
        }

        void Submit(const GeometryRenderItem2D& item)
        {
            m_Geometry.push_back(item);
        }

        const std::vector<QuadRenderItem2D>& GetQuads() const
        {
            return m_Quads;
        }

        const std::vector<LineRenderItem2D>& GetLines() const
        {
            return m_Lines;
        }

        const std::vector<GeometryRenderItem2D>& GetGeometry() const
        {
            return m_Geometry;
        }

    private:
        std::vector<QuadRenderItem2D> m_Quads;
        std::vector<LineRenderItem2D> m_Lines;
        std::vector<GeometryRenderItem2D> m_Geometry;
    };
}