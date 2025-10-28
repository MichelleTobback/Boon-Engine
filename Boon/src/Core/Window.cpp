#include "Core/Window.h"
#include <GLFW/glfw3.h>
#include "Core/Assert.h"
#include <iostream>
#include <functional>

namespace Boon
{
	//========================================================
	// Impl
	//========================================================
	class Window::GlfwWindowImpl final
	{
	public:
		GlfwWindowImpl(const WindowDesc& desc)
			: m_Desc{ desc }
		{

		}

		~GlfwWindowImpl()
		{
			
		}
		
		static std::unique_ptr<GlfwWindowImpl> Create(const WindowDesc& desc)
		{
			return std::make_unique<GlfwWindowImpl>(desc);
		}

		void Init()
		{
			static bool glfwInitialized{ false };
			if (!glfwInitialized)
			{
				if (!glfwInit())
				{
					std::cout << "Failed to initialize glfw\n";
					return;
				}

				glfwInitialized = true;
			}

			m_pWindow = glfwCreateWindow(static_cast<int>(m_Desc.width), static_cast<int>(m_Desc.height), m_Desc.name.c_str(), nullptr, nullptr);
			glfwMakeContextCurrent(m_pWindow);

			glfwSetWindowUserPointer(m_pWindow, this);

			auto* pInstance = this;
			glfwSetFramebufferSizeCallback(m_pWindow, [](GLFWwindow* win, int w, int h)
				{
					GlfwWindowImpl* pInstance = static_cast<GlfwWindowImpl*>(glfwGetWindowUserPointer(win));
					if (pInstance)
						pInstance->OnResize(w, h);
				});
		}

		void Destroy()
		{
			glfwDestroyWindow(m_pWindow);
			glfwTerminate();
		}

		void Update()
		{
			glfwPollEvents();
		}

		bool ShouldClose() const
		{
			return glfwWindowShouldClose(m_pWindow);
		}

		void* GetApiWindow()
		{
			return m_pWindow;
		}

		void Present()
		{
			glfwSwapBuffers(m_pWindow);
		}

		uint32_t GetWidth() const
		{
			return m_Desc.width;
		}

		uint32_t GetHeight() const
		{
			return m_Desc.height;
		}

	private:
		void OnResize(int width, int height)
		{
			m_Desc.width = width;
			m_Desc.height = height;
			glViewport(0, 0, width, height);
		}

		WindowDesc m_Desc;
		GLFWwindow* m_pWindow{ nullptr };
		bool m_Resize{ true };
	};

	//========================================================
	// window
	//========================================================

	Window::Window(const WindowDesc& desc)
		: m_pImpl{ GlfwWindowImpl::Create(desc) }
	{
		Init();
	}

	Window::~Window()
	{
	}

	void Window::Init()
	{
		m_pImpl->Init();
	}

	void Window::Destroy()
	{
		m_pImpl->Destroy();
	}

	bool Window::Update()
	{
		m_pImpl->Update();
		return ShouldClose();
	}

	bool Window::ShouldClose() const
	{
		return m_pImpl->ShouldClose();
	}

	void* Window::GetApiWindow()
	{
		return m_pImpl->GetApiWindow();
	}

	void Window::Present()
	{
		return m_pImpl->Present();
	}

	uint32_t Window::GetWidth() const
	{
		return m_pImpl->GetWidth();
	}

	uint32_t Window::GetHeight() const
	{
		return m_pImpl->GetHeight();
	}

	std::shared_ptr<Window> Window::Create(const WindowDesc& desc)
	{
		return std::make_shared<Window>(desc);
	}
}
