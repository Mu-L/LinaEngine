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

#ifndef ResourceStorage_HPP
#define ResourceStorage_HPP

// Headers here.
#include "Core/ResourcePackager.hpp"
#include "Core/ResourceCommon.hpp"
#include "Utility/StringId.hpp"
#include "Log/Log.hpp"
#include "EventSystem/ResourceEvents.hpp"
#include "EventSystem/EventSystem.hpp"
#include "Math/Color.hpp"
#include "Data/Vector.hpp"
#include "Data/HashMap.hpp"
#include "Data/Set.hpp"
#include "Core/IResource.hpp"
#include "Core/PlatformMacros.hpp"

namespace Lina
{
    class Engine;

    namespace Event
    {
        struct EResourcePathUpdated;
        struct EResourceReloaded;
        struct EResourceUnloaded;
    } // namespace Event
} // namespace Lina

namespace Lina::Resources
{

    class ResourceLoader;
    class ResourceHandleBase;

    struct ResourceTypeData
    {
        int                loadPriority = 0;
        PackageType        packageType  = PackageType::Custom;
        ResourceCreateFunc createFunc;
        ResourceDeleteFunc deleteFunc;
        Vector<String>     associatedExtensions;
        Color              debugColor = Color::White;
    };

#ifdef LINA_MT
    typedef ParallelHashMapMutex<StringIDType, void*>      Cache;
    typedef ParallelHashMapMutex<TypeID, Cache>            ResourceMap;
    typedef ParallelHashMapMutex<TypeID, ResourceTypeData> ResourceTypeMap;
#else
    typedef HashMap<StringIDType, void*>      Cache;
    typedef HashMap<TypeID, Cache>            ResourceMap;
    typedef HashMap<TypeID, ResourceTypeData> ResourceTypeMap;
#endif

    class ResourceStorage
    {

    public:
        static ResourceStorage* Get()
        {
            return s_instance;
        }

        /// <summary>
        /// Returns true if the given string ID exists in the cache for the type T.
        /// </summary>
        template <typename T>
        bool Exists(StringIDType sid)
        {
            auto& cache = m_resources[GetTypeID<T>()];
            return cache.find(sid) != cache.end();
        }

        /// <summary>
        /// Returns true if the given path's string ID exists in the cache for the type T.
        /// </summary>
        template <typename T>
        bool Exists(const String& path)
        {
            return Exists<T>(StringID(path.c_str()).value());
        }

        /// <summary>
        /// Returns true if the given path's string ID exists in the cache for the type T.
        /// </summary>
        bool Exists(TypeID tid, StringIDType sid)
        {
            auto& cache = m_resources[tid];
            return cache.find(sid) != cache.end();
        }

        /// <summary>
        /// Returns the resource with the given type and string ID. Will return uninitialized ptr if doesn't exist.
        /// If you are not sure about the resource's existance, use Exists() method first.
        /// </summary>
        template <typename T>
        T* GetResource(StringIDType sid)
        {
            auto& cache = m_resources[GetTypeID<T>()];
            return (T*)cache.at(sid);
        }

        /// <summary>
        /// Returns the resource with the given type and path. Will return uninitialized ptr if doesn't exist.
        /// If you are not sure about the resource's existance, use Exists() method first.
        /// </summary>
        template <typename T>
        T* GetResource(const String& path)
        {
            return GetResource<T>(StringID(path.c_str()).value());
        }

        /// <summary>
        /// Adds the given resources to it's respective cache, once added you don't have to manage the resource's lifetime
        /// as it will be managed by the storage object.
        /// </summary>
        void Add(void* resource, TypeID tid, StringIDType sid)
        {
            auto& cache = m_resources[tid];
            cache[sid]  = resource;
        }

        /// <summary>
        /// Unloads the resource from the type T cache, also deletes the underlying pointer.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="sid"></param>
        template <typename T>
        void Unload(const StringIDType sid)
        {
            TypeID tid   = GetTypeID<T>();
            auto&  cache = m_resources[tid];

            if (!Exists<T>(sid))
            {
                LINA_WARN("Resource you are trying to unload does not exists! {0}", GetPathFromSID(sid));
                return;
            }

            LINA_TRACE("[Resource Storage] -> Unloading resource: {0}", GetPathFromSID(sid));
            Event::EventSystem::Get()->Trigger<Event::EResourceUnloaded>(Event::EResourceUnloaded{sid, tid});

            auto* ptr = cache[sid];
            GetTypeData(tid).deleteFunc(cache[sid]);
            cache.erase(sid);
        }

