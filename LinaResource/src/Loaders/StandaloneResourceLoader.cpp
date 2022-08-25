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

#include "Loaders/StandaloneResourceLoader.hpp"
#include "Core/ResourceStorage.hpp"
#include "Utility/ResourceUtility.hpp"
#include "Data/HashSet.hpp"

namespace Lina::Resources
{
    void Lina::Resources::StandaloneResourceLoader::LoadStaticResources()
    {
        const int totalFiles = static_cast<int>(g_defaultResources.GetStaticResources().size());
        Event::EventSystem::Get()->Trigger<Event::EResourceProgressStarted>(Event::EResourceProgressStarted{.title = "Loading static packages", .totalFiles = totalFiles});
        m_packager.LoadPackage(PACKAGE_STATIC, this, Memory::ResourceAllocator::Static);
        Event::EventSystem::Get()->Trigger<Event::EResourceProgressEnded>();
    }

    void StandaloneResourceLoader::LoadEngineResources()
    {
        const int totalFiles = static_cast<int>(g_defaultResources.GetEngineResources().size());
        Event::EventSystem::Get()->Trigger<Event::EResourceProgressStarted>(Event::EResourceProgressStarted{.title = "Loading engine resources", .totalFiles = totalFiles});
        m_packager.LoadPackage(PACKAGE_ENGINERES, this, Memory::ResourceAllocator::Static);
        Event::EventSystem::Get()->Trigger<Event::EResourceProgressEnded>();
    }

    void StandaloneResourceLoader::LoadResource(TypeID tid, const String& path, bool async, Memory::ResourceAllocator alloc)
    {
        if (ResourceStorage::Get()->Exists(tid, HashedString(path.c_str()).value()))
            return;

        const ResourceTypeData& typeData = ResourceStorage::Get()->GetTypeData(tid);
        Event::EventSystem::Get()->Trigger<Event::EResourceProgressStarted>(Event::EResourceProgressStarted{.title = "Loading File", .totalFiles = 1});

        const auto& loadRes = [path, this, typeData]() {
            HashSet<StringID> set;
            set.insert(HashedString(path.c_str()).value());
            m_packager.LoadFilesFromPackage(ResourceUtility::PackageTypeToString(typeData.packageType), set, this);
            Event::EventSystem::Get()->Trigger<Event::EResourceProgressEnded>();
        };

        if (async)
            JobSystem::Get()->SilentAsync(loadRes);
        else
            loadRes();
    }

    void StandaloneResourceLoader::LoadLevelResources(const HashMap<TypeID, HashSet<String>>& resourceMap)
    {
        ResourceStorage*                   storage = ResourceStorage::Get();
        HashMap<TypeID, HashSet<StringID>> toLoad;

        // 1: Take a look at all the existing resources in storage, if it doesn't exist in the current level's resources we are loading, unload it.
        // 2: Iterate all the resourceMap we are about to load, find the one's that are not existing right now, add them to a seperate map.
        // 3: Load the separate map.

        UnloadUnusedResources(resourceMap);

        // Now iterate the resourceMap again, find the non-existing resources,
        for (auto pair : resourceMap)
        {
            const HashSet<String> set = resourceMap.at(pair.first);
            for (auto str : set)
            {
                const StringID sid = HashedString(str.c_str()).value();
                if (!storage->Exists(pair.first, sid))
                    toLoad[pair.first].insert(sid);
            }
        }

        int totalCount = 0;
        for (auto& pair : toLoad)
            totalCount += static_cast<int>(pair.second.size());

        Event::EventSystem::Get()->Trigger<Event::EResourceProgressStarted>(Event::EResourceProgressStarted{.title = "Loading Level Resources", .totalFiles = totalCount});

        // Finally, load all designated resources by type id.
        for (const auto& pair : toLoad)
        {
            const ResourceTypeData& typeData = storage->GetTypeData(pair.first);
            m_packager.LoadFilesFromPackage(ResourceUtility::PackageTypeToString(typeData.packageType), pair.second, this);
        }

        Event::EventSystem::Get()->Trigger<Event::EResourceProgressEnded>();
    }
} // namespace Lina::Resources
