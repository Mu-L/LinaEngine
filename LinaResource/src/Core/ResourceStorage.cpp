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

#include "Core/ResourceStorage.hpp"
#include "Data/DataCommon.hpp"
#include "EventSystem/EventSystem.hpp"
#include "EventSystem/ResourceEvents.hpp"
#include "Utility/ResourceUtility.hpp"
#include "Core/ResourceHandle.hpp"
#include "Loaders/EditorResourceLoader.hpp"
#include "Loaders/StandaloneResourceLoader.hpp"

namespace Lina::Resources
{

    ResourceStorage* ResourceStorage::s_instance = nullptr;

    void ResourceStorage::Initialize(ApplicationInfo& appInfo)
    {
        LINA_TRACE("[Initialization] -> Resource Storage {0}", typeid(*this).name());

        Event::EventSystem::Get()->Connect<Event::EResourcePathUpdated, &ResourceStorage::OnResourcePathUpdated>(this);
        Event::EventSystem::Get()->Connect<Event::EResourceReloaded, &ResourceStorage::OnResourceReloaded>(this);
        Event::EventSystem::Get()->Connect<Event::EResourceUnloaded, &ResourceStorage::OnResourceUnloaded>(this);

        m_appInfo = appInfo;

        if (appInfo.appMode == ApplicationMode::Editor)
            m_loader = new EditorResourceLoader();
        else
            m_loader = new StandaloneResourceLoader();

        m_loader->Initialize(appInfo);

        ResourceUtility::InitializeWorkingDirectory();

        // Fill the folder structure.
        if (appInfo.appMode == ApplicationMode::Editor)
            ResourceUtility::ScanRootFolder();
    }

    void ResourceStorage::Shutdown()
    {
        LINA_TRACE("[Shutdown] -> Resource Storage {0}", typeid(*this).name());

        for (auto& [resType, cache] : m_resources)
        {
            for (auto& [sid, ptr] : cache)
                GetTypeData(resType).deleteFunc(ptr);

            cache.clear();
        }
        m_resources.clear();
        m_resourceTypes.clear();

        delete ResourceUtility::s_rootFolder;
        delete m_loader;
    }

    void ResourceStorage::Load(TypeID tid, const String& path)
    {
        GetLoader()->LoadResource(tid, path);
    }

    void ResourceStorage::UnloadAll()
    {
        LINA_TRACE("[Resource Storage] -> Unloading all.");

        for (auto& [tid, cache] : m_resources)
        {
            for (auto& [sid, ptr] : cache)
                Unload(tid, sid);
        }
    }

    void ResourceStorage::OnResourcePathUpdated(const Event::EResourcePathUpdated& ev)
    {
        // Find the resources with the updated sid, add the new string ID & delete the previous one.
        for (auto& [typeID, cache] : m_resources)
        {
            for (auto& [stringID, ptr] : cache)
            {
                if (stringID == ev.previousStringID)
                {
                    IResource* res = static_cast<IResource*>(ptr);
                    res->m_sid     = ev.newStringID;
                    res->m_path    = ev.newPath;

                    cache[ev.newStringID] = ptr;
                    cache.erase(stringID);

                    break;
                }
            }
        }
    }

    void ResourceStorage::OnResourceReloaded(const Event::EResourceReloaded& ev)
    {
    }

    void ResourceStorage::OnResourceUnloaded(const Event::EResourceUnloaded& ev)
    {
    }

    TypeID ResourceStorage::GetTypeIDFromExtension(const String& extension)
    {
        bool   found = false;
        TypeID tid   = -1;

        for (auto& typeCachePair : m_resourceTypes)
        {
            auto& extensions = typeCachePair.second.associatedExtensions;

            for (int i = 0; i < extensions.size(); i++)
            {
                if (extensions[i].compare(extension) == 0)
                {
                    tid   = typeCachePair.first;
                    found = true;
                    break;
                }
            }

            if (found)
                break;
        }

        return tid;
    }

    String ResourceStorage::GetPathFromSID(StringIDType sid)
    {
#ifdef LINA_ENABLE_LOGGING

        for (auto& [tid, cache] : m_resources)
        {
            for (auto& [s, ptr] : cache)
            {
                if (s == sid)
                {
                    IResource* res = static_cast<IResource*>(ptr);
                    return res->GetPath();
                }
            }
        }

        return "-path-not-found-";

#else
        return "";
#endif
    }

    void ResourceStorage::AddResourceHandle(ResourceHandleBase* handle)
    {
        m_resourceHandles.push_back(handle);
    }

    void ResourceStorage::RemoveResourceHandle(ResourceHandleBase* handle)
    {
        const auto it = linatl::find(m_resourceHandles.begin(), m_resourceHandles.end(), handle);
        if (it != m_resourceHandles.end())
            m_resourceHandles.erase(it);
    }

} // namespace Lina::Resources
