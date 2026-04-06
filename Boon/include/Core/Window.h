#pragma once
#include "Core/BitFlag.h"
#include <memory>
#include <string>

namespace Boon
{
	class Window final
	{
	public:
		/**
		 * @brief Cross-platform window abstraction.
		 *
		 * Window encapsulates the platform specific window implementation and
		 * exposes a minimal API for initialization, event polling and presentation.
		 */
		enum class WinConfigFlag
		{
			None = 0, Vsync
		};

		struct WindowDesc
		{
			uint32_t width{};
			uint32_t height{};
			std::string name{};
			WinConfigFlag flags{};
		};

		/**
		 * @brief Create a native Window instance described by WindowDesc.
		 * @param desc Window creation parameters.
		 */
		Window(const WindowDesc& desc);

		/**
		 * @brief Destroy the Window and release platform resources.
		 */
		~Window();

		Window(const Window& other) = delete;
		Window(Window&& other) = delete;
		Window& operator=(const Window& other) = delete;
		Window& operator=(Window&& other) = delete;

		/**
		 * @brief Initialize the underlying platform window and graphics context.
		 */
		void Init();

		/**
		 * @brief Destroy the underlying platform window and release resources.
		 */
		void Destroy();

		/**
		 * @brief Poll window events and update internal state.
		 *
		 * @return False when the window should close, true otherwise.
		 */
		bool Update();

		/**
		 * @brief Get the native platform window handle.
		 * @return Void pointer to the native window (platform-specific).
		 */
		void* GetApiWindow();

		/**
		 * @brief Present the current backbuffer to the screen.
		 */
		void Present();

		/**
		 * @brief Get the current width of the window in pixels.
		 * @return Window width in pixels.
		 */
		uint32_t GetWidth() const;

		/**
		 * @brief Get the current height of the window in pixels.
		 * @return Window height in pixels.
		 */
		uint32_t GetHeight() const;

		/**
		 * @brief Factory to create a shared Window instance.
		 * @param desc WindowDesc that describes the window to create.
		 * @return Shared pointer to the created Window.
		 */
		static std::shared_ptr<Window> Create(const WindowDesc& desc);

	private:
		bool ShouldClose() const;
		class GlfwWindowImpl;
		std::unique_ptr<GlfwWindowImpl> m_pImpl;
	};

}