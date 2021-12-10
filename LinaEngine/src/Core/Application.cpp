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

#include "LinaPch.hpp"
#include "Core/Application.hpp"
#include "Rendering/RenderEngine.hpp"
#include "Core/PhysicsEngine.hpp"
#include "Core/AudioEngine.hpp"
#include "Input/InputEngine.hpp"
#include "Rendering/Window.hpp"
#include "Core/Layer.hpp"
#include "World/DefaultLevel.hpp"
#include "Core/Timer.hpp"
#include "ECS/Components/EntityDataComponent.hpp"
#include <chrono>

namespace Lina
{
#define PHYSICS_DELTA 0.01666

	Action::ActionDispatcher Application::s_engineDispatcher;
	Input::InputEngine* Application::s_inputEngine = nullptr;
	Graphics::RenderEngine* Application::s_renderEngine = nullptr;
	Physics::PhysicsEngine* Application::s_physicsEngine = nullptr;
	Audio::AudioEngine* Application::s_audioEngine = nullptr;
	Graphics::Window* Application::s_appWindow = nullptr;
	ECS::ECSRegistry Application::s_ecs;
	Application* Application::s_application = nullptr;


	Application::Application()
	{
		s_application = this;

		s_engineDispatcher.Initialize(Action::ActionType::EngineActionsStartIndex, Action::ActionType::EngineActionsEndIndex);

		// Make sure log event is delegated to the application.
		Log::s_onLog = std::bind(&Application::OnLog, this, std::placeholders::_1);

		LINA_TRACE("[Constructor] -> Application ({0})", typeid(*this).name());
		LINA_ASSERT(!instance, "Application already exists!");

	}

