#include "UI/EditorRenderer.h"
#include <UI/IconsFontAwesome7.h>

#include <Core/Application.h>

#include <Renderer/Texture.h>

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
    struct Fonts
    {
        ImFont* Regular = nullptr;
        ImFont* Medium = nullptr;
        ImFont* Title = nullptr;
        ImFont* Mono = nullptr;
        ImFont* Small = nullptr;
    };

    enum class WindowDragMode
    {
        None,
        Move,
        ResizeLeft,
        ResizeRight,
        ResizeTop,
        ResizeBottom,
        ResizeTopLeft,
        ResizeTopRight,
        ResizeBottomLeft,
        ResizeBottomRight
    };

public:
    EditorRendererImpl(const ProjectConfig& config, const std::shared_ptr<Texture2D>& icon)
        : m_LogoTexture{icon}
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        static std::string iniPath =
            (config.Editor.EditorResourcesRoot / "imgui.ini").string();

        io.IniFilename = iniPath.c_str();

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowMenuButtonPosition = ImGuiDir_None;

        LoadFonts(io, config);

        ImGui::StyleColorsDark();

        ApplyDarkThemeColors();

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGuiStyle& style = ImGui::GetStyle();

            style.WindowRounding = 10.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        Application& app = Application::Get();

        GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetApiWindow());

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

        Application& app = Application::Get();

        GLFWwindow* window =
            static_cast<GLFWwindow*>(app.GetWindow().GetApiWindow());

        WindowDragMode resizeMode = GetResizeMode(window);

        if (m_WindowDragMode == WindowDragMode::None && resizeMode != WindowDragMode::None)
        {
            switch (resizeMode)
            {
            case WindowDragMode::ResizeLeft:
            case WindowDragMode::ResizeRight:
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                break;

            case WindowDragMode::ResizeTop:
            case WindowDragMode::ResizeBottom:
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
                break;

            case WindowDragMode::ResizeTopLeft:
            case WindowDragMode::ResizeBottomRight:
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
                break;

            case WindowDragMode::ResizeTopRight:
            case WindowDragMode::ResizeBottomLeft:
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW);
                break;

            default:
                break;
            }

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                BeginWindowDrag(window, resizeMode);
        }

        UpdateWindowDrag(window);

        constexpr float titlebarHeight = 40.0f;

        DrawCustomTitlebar(window, titlebarHeight);

        HandleBorderlessResize(window, titlebarHeight);

        static bool dockspaceOpen = true;
        static bool opt_fullscreen_persistant = true;

        bool opt_fullscreen = opt_fullscreen_persistant;

        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_AutoHideTabBar;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;

        if (opt_fullscreen)
        {
            ImGuiViewport* viewport = ImGui::GetMainViewport();

            ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + titlebarHeight));
            ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - titlebarHeight));
            ImGui::SetNextWindowViewport(viewport->ID);

            ImGui::PushStyleVar(
                ImGuiStyleVar_WindowRounding,
                0.0f
            );

            ImGui::PushStyleVar(
                ImGuiStyleVar_WindowBorderSize,
                0.0f
            );

            window_flags |=
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove;

            window_flags |=
                ImGuiWindowFlags_NoBringToFrontOnFocus |
                ImGuiWindowFlags_NoNavFocus;
        }

        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(
            ImGuiStyleVar_WindowPadding,
            ImVec2(0.0f, 0.0f)
        );

        ImGui::Begin(
            "DockSpace Demo",
            &dockspaceOpen,
            window_flags
        );

        ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        ImGuiStyle& style = ImGui::GetStyle();

        float minWinSizeX = style.WindowMinSize.x;
        style.WindowMinSize.x = 100.0f;

        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id =
                ImGui::GetID("MyDockSpace");

            ImGui::DockSpace(
                dockspace_id,
                ImVec2(0.0f, 0.0f),
                dockspace_flags
            );
        }

        style.WindowMinSize.x = minWinSizeX;

        ImGui::End();
    }

    void EndFrame()
    {
        ImGuiIO& io = ImGui::GetIO();

        Application& app = Application::Get();

        io.DisplaySize = ImVec2(
            (float)app.GetWindow().GetWidth(),
            (float)app.GetWindow().GetHeight()
        );

        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(
            ImGui::GetDrawData()
        );

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context =
                glfwGetCurrentContext();

            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();

            glfwMakeContextCurrent(
                backup_current_context
            );
        }
    }

    void SetMenuBarCallback(const MenuBarCallback& callback)
    {
        MenuBarRenderer = callback;
    }

