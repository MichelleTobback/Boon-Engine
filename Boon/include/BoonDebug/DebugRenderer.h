#include "Core/Singleton.h"

#include "Renderer/Renderer2D.h"
#include <queue>

namespace Boon
{
	class DebugDrawCmd
	{
	public:
		virtual void Draw(Renderer2D& renderer) = 0;
	};

	class DrawDebugLineCmd : public DebugDrawCmd
	{
	public:
		DrawDebugLineCmd(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color)
		: m_p0(p0), m_p1(p1), m_Color(color) { }

		virtual void Draw(Renderer2D& renderer) override
		{
			renderer.SubmitLine(m_p0, m_p1, m_Color);
		}

	private:
		glm::vec3 m_p0, m_p1;
		glm::vec4 m_Color;
	};

	class DrawDebugRectCmd : public DebugDrawCmd
	{
	public:
			DrawDebugRectCmd(const glm::vec3& p, const glm::vec2& size, const glm::vec4& color)
			: m_p(p), m_Size(size), m_Color(color) {
		}

		virtual void Draw(Renderer2D& renderer) override
		{
			renderer.SubmitRect(m_p, m_Size, m_Color);
		}

	private:
		glm::vec3 m_p;
		glm::vec2 m_Size;
		glm::vec4 m_Color;
	};

	class DebugRenderer final : public Singleton<DebugRenderer>
	{
	public:
		DebugRenderer() = default;

		void FlushQueue(Renderer2D& renderer)
		{
			auto& queue = m_pCommands;
			while (!queue.empty())
			{
				queue.front()->Draw(renderer);
				delete queue.front();
				queue.pop();
			}
		}

		void BeginFrame()
		{
			return;
			while (!m_pCommands.empty())
			{
				delete m_pCommands.front();
				m_pCommands.pop();
			}
		}

		void PushCommand(DebugDrawCmd* pCmd)
		{
			m_pCommands.push(pCmd);
		}

	private:
		std::queue<DebugDrawCmd*> m_pCommands;
	};
}

#define DEBUG_DRAW_LINE(p0, p1, color) \
    Boon::DebugRenderer::Get().PushCommand(new Boon::DrawDebugLineCmd((p0), (p1), (color)))

#define DEBUG_DRAW_RECT(p, size, color) \
    Boon::DebugRenderer::Get().PushCommand(new Boon::DrawDebugRectCmd((p), (size), (color)))