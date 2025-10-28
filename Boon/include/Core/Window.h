#pragma once
#include "Core/BitFlag.h"
#include <memory>
#include <string>

namespace Boon
{
	class Window final
	{
	public:
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

		Window(const WindowDesc& desc);
		~Window();

		Window(const Window& other) = delete;
		Window(Window&& other) = delete;
		Window& operator=(const Window& other) = delete;
		Window& operator=(Window&& other) = delete;

		void Init();
		void Destroy();
		bool Update();
		void* GetApiWindow();

		void Present();

		uint32_t GetWidth() const;
		uint32_t GetHeight() const;

		static std::shared_ptr<Window> Create(const WindowDesc& desc);

	private:
		bool ShouldClose() const;
		class GlfwWindowImpl;
		std::unique_ptr<GlfwWindowImpl> m_pImpl;
	};

}