private:
    void LoadFonts(ImGuiIO& io, const ProjectConfig& config)
    {
        constexpr float baseFontSize = 15.0f;
        constexpr float iconFontSize = 13.0f;

        const auto fontsRoot =
            config.Editor.EditorResourcesRoot / "Fonts";

        // ------------------------------------------------------------
        // Main font
        // ------------------------------------------------------------

        m_Fonts.Regular = io.Fonts->AddFontFromFileTTF(
            (fontsRoot / "Inter-Regular.ttf").string().c_str(),
            baseFontSize);

        // ------------------------------------------------------------
        // Merge icons INTO regular font
        // ------------------------------------------------------------

        static constexpr ImWchar iconRanges[] =
        {
            ICON_MIN_FA,
            ICON_MAX_16_FA,
            0
        };

        ImFontConfig iconConfig;
        iconConfig.MergeMode = true;
        iconConfig.PixelSnapH = true;

        ImFont* iconFont = io.Fonts->AddFontFromFileTTF(
            (fontsRoot / "Font Awesome 7 Free-Solid-900.otf").string().c_str(),
            iconFontSize,
            &iconConfig,
            iconRanges);

        IM_ASSERT(iconFont && "Failed to load Font Awesome icon font");

        // ------------------------------------------------------------
        // Other fonts
        // ------------------------------------------------------------

        m_Fonts.Medium = io.Fonts->AddFontFromFileTTF(
            (fontsRoot / "Inter-Medium.ttf").string().c_str(),
            baseFontSize);

        m_Fonts.Title = io.Fonts->AddFontFromFileTTF(
            (fontsRoot / "Inter-SemiBold.ttf").string().c_str(),
            baseFontSize);

        m_Fonts.Small = io.Fonts->AddFontFromFileTTF(
            (fontsRoot / "Inter-Regular.ttf").string().c_str(),
            baseFontSize);

        m_Fonts.Mono = io.Fonts->AddFontFromFileTTF(
            (fontsRoot / "JetBrainsMono-Regular.ttf").string().c_str(),
            baseFontSize);

        if (m_Fonts.Regular)
            io.FontDefault = m_Fonts.Regular;
    }

    void ApplyDarkThemeColors()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // Layout
        style.WindowRounding = 10.0f;
        style.ChildRounding = 10.0f;
        style.FrameRounding = 7.0f;
        style.PopupRounding = 8.0f;
        style.ScrollbarRounding = 8.0f;
        style.GrabRounding = 6.0f;
        style.TabRounding = 7.0f;

        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;
        style.TabBorderSize = 0.0f;

        style.WindowPadding = ImVec2(12, 12);
        style.FramePadding = ImVec2(10, 6);
        style.ItemSpacing = ImVec2(10, 8);
        style.ItemInnerSpacing = ImVec2(8, 6);

        style.WindowMenuButtonPosition = ImGuiDir_None;

        // Backgrounds
        colors[ImGuiCol_WindowBg] =
            ImVec4(0.075f, 0.08f, 0.09f, 1.00f);

        colors[ImGuiCol_ChildBg] =
            ImVec4(0.12f, 0.13f, 0.145f, 1.00f);

        colors[ImGuiCol_PopupBg] =
            ImVec4(0.11f, 0.12f, 0.14f, 0.98f);

        // Borders
        colors[ImGuiCol_Border] =
            ImVec4(1.0f, 1.0f, 1.0f, 0.05f);

        colors[ImGuiCol_BorderShadow] =
            ImVec4(0, 0, 0, 0);

        // Text
        colors[ImGuiCol_Text] =
            ImVec4(0.92f, 0.94f, 0.96f, 1.00f);

        colors[ImGuiCol_TextDisabled] =
            ImVec4(0.55f, 0.58f, 0.62f, 1.00f);

        // Frames
        colors[ImGuiCol_FrameBg] =
            ImVec4(0.10f, 0.11f, 0.13f, 1.00f);

        colors[ImGuiCol_FrameBgHovered] =
            ImVec4(0.14f, 0.15f, 0.18f, 1.00f);

        colors[ImGuiCol_FrameBgActive] =
            ImVec4(0.18f, 0.20f, 0.24f, 1.00f);

        // Buttons
        colors[ImGuiCol_Button] =
            ImVec4(0.13f, 0.14f, 0.16f, 1.00f);

        colors[ImGuiCol_ButtonHovered] =
            ImVec4(0.58f, 0.36f, 0.78f, 0.18f);

        colors[ImGuiCol_ButtonActive] =
            ImVec4(0.58f, 0.36f, 0.78f, 0.28f);

        // Headers
        colors[ImGuiCol_Header] =
            ImVec4(0.14f, 0.15f, 0.17f, 1.00f);

        colors[ImGuiCol_HeaderHovered] =
            ImVec4(0.58f, 0.36f, 0.78f, 0.18f);

        colors[ImGuiCol_HeaderActive] =
            ImVec4(0.58f, 0.36f, 0.78f, 0.28f);

        // Tabs
        colors[ImGuiCol_Tab] =
            ImVec4(0.11f, 0.12f, 0.14f, 1.00f);

        colors[ImGuiCol_TabHovered] =
            ImVec4(0.58f, 0.36f, 0.78f, 0.30f);

        colors[ImGuiCol_TabActive] =
            ImVec4(0.58f, 0.36f, 0.78f, 0.65f);

        colors[ImGuiCol_TabUnfocused] =
            ImVec4(0.10f, 0.11f, 0.12f, 1.00f);

        colors[ImGuiCol_TabUnfocusedActive] =
            ImVec4(0.18f, 0.19f, 0.21f, 1.00f);

        // Title bar
        colors[ImGuiCol_TitleBg] =
            ImVec4(0.09f, 0.10f, 0.11f, 1.00f);

        colors[ImGuiCol_TitleBgActive] =
            ImVec4(0.12f, 0.13f, 0.15f, 1.00f);

        colors[ImGuiCol_TitleBgCollapsed] =
            ImVec4(0.08f, 0.09f, 0.10f, 1.00f);

        // Scrollbars
        colors[ImGuiCol_ScrollbarBg] =
            ImVec4(0.07f, 0.07f, 0.08f, 1.00f);

        colors[ImGuiCol_ScrollbarGrab] =
            ImVec4(0.22f, 0.24f, 0.27f, 1.00f);

        colors[ImGuiCol_ScrollbarGrabHovered] =
            ImVec4(0.32f, 0.34f, 0.38f, 1.00f);

        colors[ImGuiCol_ScrollbarGrabActive] =
            ImVec4(0.42f, 0.44f, 0.48f, 1.00f);

        // Sliders
        colors[ImGuiCol_CheckMark] =
            ImVec4(0.58f, 0.36f, 0.78f, 1.00f);

        colors[ImGuiCol_SliderGrab] =
            ImVec4(0.58f, 0.36f, 0.78f, 0.85f);

        colors[ImGuiCol_SliderGrabActive] =
            ImVec4(0.58f, 0.36f, 0.78f, 1.00f);

        // Separators
        colors[ImGuiCol_Separator] =
            ImVec4(1, 1, 1, 0.06f);

        colors[ImGuiCol_SeparatorHovered] =
            ImVec4(0.58f, 0.36f, 0.78f, 0.40f);

        colors[ImGuiCol_SeparatorActive] =
            ImVec4(0.58f, 0.36f, 0.78f, 0.65f);

        // Resize grips
        colors[ImGuiCol_ResizeGrip] =
            ImVec4(0.22f, 0.24f, 0.26f, 0.60f);

        colors[ImGuiCol_ResizeGripHovered] =
            ImVec4(0.58f, 0.36f, 0.78f, 0.45f);

        colors[ImGuiCol_ResizeGripActive] =
            ImVec4(0.58f, 0.36f, 0.78f, 0.75f);

        // Selection
        colors[ImGuiCol_TextSelectedBg] =
            ImVec4(0.58f, 0.36f, 0.78f, 0.20f);

        colors[ImGuiCol_NavHighlight] =
            ImVec4(0.58f, 0.36f, 0.78f, 0.60f);

        // Menu bar
        colors[ImGuiCol_MenuBarBg] =
            ImVec4(0.10f, 0.11f, 0.12f, 1.00f);

        // Drag/drop
        colors[ImGuiCol_DragDropTarget] =
            ImVec4(0.58f, 0.36f, 0.78f, 1.00f);

        // Docking
        colors[ImGuiCol_DockingPreview] =
            ImVec4(0.58f, 0.36f, 0.78f, 0.30f);

        colors[ImGuiCol_DockingEmptyBg] =
            ImVec4(0.06f, 0.065f, 0.075f, 1.00f);
    }

    void DrawCustomTitlebar(GLFWwindow* window, float titlebarHeight)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, titlebarHeight));
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 7.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.075f, 0.08f, 0.09f, 1.0f));

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::Begin("##CustomTitlebar", nullptr, flags);

        constexpr float logoSize = 24.0f;
        constexpr float buttonWidth = 36.0f;
        constexpr float buttonHeight = 26.0f;

        // Logo
        if (m_LogoTexture)
        {
            ImGui::Image(
                (ImTextureID)(intptr_t)m_LogoTexture->GetRendererID(),
                ImVec2(logoSize, logoSize), ImVec2(1, 1), ImVec2(0, 0)
            );
        }
        else
        {
            ImGui::TextUnformatted("◆");
        }

        // ─────────────────────────────
        // Left-side menu
        // ─────────────────────────────
        ImGui::SameLine();

        if (MenuBarRenderer)
            MenuBarRenderer();

        // ─────────────────────────────
        // Centered title
        // ─────────────────────────────
        const char* title = Application::Get().GetDescriptor().Window.Title.c_str();

        ImGui::PushFont(m_Fonts.Title);

        ImVec2 titleSize = ImGui::CalcTextSize(title);

        float titleX =
            (ImGui::GetWindowWidth() * 0.5f) -
            (titleSize.x * 0.5f);

        ImGui::SetCursorPos(ImVec2(titleX, 8.0f));

        ImGui::TextUnformatted(title);

        ImGui::PopFont();

        // Window buttons
        const float rightOffset = buttonWidth * 3.0f + 10.0f;
        ImGui::SameLine(ImGui::GetWindowWidth() - rightOffset);
        ImGui::SetCursorPosY(7.0f);

        auto TitleButton = [&](const char* label, bool danger = false)
            {
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleColor(
                    ImGuiCol_ButtonHovered,
                    danger ? ImVec4(0.80f, 0.18f, 0.24f, 0.85f)
                    : ImVec4(1.0f, 1.0f, 1.0f, 0.08f)
                );
                ImGui::PushStyleColor(
                    ImGuiCol_ButtonActive,
                    danger ? ImVec4(0.90f, 0.12f, 0.18f, 1.0f)
                    : ImVec4(0.58f, 0.36f, 0.78f, 0.28f)
                );

                bool pressed = ImGui::Button(label, ImVec2(buttonWidth, buttonHeight));

                ImGui::PopStyleColor(3);
                ImGui::PopStyleVar(2);

                return pressed;
            };

        if (TitleButton("—"))
            glfwIconifyWindow(window);

        ImGui::SameLine(0.0f, 2.0f);

        if (TitleButton("□"))
            ToggleWindowMaximized(window);

        ImGui::SameLine(0.0f, 2.0f);

        if (TitleButton("X", true))
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        // Bottom separator
        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2 min = ImGui::GetWindowPos();
        ImVec2 max(min.x + ImGui::GetWindowWidth(), min.y + titlebarHeight);

        draw->AddLine(
            ImVec2(min.x, max.y - 1.0f),
            ImVec2(max.x, max.y - 1.0f),
            ImGui::GetColorU32(ImVec4(1, 1, 1, 0.07f))
        );

        // Drag window from empty titlebar space
        const bool titlebarHovered =
            ImGui::IsWindowHovered() &&
            !ImGui::IsAnyItemHovered();

        if (m_WindowDragMode == WindowDragMode::None &&
            titlebarHovered &&
            ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            ToggleWindowMaximized(window);
        }
        else if (m_WindowDragMode == WindowDragMode::None &&
            titlebarHovered &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            BeginWindowDrag(window, WindowDragMode::Move);
        }

        ImGui::End();

        ImGui::PopStyleColor();
        ImGui::PopStyleVar(3);
    }

    void HandleBorderlessResize(GLFWwindow* window, float titlebarHeight)
    {
        if (glfwGetWindowAttrib(window, GLFW_MAXIMIZED))
            return;

        constexpr float border = 6.0f;
        constexpr int minWidth = 900;
        constexpr int minHeight = 520;

        ImGuiIO& io = ImGui::GetIO();

        int winX, winY;
        int winW, winH;

        glfwGetWindowPos(window, &winX, &winY);
        glfwGetWindowSize(window, &winW, &winH);

        ImVec2 mouse = io.MousePos;

        bool left = mouse.x <= winX + border;
        bool right = mouse.x >= winX + winW - border;
        bool top = mouse.y <= winY + border;
        bool bottom = mouse.y >= winY + winH - border;

        if (left || right)
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        if (top || bottom)
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
        if ((left && top) || (right && bottom))
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
        if ((right && top) || (left && bottom))
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW);

        if (!ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            return;

        ImVec2 delta = io.MouseDelta;

        if (left)
        {
            int newW = winW - (int)delta.x;
            if (newW >= minWidth)
            {
                glfwSetWindowPos(window, winX + (int)delta.x, winY);
                glfwSetWindowSize(window, newW, winH);
            }
        }

        if (right)
        {
            int newW = winW + (int)delta.x;
            if (newW >= minWidth)
                glfwSetWindowSize(window, newW, winH);
        }

        if (top)
        {
            int newH = winH - (int)delta.y;
            if (newH >= minHeight)
            {
                glfwSetWindowPos(window, winX, winY + (int)delta.y);
                glfwSetWindowSize(window, winW, newH);
            }
        }

        if (bottom)
        {
            int newH = winH + (int)delta.y;
            if (newH >= minHeight)
                glfwSetWindowSize(window, winW, newH);
        }
    }

    void BeginWindowDrag(GLFWwindow* window, WindowDragMode mode)
    {
        if (mode == WindowDragMode::None)
            return;

        m_WindowDragMode = mode;
        m_DragStartMouse = ImGui::GetIO().MousePos;

        glfwGetWindowPos(window, &m_DragStartX, &m_DragStartY);
        glfwGetWindowSize(window, &m_DragStartW, &m_DragStartH);
    }

    void UpdateWindowDrag(GLFWwindow* window)
    {
        constexpr int minWidth = 900;
        constexpr int minHeight = 520;

        if (m_WindowDragMode == WindowDragMode::None)
            return;

        ImGuiIO& io = ImGui::GetIO();

        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            m_WindowDragMode = WindowDragMode::None;
            return;
        }

        const int dx = static_cast<int>(io.MousePos.x - m_DragStartMouse.x);
        const int dy = static_cast<int>(io.MousePos.y - m_DragStartMouse.y);

        int x = m_DragStartX;
        int y = m_DragStartY;
        int w = m_DragStartW;
        int h = m_DragStartH;

        switch (m_WindowDragMode)
        {
        case WindowDragMode::Move:
            x = m_DragStartX + dx;
            y = m_DragStartY + dy;
            break;

        case WindowDragMode::ResizeLeft:
            x = m_DragStartX + dx;
            w = m_DragStartW - dx;
            break;

        case WindowDragMode::ResizeRight:
            w = m_DragStartW + dx;
            break;

        case WindowDragMode::ResizeTop:
            y = m_DragStartY + dy;
            h = m_DragStartH - dy;
            break;

        case WindowDragMode::ResizeBottom:
            h = m_DragStartH + dy;
            break;

        case WindowDragMode::ResizeTopLeft:
            x = m_DragStartX + dx;
            y = m_DragStartY + dy;
            w = m_DragStartW - dx;
            h = m_DragStartH - dy;
            break;

        case WindowDragMode::ResizeTopRight:
            y = m_DragStartY + dy;
            w = m_DragStartW + dx;
            h = m_DragStartH - dy;
            break;

        case WindowDragMode::ResizeBottomLeft:
            x = m_DragStartX + dx;
            w = m_DragStartW - dx;
            h = m_DragStartH + dy;
            break;

        case WindowDragMode::ResizeBottomRight:
            w = m_DragStartW + dx;
            h = m_DragStartH + dy;
            break;

        default:
            break;
        }

        if (w < minWidth)
        {
            if (m_WindowDragMode == WindowDragMode::ResizeLeft ||
                m_WindowDragMode == WindowDragMode::ResizeTopLeft ||
                m_WindowDragMode == WindowDragMode::ResizeBottomLeft)
                x += w - minWidth;

            w = minWidth;
        }

        if (h < minHeight)
        {
            if (m_WindowDragMode == WindowDragMode::ResizeTop ||
                m_WindowDragMode == WindowDragMode::ResizeTopLeft ||
                m_WindowDragMode == WindowDragMode::ResizeTopRight)
                y += h - minHeight;

            h = minHeight;
        }

        glfwSetWindowPos(window, x, y);
        glfwSetWindowSize(window, w, h);
    }

    EditorRenderer::EditorRendererImpl::WindowDragMode GetResizeMode(GLFWwindow* window)
    {
        if (glfwGetWindowAttrib(window, GLFW_MAXIMIZED))
            return WindowDragMode::None;

        constexpr float border = 7.0f;

        int x, y, w, h;
        glfwGetWindowPos(window, &x, &y);
        glfwGetWindowSize(window, &w, &h);

        ImVec2 mouse = ImGui::GetIO().MousePos;

        bool left = mouse.x >= x && mouse.x <= x + border;
        bool right = mouse.x >= x + w - border && mouse.x <= x + w;
        bool top = mouse.y >= y && mouse.y <= y + border;
        bool bottom = mouse.y >= y + h - border && mouse.y <= y + h;

        if (left && top) return WindowDragMode::ResizeTopLeft;
        if (right && top) return WindowDragMode::ResizeTopRight;
        if (left && bottom) return WindowDragMode::ResizeBottomLeft;
        if (right && bottom) return WindowDragMode::ResizeBottomRight;
        if (left) return WindowDragMode::ResizeLeft;
        if (right) return WindowDragMode::ResizeRight;
        if (top) return WindowDragMode::ResizeTop;
        if (bottom) return WindowDragMode::ResizeBottom;

        return WindowDragMode::None;
    }

    void ToggleWindowMaximized(GLFWwindow* window)
    {
        if (glfwGetWindowAttrib(window, GLFW_MAXIMIZED))
            glfwRestoreWindow(window);
        else
            glfwMaximizeWindow(window);
    }

private:
    Fonts m_Fonts;
    MenuBarCallback MenuBarRenderer;
    std::shared_ptr<Texture2D> m_LogoTexture;

    WindowDragMode m_WindowDragMode = WindowDragMode::None;

    ImVec2 m_DragStartMouse{};
    int m_DragStartX = 0;
    int m_DragStartY = 0;
    int m_DragStartW = 0;
    int m_DragStartH = 0;
};

EditorRenderer::EditorRenderer(const ProjectConfig& config, const std::shared_ptr<Texture2D>& icon)
	: m_pImpl{ std::make_unique<EditorRendererImpl>(config, icon) } {}

EditorRenderer::~EditorRenderer(){}

void EditorRenderer::BeginFrame() { m_pImpl->BeginFrame(); }
void EditorRenderer::EndFrame() { m_pImpl->EndFrame(); }
void EditorRenderer::SetMenuBarCallback(const MenuBarCallback& callback){ m_pImpl->SetMenuBarCallback(callback); }