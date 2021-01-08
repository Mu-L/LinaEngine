/* 
This file is a part of: Lina Engine
https://github.com/inanevin/LinaEngine

Author: Inan Evin
http://www.inanevin.com

Copyright (c) [2018-2020] [Inan Evin]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Core/Backend/OpenGL/WindowOpenGL.hpp"
#include "Core/Log.hpp"
#include "EventSystem/EventSystem.hpp"

#include <glad/glad.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

namespace Lina::Graphics
{

	static void GLFWErrorCallback(int error, const char* desc)
	{
		LINA_ERR("[Window GLFW] -> GLFW Error: {0} Description: {1} ", error, desc);
	}

	WindowOpenGL::~WindowOpenGL()
	{

	}

	void WindowOpenGL::Terminate()
	{
		LINA_TRACE("[Window GLFW] -> Terminating...");
		glfwTerminate();
	}


	void WindowOpenGL::WaitEvents()
	{
		glfwGetFramebufferSize(m_glfwWindow, &m_windowProperties.m_width, &m_windowProperties.m_height);
		glfwWaitEvents();
	}
	void WindowOpenGL::OnWindowResize(int w, int h)
	{
		m_windowProperties.m_width = w;
		m_windowProperties.m_height = h;
		m_eventSys->Trigger<Event::EWindowResized>({ (void*)w, m_windowProperties });
	}

	void WindowOpenGL::SwapBuffers()
	{
		glfwSwapBuffers(m_glfwWindow);
		glfwPollEvents();
	}


	void WindowOpenGL::SetReferences(Event::EventSystem* eventSys, WindowProperties& props)
	{
		m_eventSys = eventSys;
		m_windowProperties = props;
	}

	bool WindowOpenGL::CreateContext()
	{
		glfwInit();

		// Initialize glfw & set window hints
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_SAMPLES, 4);
		glfwWindowHint(GLFW_DECORATED, m_windowProperties.m_decorated);
		glfwWindowHint(GLFW_RESIZABLE, m_windowProperties.m_resizable);

		if (m_windowProperties.m_windowState == WindowState::Iconified)
			glfwWindowHint(GLFW_ICONIFIED, GLFW_TRUE);
		else if (m_windowProperties.m_windowState == WindowState::Maximized)
			glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

		if (m_windowProperties.m_fullscreen)
		{
			m_windowProperties.m_width = glfwGetVideoMode(glfwGetPrimaryMonitor())->width;
			m_windowProperties.m_height = glfwGetVideoMode(glfwGetPrimaryMonitor())->height;
		}

		glfwSetErrorCallback(GLFWErrorCallback);

		// Build window
		m_glfwWindow = (glfwCreateWindow(m_windowProperties.m_width, m_windowProperties.m_height, m_windowProperties.m_title.c_str(), m_windowProperties.m_fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL));
			
		if (!m_glfwWindow)
		{
			// Assert window creation.
			LINA_ERR("[Window GLFW] -> GLFW could not initialize!");
			return false;
		}


		m_eventSys->Trigger<Event::EWindowContextCreated>({ (void*)m_glfwWindow });
		LINA_TRACE("[Window GLFW] -> Created context ready for OpenGL");

		// Set context.
		glfwMakeContextCurrent(m_glfwWindow);

		// Load glad
		bool loaded = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		if (!loaded)
		{
			LINA_ERR("[Window GLFW] -> GLAD Loader failed!");
			return false;
		}

		// Update OpenGL about the window data.
		glViewport(0, 0, m_windowProperties.m_width, m_windowProperties.m_height);

		// set user pointer for callbacks.
		glfwSetWindowUserPointer(m_glfwWindow, this);

		glfwSetWindowSizeCallback(m_glfwWindow, [](GLFWwindow* w, int wi, int he)
			{
				static_cast<WindowOpenGL*>(glfwGetWindowUserPointer(w))->OnWindowResize(wi, he);
			});

		glfwSetWindowPosCallback(m_glfwWindow, [](GLFWwindow* w, int xpos, int ypos)
			{
				static_cast<WindowOpenGL*>(glfwGetWindowUserPointer(w))->m_eventSys->Trigger<Event::EWindowMoved>({ (void*)w, xpos, ypos });
			}
		);

		glfwSetWindowCloseCallback(m_glfwWindow, [](GLFWwindow* w)
			{

				static_cast<WindowOpenGL*>(glfwGetWindowUserPointer(w))->m_eventSys->Trigger<Event::EWindowClosed>({ (void*)w });
			}
		);

		glfwSetWindowFocusCallback(m_glfwWindow, [](GLFWwindow* w, int focus)
			{
				static_cast<WindowOpenGL*>(glfwGetWindowUserPointer(w))->m_eventSys->Trigger<Event::EWindowFocused>({ (void*)w, focus });
			}
		);


		glfwSetWindowRefreshCallback(m_glfwWindow, [](GLFWwindow* w)
			{
				static_cast<WindowOpenGL*>(glfwGetWindowUserPointer(w))->m_eventSys->Trigger<Event::EWindowRefreshed>({ (void*)w });
			}
		);

		glfwSetWindowMaximizeCallback(m_glfwWindow, [](GLFWwindow* w, int maximized)
			{
				static_cast<WindowOpenGL*>(glfwGetWindowUserPointer(w))->m_eventSys->Trigger<Event::EWindowMaximized>({ (void*)w, maximized });
			}
		);

		glfwSetWindowIconifyCallback(m_glfwWindow, [](GLFWwindow* w, int iconified)
			{
				static_cast<WindowOpenGL*>(glfwGetWindowUserPointer(w))->m_eventSys->Trigger<Event::EWindowIconified>({ (void*)w, iconified });
			}
		);

		glfwSetKeyCallback(m_glfwWindow, [](GLFWwindow* w, int k, int s, int a, int m)
			{
				static_cast<WindowOpenGL*>(glfwGetWindowUserPointer(w))->m_eventSys->Trigger<Event::EWindowKeyCallback>({ (void*)w, k, s,a, m });
			}
		);

		glfwSetMouseButtonCallback(m_glfwWindow, [](GLFWwindow* w, int b, int a, int m)
			{
				static_cast<WindowOpenGL*>(glfwGetWindowUserPointer(w))->m_eventSys->Trigger<Event::EWindowMouseButtonCallback>({ (void*)w, b, a, m });
			}
		);

		glfwSetScrollCallback(m_glfwWindow, [](GLFWwindow* w, double xoff, double yoff)
			{
				static_cast<WindowOpenGL*>(glfwGetWindowUserPointer(w))->m_eventSys->Trigger<Event::EWindowMouseScrollCallback>({ (void*)w, xoff, yoff });
			}
		);

		glfwSetCursorPosCallback(m_glfwWindow, [](GLFWwindow* w, double xpos, double ypos)
			{
				static_cast<WindowOpenGL*>(glfwGetWindowUserPointer(w))->m_eventSys->Trigger<Event::EWindowMouseCursorPosCallback>({ (void*)w, xpos, ypos });
			}
		);

		m_window = static_cast<void*>(m_glfwWindow);
		return true;
	}



	void WindowOpenGL::SetVsync(bool enabled)
	{
		glfwSwapInterval(enabled);
	}

	void WindowOpenGL::SetSize(const Vector2& newSize)
	{
		glfwSetWindowSize(m_glfwWindow, (int)newSize.x, (int)newSize.y);
		m_windowProperties.m_width = (unsigned int)newSize.x;
		m_windowProperties.m_height = (unsigned int)newSize.y;
	}

	void WindowOpenGL::SetPos(const Vector2& newPos)
	{
		m_windowProperties.m_xPos = (unsigned int)newPos.x;
		m_windowProperties.m_yPos = (unsigned int)newPos.y;
		glfwSetWindowPos(m_glfwWindow, (int)newPos.x, (int)newPos.y);
	}

	void WindowOpenGL::SetPosCentered(const Vector2 newPos)
	{
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		SetPos(Lina::Vector2(mode->width / 2.0f + newPos.x - m_windowProperties.m_width / 2.0f, mode->height / 2.0f + newPos.y - m_windowProperties.m_height / 2.0f));
	}


	void WindowOpenGL::Iconify()
	{
		m_windowProperties.m_windowState = WindowState::Iconified;
		glfwIconifyWindow(m_glfwWindow);
	}

	void WindowOpenGL::Maximize()
	{
		if (m_windowProperties.m_windowState != WindowState::Maximized)
		{
			m_windowProperties.m_windowState = WindowState::Maximized;
			glfwMaximizeWindow(m_glfwWindow);
		}
		else
		{
			m_windowProperties.m_windowState = WindowState::Normal;
			glfwRestoreWindow(m_glfwWindow);
		}

		SetVsync(false);
	}

}