#pragma once
#include "Project/ProjectConfig.h"
#include <memory>
#include <functional>

namespace Boon
{
	class Texture2D;
}

namespace BoonEditor
{
	class EditorRenderer final
	{
	public:
		EditorRenderer(const Boon::ProjectConfig& config, const std::shared_ptr<Boon::Texture2D>& icon);
		~EditorRenderer();

		EditorRenderer(const EditorRenderer& other) = delete;
		EditorRenderer(EditorRenderer&& other) = delete;
		EditorRenderer& operator=(const EditorRenderer& other) = delete;
		EditorRenderer& operator=(EditorRenderer&& other) = delete;

		void BeginFrame();
		void EndFrame();

		using MenuBarCallback = std::function<void()>;

		void SetMenuBarCallback(const MenuBarCallback& callback);

	private:
		class EditorRendererImpl;
		std::unique_ptr<EditorRendererImpl> m_pImpl;
	};
}