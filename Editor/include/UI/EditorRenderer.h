#pragma once
#include "Project/ProjectConfig.h"
#include <memory>

namespace BoonEditor
{
	class EditorRenderer final
	{
	public:
		EditorRenderer(const Boon::ProjectConfig& config);
		~EditorRenderer();

		EditorRenderer(const EditorRenderer& other) = delete;
		EditorRenderer(EditorRenderer&& other) = delete;
		EditorRenderer& operator=(const EditorRenderer& other) = delete;
		EditorRenderer& operator=(EditorRenderer&& other) = delete;

		void BeginFrame();
		void EndFrame();

	private:
		class EditorRendererImpl;
		std::unique_ptr<EditorRendererImpl> m_pImpl;
	};
}