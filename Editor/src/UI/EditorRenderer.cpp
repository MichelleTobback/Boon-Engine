#include "UI/EditorRenderer.h"

#include <Core/Application.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

// TEMPORARY
#include <GLFW/glfw3.h>

using namespace BoonEditor;
using namespace Boon;

class EditorRenderer::EditorRendererImpl final
{
public:
	EditorRendererImpl()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		//SetDarkThemeColors();
		ApplyDarkPastelThemeColors();

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetApiWindow());

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");
	}

	~EditorRendererImpl()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void BeginFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 100.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		ImGui::End();
	}

	void EndFrame()
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	void SetDarkThemeColors()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	}

	void ApplyDarkPastelThemeColors()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* colors = style.Colors;

		// === Layout Tweaks ===
		style.WindowRounding = 8.0f;
		style.FrameRounding = 6.0f;
		style.GrabRounding = 6.0f;
		style.ScrollbarRounding = 8.0f;
		style.TabRounding = 6.0f;
		style.FrameBorderSize = 0.0f;
		style.WindowBorderSize = 0.0f;
		style.ItemSpacing = ImVec2(8, 6);
		style.WindowPadding = ImVec2(10, 10);
		style.FramePadding = ImVec2(6, 4);
		style.TabBorderSize = 0.0f;
		style.WindowMenuButtonPosition = ImGuiDir_None;
		style.SeparatorTextBorderSize = 0.0f;
		style.PopupRounding = 6.0f;

		// === Base Backgrounds ===
		colors[ImGuiCol_WindowBg] = ImVec4(0.16f, 0.13f, 0.20f, 1.00f);  // Deep muted purple
		colors[ImGuiCol_ChildBg] = ImVec4(0.18f, 0.14f, 0.22f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.20f, 0.16f, 0.26f, 0.98f);
		colors[ImGuiCol_Border] = ImVec4(0.45f, 0.35f, 0.50f, 0.50f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);

		// === Text ===
		colors[ImGuiCol_Text] = ImVec4(0.97f, 0.90f, 0.95f, 1.00f);  // Soft pastel pink
		colors[ImGuiCol_TextDisabled] = ImVec4(0.65f, 0.55f, 0.60f, 1.00f);

		// === Headers & Tree Nodes ===
		colors[ImGuiCol_Header] = ImVec4(0.75f, 0.55f, 0.80f, 0.40f);  // Pastel pink/purple
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.85f, 0.65f, 0.95f, 0.60f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.95f, 0.70f, 1.00f, 0.80f);

		// === Buttons ===
		colors[ImGuiCol_Button] = ImVec4(0.70f, 0.50f, 0.80f, 0.60f);  // Muted pastel purple
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.85f, 0.65f, 0.95f, 0.80f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.90f, 0.60f, 1.00f, 1.00f);

		// === Frame background (inputs, sliders) ===
		colors[ImGuiCol_FrameBg] = ImVec4(0.25f, 0.20f, 0.30f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.40f, 0.30f, 0.50f, 0.85f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.55f, 0.40f, 0.70f, 0.95f);

		// === Tabs ===
		colors[ImGuiCol_Tab] = ImVec4(0.22f, 0.18f, 0.28f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.70f, 0.50f, 0.85f, 0.85f);
		colors[ImGuiCol_TabActive] = ImVec4(0.85f, 0.60f, 0.95f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.18f, 0.15f, 0.25f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.50f, 0.40f, 0.65f, 1.00f);

		// === Title bar ===
		colors[ImGuiCol_TitleBg] = ImVec4(0.20f, 0.16f, 0.26f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.28f, 0.22f, 0.35f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.15f, 0.12f, 0.20f, 0.75f);

		// === Scrollbar ===
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.12f, 0.10f, 0.18f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.45f, 0.35f, 0.55f, 0.60f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.70f, 0.55f, 0.85f, 0.75f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.90f, 0.65f, 1.00f, 0.95f);

		// === Sliders and Checks ===
		colors[ImGuiCol_CheckMark] = ImVec4(0.95f, 0.70f, 1.00f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.85f, 0.60f, 0.95f, 0.75f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.95f, 0.70f, 1.00f, 1.00f);

		// === Resize grip ===
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.70f, 0.50f, 0.80f, 0.40f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.85f, 0.60f, 0.95f, 0.60f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.95f, 0.70f, 1.00f, 0.80f);

		// === Navigation and selection ===
		colors[ImGuiCol_NavHighlight] = ImVec4(0.90f, 0.65f, 1.00f, 0.80f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.85f, 0.60f, 0.95f, 0.35f);

		// === Menus and popups ===
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.18f, 0.14f, 0.22f, 1.00f);

		// === Overlays ===
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.12f, 0.10f, 0.18f, 0.85f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.12f, 0.10f, 0.18f, 0.60f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.75f, 1.00f, 0.70f);

		// === DragDrop ===
		colors[ImGuiCol_DragDropTarget] = ImVec4(0.90f, 0.60f, 1.00f, 1.00f);
	}
};

EditorRenderer::EditorRenderer()
	: m_pImpl{ std::make_unique<EditorRendererImpl>() } {}

EditorRenderer::~EditorRenderer(){}

void EditorRenderer::BeginFrame() { m_pImpl->BeginFrame(); }
void EditorRenderer::EndFrame() { m_pImpl->EndFrame(); }