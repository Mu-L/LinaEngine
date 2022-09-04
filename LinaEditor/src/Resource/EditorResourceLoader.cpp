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

#include "Resource/EditorResourceLoader.hpp"
#include "JobSystem/JobSystem.hpp"
#include "Core/ResourceManager.hpp"
#include "EventSystem/ResourceEvents.hpp"
#include "EventSystem/EventSystem.hpp"
#include "Core/Level.hpp"
#include <iostream>
#include <fstream>
namespace Lina::Resources
{

    void EditorResourceLoader::LoadEngineResources()
    {
        auto& engineRes  = g_defaultResources.GetEngineResources();
        int   totalFiles = 0;
        for (auto& [tid, vec] : engineRes)
            totalFiles += static_cast<int>(vec.size());

        const double time = Time::GetCPUTime();
        Event::EventSystem::Get()->Trigger<Event::EResourceProgressStarted>(Event::EResourceProgressStarted{.title = "Loading engine resources", .totalFiles = totalFiles});

        for (auto& [tid, vec] : engineRes)
        {
            for (auto& path : vec)
                LoadResource(tid, path, true);
        }

        JobSystem::Get()->GetResourceExecutor().Wait();
        Event::EventSystem::Get()->Trigger<Event::EResourceProgressEnded>(Event::EResourceProgressEnded{});
        const double diff = Time::GetCPUTime() - time;
        LINA_TRACE("[Resource Loader] -> Loading engine resources took {0} seconds.", diff);
    }

    void EditorResourceLoader::LoadLevelResources(const Vector<Pair<TypeID, String>>& resources)
    {
        auto* rm = Resources::ResourceManager::Get();

        // Remove the currently loaded resources if they are not used by the next level.

        const auto& caches = rm->GetCaches();

        const TypeID levelTid = GetTypeID<World::Level>();
        for (auto [tid, cache] : caches)
        {
            if (tid != levelTid)
                cache->UnloadUnusedLevelResources(resources);
        }

        // Add the ones that are not currently loaded.
        Vector<Pair<TypeID, String>> toLoad;
        for (auto& pair : resources)
        {
            const StringID sid = TO_SID(pair.second);
            if (!rm->Exists(pair.first, sid))
                toLoad.push_back(linatl::make_pair(pair.first, pair.second));
        }

        const int    totalFiles = static_cast<int>(toLoad.size());
        const double time       = Time::GetCPUTime();
        Event::EventSystem::Get()->Trigger<Event::EResourceProgressStarted>(Event::EResourceProgressStarted{.title = "Loading level resources", .totalFiles = totalFiles});

        for (auto& pair : toLoad)
            LoadResource(pair.first, pair.second, true);

        JobSystem::Get()->GetResourceExecutor().Wait();
        Event::EventSystem::Get()->Trigger<Event::EResourceProgressEnded>(Event::EResourceProgressEnded{});
        const double diff = Time::GetCPUTime() - time;
        LINA_TRACE("[Resource Loader] -> Loading level resources took {0} seconds.", diff);
    }

    void EditorResourceLoader::LoadResources(const Vector<Pair<TypeID, String>>& resources, bool async)
    {
    }

    void EditorResourceLoader::LoadResource(TypeID tid, const String& path, bool async)
    {
        const StringID sid = HashedString(path.c_str()).value();
        auto*          rm  = ResourceManager::Get();

        if (rm->Exists(tid, sid))
            return;

        const auto& load = [rm, tid, sid, path, this]() {
            auto*     cache = rm->GetCache(tid);
            Resource* res   = cache->Create(sid);
            res->m_sid      = sid;
            res->m_path     = path;
            res->m_tid      = tid;
            res->LoadFromFile(path);

            LOCK_GUARD(m_mtx);
            Event::EventSystem::Get()->Trigger<Event::EResourceLoaded>(Event::EResourceLoaded{.tid = tid, .sid = sid});
            Event::EventSystem::Get()->Trigger<Event::EResourceProgressUpdated>(Event::EResourceProgressUpdated{.currentResource = path});
        };

        if (async)
            JobSystem::Get()->GetResourceExecutor().SilentAsync(load);
        else
            load();
    }
} // namespace Lina::Resources