        /// <summary>
        /// Unloads the resource from the type T cache, also deletes the underlying pointer.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="sid"></param>
        template <typename T>
        void Unload(const String& path)
        {
            Unload<T>(StringID(path.c_str()).value());
        }

        /// <summary>
        /// Unloads the resource from the type T cache, also deletes the underlying pointer.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="sid"></param>
        void Unload(TypeID tid, StringIDType sid)
        {
            auto& cache = m_resources[tid];

            if (!Exists(tid, sid))
            {
                LINA_WARN("Resource you are trying to unload does not exists! {0}", GetPathFromSID(sid));
                return;
            }

            LINA_TRACE("[Resource Storage] -> Unloading resource: {0}", GetPathFromSID(sid));
            Event::EventSystem::Get()->Trigger<Event::EResourceUnloaded>(Event::EResourceUnloaded{sid, tid});

            auto* ptr = cache[sid];
            GetTypeData(tid).deleteFunc(cache[sid]);
            cache.erase(sid);
        }

        /// <summary>
        /// Unloads the resource from the type T cache, also deletes the underlying pointer.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="sid"></param>
        void Unload(TypeID tid, const String& path)
        {
            Unload(tid, StringID(path.c_str()).value());
        }

        /// <summary>
        /// Unloads all resources, use with caution!
        /// </summary>
        void UnloadAll();

        /// <summary>
        /// Loads a single resource, preferably use for runtime-loading.
        /// </summary>
        /// <param name="tid"></param>
        /// <param name="path"></param>
        void Load(TypeID tid, const String& path);

        /// <summary>
        /// Registering a resource will allow the ResourceManager to load & save the resource during editor, run-time and
        /// packaging processes. Resource Manager will pick-up the resource type from type ID and associate it with the given
        /// extensions.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="extensions"></param>
        template <typename T>
        void RegisterResource(const ResourceTypeData& data)
        {
            TypeID tid           = GetTypeID<T>();
            m_resourceTypes[tid] = data;
        }

        template <typename T>
        const Cache& GetCache() const
        {
            return m_resources[GetTypeID<T>()];
        }

        const Cache& GetCache(TypeID tid)
        {
            return m_resources[tid];
        }

        String GetPathFromSID(StringIDType sid);

        /// <summary>
        /// Returns the type ID of the resource associated with the given extension.
        /// </summary>
        TypeID GetTypeIDFromExtension(const String& extension);

        /// <summary>
        /// Returns the binded type data with the resource type.
        /// </summary>
        inline ResourceTypeData& GetTypeData(TypeID tid)
        {
            return m_resourceTypes[tid];
        }

        /// <summary>
        /// Returns the color associated with the given type.
        /// </summary>
        template <typename T>
        Color GetTypeColor()
        {
            return m_resourceTypes[GetTypeID<T>()].debugColor;
        }

        /// <summary>
        /// Returns the color associated with the given type.
        /// </summary>
        Color GetTypeColor(TypeID tid)
        {
            return m_resourceTypes[tid].debugColor;
        }

        /// <summary>
        /// Returns whether the templated type is registered or not.
        /// </summary>
        template <typename T>
        bool IsTypeRegistered()
        {
            return m_resourceTypes.find(GetTypeID<T>()) != m_resourceTypes.end();
        }

        /// <summary>
        /// Returns whether the templated type is registered or not.
        /// </summary>
        bool IsTypeRegistered(TypeID tid)
        {
            return m_resourceTypes.find(tid) != m_resourceTypes.end();
        }

        /// <summary>
        /// All registered resource types.
        /// </summary>
        /// <returns></returns>
        inline const ResourceTypeMap& GetResourceTypes() const
        {
            return m_resourceTypes;
        }

        inline ResourceLoader* GetLoader()
        {
            return m_loader;
        }

        void AddResourceHandle(ResourceHandleBase* handle);
        void RemoveResourceHandle(ResourceHandleBase* handle);

    private:
        friend class Engine;

        ResourceStorage()  = default;
        ~ResourceStorage() = default;

        void Initialize(ApplicationInfo& appInfo);
        void Shutdown();
        void OnResourcePathUpdated(const Event::EResourcePathUpdated& ev);
        void OnResourceReloaded(const Event::EResourceReloaded& ev);
        void OnResourceUnloaded(const Event::EResourceUnloaded& ev);

    private:
        Vector<ResourceHandleBase*> m_resourceHandles;
        static ResourceStorage*     s_instance;
        ResourceMap                 m_resources;
        ResourceTypeMap             m_resourceTypes;
        ResourceLoader*             m_loader = nullptr;
        ApplicationInfo             m_appInfo;
    };
} // namespace Lina::Resources

#endif
