/*
This file is a part of: Lina Engine
https://github.com/inanevin/LinaEngine

Author: Inan Evin
http://www.inanevin.com

Copyright (c) [2018-] [Inan Evin]

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

#pragma once

#ifndef Engine_HPP
#define Engine_HPP

// Headers here.
#include "Core/AudioEngine.hpp"
#include "Core/InputEngine.hpp"
#include "Core/MessageBus.hpp"
#include "Core/PhysicsEngine.hpp"
#include "Core/RenderEngine.hpp"
#include "Core/LevelManager.hpp"
#include "Core/ResourceStorage.hpp"
#include "ECS/System.hpp"
#include "ECS/SystemList.hpp"
#include "EventSystem/EventSystem.hpp"
#include "Data/Vector.hpp"
#include "JobSystem/JobSystem.hpp"

#define DELTA_TIME_HISTORY 11

namespace Lina
{
    namespace Resources
    {
        class ResourceDataManager;
    }
    class Application;
    class EngineSettings;
    class RenderSettings;

    class Engine
    {

    public:
        static Engine* Get()
        {
            return s_engine;
        }

        void   PackageProject(const String& path);
        double GetElapsedTime();
        void   SetPlayMode(bool enabled);
        void   SetIsPaused(bool paused);
        void   SkipNextFrame();

        /// <summary>
        /// Tries to cap the application to a certain frames-per-second. Accurate up to 240~ frames depending on the platform.
        /// Setting 0 removes the cap.
        /// Frame limit is first and foremost depending on GPU and vsync mode. After rendering and refreshing, if there are still some time to cap,
        /// then this frame limit is used.
        /// </summary>
        /// <param name="maxFPS"></param>
        inline void SetFrameLimit(int maxFPS)
        {
            m_frameLimit        = maxFPS;
            m_frameLimitSeconds = 1.0 / static_cast<double>(m_frameLimit);
        }

        inline EngineSettings* GetEngineSettings()
        {
            return m_engineSettings;
        }

        inline RenderSettings* GetRenderSettings()
        {
            return m_renderSettings;
        }
        inline void AddToMainPipeline(ECS::System& system)
        {
            m_mainECSPipeline.AddSystem(system);
        }
        inline int GetCurrentFPS()
        {
            return m_currentFPS;
        }
        inline int GetCurrentUPS()
        {
            return m_currentUPS;
        }
        inline double GetRawDelta()
        {
            return m_rawDeltaTime;
        }
        inline double GetSmoothDelta()
        {
            return m_smoothDeltaTime;
        }
        inline double GetFrameTime()
        {
            return m_frameTime;
        }
        inline double GetRenderTime()
        {
            return m_renderTime;
        }
        inline double GetUpdateTime()
        {
            return m_updateTime;
        }
        inline bool GetPlayMode()
        {
            return m_isInPlayMode;
        }
        inline bool GetIsPaused()
        {
            return m_paused;
        }
        inline bool GetPauseMode()
        {
            return m_paused;
        }
        inline const ECS::SystemList& GetPipeline()
        {
            return m_mainECSPipeline;
        }

    private:
        friend class Application;

        Engine()  = default;
        ~Engine() = default;

        void   Initialize(ApplicationInfo& appInfo);
        void   Run();
        void   RunSimulation(float deltaTime);
        void   RemoveOutliers(bool biggest);
        void   RegisterResourceTypes();
        double SmoothDeltaTime(double dt);

    private:
        static Engine*                  s_engine;
        Resources::ResourceStorage      m_resourceStorage;
        Physics::PhysicsEngine          m_physicsEngine;
        Audio::AudioEngine              m_audioEngine;
        Input::InputEngine              m_inputEngine;
        Event::EventSystem              m_eventSystem;
        ECS::SystemList                 m_mainECSPipeline;
        World::LevelManager             m_levelManager;
        Graphics::RenderEngine          m_renderEngine;
        Resources::ResourceDataManager* m_resourceDataManager;
        MessageBus                      m_messageBus;
        JobSystem                       m_jobSystem;
        bool                            m_running           = false;
        bool                            m_canRender         = true;
        bool                            m_firstRun          = true;
        bool                            m_isInPlayMode      = false;
        bool                            m_paused            = false;
        bool                            m_shouldSkipFrame   = false;
        int                             m_frameLimit        = 0;
        double                          m_frameLimitSeconds = 0;
        // Performance & variable stepping
        int                                    m_currentFPS = 0;
        int                                    m_currentUPS = 0;
        double                                 m_updateTime = 0;
        double                                 m_renderTime = 0;
        double                                 m_frameTime  = 0;
        double                                 m_smoothDeltaTime;
        double                                 m_rawDeltaTime;
        std::array<double, DELTA_TIME_HISTORY> m_deltaTimeArray;
        uint8                                  m_deltaTimeArrIndex  = 0;
        uint8                                  m_deltaTimeArrOffset = 0;
        int                                    m_deltaFirstFill     = 0;
        bool                                   m_deltaFilled        = false;
        double                                 m_startTime          = 0.0;
        float                                  m_physicsAccumulator = 0.0f;
        EngineSettings*                        m_engineSettings     = nullptr;
        RenderSettings*                        m_renderSettings     = nullptr;
    };
} // namespace Lina

#endif