	void Application::Initialize(Graphics::WindowProperties& props)
	{
		// Get engine instances.
		s_appWindow = CreateContextWindow();
		m_inputDevice = CreateInputDevice();
		s_renderEngine = CreateRenderEngine();
		s_inputEngine = CreateInputEngine();
		s_physicsEngine = CreatePhysicsEngine();
		s_audioEngine = CreateAudioEngine();
	
		// Build main window.
		bool windowCreationSuccess = s_appWindow->CreateContext(props);
		if (!windowCreationSuccess)
		{
			LINA_ERR("Window Creation Failed!");
			return;
		}

		// Set callbacks.
		m_keyCallback = std::bind(&Application::KeyCallback, this, std::placeholders::_1, std::placeholders::_2);
		m_mouseCallback = std::bind(&Application::MouseCallback, this, std::placeholders::_1, std::placeholders::_2);
		m_WwndowResizeCallback = std::bind(&Application::OnWindowResize, this, std::placeholders::_1);
		m_windowClosedCallback = std::bind(&Application::OnWindowClose, this);
		m_drawLineCallback = std::bind(&Application::OnDrawLine, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
		m_postSceneDrawCallback = std::bind(&Application::OnPostSceneDraw, this);
		m_preDrawCallback = std::bind(&Application::OnPreDraw, this);
		m_postDrawCallback = std::bind(&Application::OnPostDraw, this);

		// Set event callback for main window.
		s_appWindow->SetKeyCallback(m_keyCallback);
		s_appWindow->SetMouseCallback(m_mouseCallback);
		s_appWindow->SetWindowResizeCallback(m_WwndowResizeCallback);
		s_appWindow->SetWindowClosedCallback(m_windowClosedCallback);
		s_renderEngine->SetPostSceneDrawCallback(m_postSceneDrawCallback);
		s_renderEngine->SetPostDrawCallback(m_postDrawCallback);
		s_renderEngine->SetPreDrawCallback(m_preDrawCallback);
		s_renderEngine->SetViewportDisplay(Vector2::Zero, s_appWindow->GetSize());

		s_ecs.Initialize();
		s_inputEngine->Initialize(s_ecs, m_mainECSPipeline, s_appWindow->GetNativeWindow(), m_inputDevice);
		s_physicsEngine->Initialize(s_ecs, m_drawLineCallback);
		s_renderEngine->Initialize(s_ecs, *s_appWindow);
		s_audioEngine->Initialize();

		// Register ECS components for cloning functionality.
		s_ecs.RegisterComponentToClone<ECS::EntityDataComponent>();
		s_ecs.RegisterComponentToClone<ECS::EntityDataComponent>();

		m_deltaTimeArray.fill(-1.0);
		m_isInPlayMode = true;
		m_running = true;
	}


	Application::~Application()
	{
		if (s_physicsEngine)
			delete s_physicsEngine;

		if (s_inputEngine)
			delete s_inputEngine;

		if (s_renderEngine)
			delete s_renderEngine;

		if (m_inputDevice)
			delete m_inputDevice;

		if (s_appWindow)
			delete s_appWindow;

		LINA_TRACE("[Destructor] -> Application ({0})", typeid(*this).name());
	}


	void Application::OnLog(Log::LogDump dump)
	{
		// Dump to cout
		std::cout << dump.m_message << std::endl;

		// Dispatch the action to any listener.
		s_engineDispatcher.DispatchAction<Log::LogDump>(Action::ActionType::MessageLogged, dump);
	}

	void Application::Run()
	{
		int frames = 0;
		int updates = 0;
		double totalFPSTime = GetTime();
		double previousFrameTime;
		double currentFrameTime = GetTime();

		while (m_running)
		{

			previousFrameTime = currentFrameTime;
			currentFrameTime = GetTime();

			m_rawDeltaTime = (currentFrameTime - previousFrameTime);
			m_smoothDeltaTime = SmoothDeltaTime(m_rawDeltaTime);
			s_inputEngine->Tick();

			updates++;
			LINA_TIMER_START("Update MS");
			UpdateGame((float)m_rawDeltaTime);
			LINA_TIMER_STOP("Update MS");
			m_updateTime = Lina::Timer::GetTimer("UPDATE MS").GetDuration();

			LINA_TIMER_START("RENDER MS");
			DisplayGame(1.0f);
			LINA_TIMER_STOP("RENDER MS");
			m_renderTime = Lina::Timer::GetTimer("RENDER MS").GetDuration();
			frames++;

			double now = GetTime();
			// Calculate FPS, UPS.
			if (now - totalFPSTime >= 1.0)
			{
				m_frameTime = m_rawDeltaTime * 1000;
				m_currentFPS = frames;
				m_currentUPS = updates;
				totalFPSTime += 1.0;
				frames = 0;
				updates = 0;
			}

			if (m_firstRun)
				m_firstRun = false;
		}

		Timer::UnloadTimers();
	}


	void Application::UpdateGame(float deltaTime)
	{
		
		// Tick physics (fixed)
		s_physicsEngine->Tick(0.02);

		// Main layers.
		for (Layer* layer : m_mainLayerStack)
			layer->Tick(deltaTime);

		// Game layers.
		if (m_isInPlayMode)
		{
			for (Layer* layer : m_playModeStack)
				layer->Tick(deltaTime);
		}

		// Level
		if (m_activeLevelExists)
			m_currentLevel->Tick(m_isInPlayMode, deltaTime);

		// Other main systems (engine or game)
		m_mainECSPipeline.UpdateSystems(deltaTime);

		// Animation, particle systems.
		s_renderEngine->Tick(deltaTime);


		// Main layers post.
		for (Layer* layer : m_mainLayerStack)
			layer->PostTick(deltaTime);

		// Game layers post.
		if (m_isInPlayMode)
		{
			for (Layer* layer : m_playModeStack)
				layer->PostTick(deltaTime);
		}

		// Level post.
		if (m_activeLevelExists)
			m_currentLevel->PostTick(m_isInPlayMode, deltaTime);
	}

	void Application::DisplayGame(float interpolation)
	{
		if (m_canRender)
		{
			// render level.
			if (m_activeLevelExists)
				s_renderEngine->Render(interpolation);

			s_renderEngine->RenderLayers();
			s_appWindow->Tick();
		}
	}

	bool Application::OnWindowClose()
	{
		m_running = false;
		s_engineDispatcher.DispatchAction<void*>(Action::ActionType::WindowClosed, 0);
		return true;
	}

	void Application::OnWindowResize(Vector2 size)
	{
		if (size.x == 0.0f || size.y == 0.0f)
			m_canRender = false;
		else
			m_canRender = true;

		s_renderEngine->SetViewportDisplay(Vector2::Zero, size);

		s_engineDispatcher.DispatchAction<Vector2>(Action::ActionType::WindowResized, size);
	}

	void Application::OnPostSceneDraw()
	{
		s_physicsEngine->OnPostSceneDraw();
		s_engineDispatcher.DispatchAction<void*>(Action::ActionType::PostSceneDraw, 0);
	}

	void Application::OnPostDraw()
	{
		s_engineDispatcher.DispatchAction<void*>(Action::ActionType::PostDraw, 0);
	}

	void Application::OnPreDraw()
	{
		s_engineDispatcher.DispatchAction<void*>(Action::ActionType::PreDraw, 0);
	}

	void Application::KeyCallback(int key, int action)
	{
		s_inputEngine->DispatchKeyAction(static_cast<Lina::Input::InputCode::Key>(key), action);
	}

	void Application::MouseCallback(int button, int action)
	{
		s_inputEngine->DispatchMouseAction(static_cast<Lina::Input::InputCode::Mouse>(button), action);
	}

	void Application::RemoveOutliers(bool biggest)
	{
		int outlier = biggest ? 0 : 10;
		int outlierIndex = -1;
		int indexCounter = 0;
		for (double d : m_deltaTimeArray)
		{
			if (d < 0)
			{
				indexCounter++;
				continue;
			}

			if (biggest)
			{
				if (d > outlier)
				{
					outlierIndex = indexCounter;
					outlier = d;
				}
			}
			else
			{
				if (d < outlier)
				{
					outlierIndex = indexCounter;
					outlier = d;
				}
			}

			indexCounter++;
		}

		if (outlierIndex != -1)
			m_deltaTimeArray[outlierIndex] = m_deltaTimeArray[outlierIndex] * -1.0;
	}

	double Application::SmoothDeltaTime(double dt)
	{
		if (m_deltaFirstFill < DELTA_TIME_HISTORY)
		{
			m_deltaFirstFill++;
		}
		else if (!m_deltaFilled)
			m_deltaFilled = true;

		m_deltaTimeArray[m_deltaTimeArrOffset] = dt;
		m_deltaTimeArrOffset++;


		if (m_deltaTimeArrOffset == DELTA_TIME_HISTORY)
			m_deltaTimeArrOffset = 0;

		if (!m_deltaFilled)
			return dt;

		// Remove the biggest & smalles 2 deltas.
		RemoveOutliers(true);
		RemoveOutliers(true);
		RemoveOutliers(false);
		RemoveOutliers(false);

		double avg = 0.0;
		int index = 0;
		for (double d : m_deltaTimeArray)
		{
			if (d < 0.0)
			{
				m_deltaTimeArray[index] = m_deltaTimeArray[index] * -1.0;
				index++;
				continue;
			}

			avg += d;
			index++;
		}

		avg /= DELTA_TIME_HISTORY - 4;

		return avg;
	}



	void Application::SetPlayMode(bool enabled)
	{
		m_isInPlayMode = enabled;

		if (enabled)
			s_inputEngine->SetCursorMode(Lina::Input::CursorMode::Disabled);
		else
			s_inputEngine->SetCursorMode(Lina::Input::CursorMode::Visible);

		s_engineDispatcher.DispatchAction<bool>(Lina::Action::ActionType::PlayModeActivation, enabled);
	}

	bool Application::InstallLevel(Lina::World::Level& level, bool loadFromFile, const std::string& path, const std::string& levelName)
	{
		UninstallLevel();

		bool install = level.Install(loadFromFile, path, levelName);
		s_engineDispatcher.DispatchAction<World::Level*>(Action::ActionType::LevelInstalled, &level);
		m_currentLevel = &level;
		InitializeLevel(level);
		return install;
	}

	void Application::InitializeLevel(Lina::World::Level& level)
	{
		m_currentLevel->Initialize();
		s_engineDispatcher.DispatchAction<World::Level*>(Action::ActionType::LevelInitialized, &level);
		m_activeLevelExists = true;
	}

	void Application::SaveLevelData(const std::string& folderPath, const std::string& fileName)
	{
		if (m_currentLevel != nullptr)
			m_currentLevel->SerializeLevelData(folderPath, fileName);
	}

	void Application::LoadLevelData(const std::string& folderPath, const std::string& fileName)
	{
		if (m_currentLevel != nullptr)
		{
			InstallLevel(*m_currentLevel, true, folderPath, fileName);
		}
	}

	void Application::RestartLevel()
	{
		InstallLevel(*m_currentLevel);
	}

	void Application::UninstallLevel()
	{
		if (m_currentLevel != nullptr)
		{
			m_currentLevel->Uninstall();
			s_ecs.each([](auto entity)
				{
					s_ecs.DestroyEntity(entity);
				});
			s_ecs.clear();
			s_engineDispatcher.DispatchAction<World::Level*>(Action::ActionType::LevelUninstalled, m_currentLevel);
			m_currentLevel = nullptr;
		}
	}

	void Application::OnDrawLine(Vector3 from, Vector3 to, Color color, float width)
	{
		s_renderEngine->DrawLine(from, to, color, width);
	}

	double Application::GetTime()
	{
		return s_appWindow->GetTime();
	}

}

