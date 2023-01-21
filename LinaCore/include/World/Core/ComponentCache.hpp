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

#ifndef ComponentCache_HPP
#define ComponentCache_HPP

#include "CommonWorld.hpp"
#include "Entity.hpp"
#include "Core/SizeDefinitions.hpp"
#include "Data/IDList.hpp"
#include "Serialization/HashMapSerialization.hpp"

#include <functional>

namespace Lina
{
    class EntityWorld;
#define COMPONENT_POOL_SIZE 100

    class ComponentCacheBase : public ISerializable
    {
    protected:
        friend class EntityWorld;

        virtual ComponentCacheBase* CopyCreate()                 = 0;
        virtual void                OnEntityDestroyed(Entity* e) = 0;
        virtual void                Destroy()                    = 0;
        Entity**                    m_entities                   = nullptr;
    };

    template <typename T> class ComponentCache : public ComponentCacheBase
    {
    public:
        ComponentCache() : m_components(IDList<T*>(COMPONENT_POOL_SIZE, nullptr))
        {
            m_allocatorPool = MemoryManager::Get().CreateAllocatorPoolThreadSafe(AllocatorType::Pool, sizeof(T) * COMPONENT_POOL_SIZE, AllocatorPoolGrowPolicy::UseInitialSize, 0, sizeof(T));
        }

        virtual ~ComponentCache()
        {
            MemoryManager::Get().DestroyAllocatorPoolThreadSafe(m_allocatorPool);
        }

    protected:
        // When copying the world.
        virtual ComponentCacheBase* CopyCreate() override
        {
            ComponentCache<T>* newCache = new ComponentCache<T>();

            int i = 0;
            for (auto comp : m_components)
            {
                if (comp != nullptr)
                {
                    T* newComp = new (newCache->m_allocatorPool->Allocate(sizeof(T))) T();
                    *newComp   = *comp;
                    newComp->OnComponentCreated();
                    newCache->m_components.AddItem(newComp, i);
                }
                i++;
            }

            return newCache;
        }

        // Mutators
        inline T* AddComponent(Entity* e)
        {
            T* comp          = new (m_allocatorPool->Allocate(sizeof(T))) T();
            comp->m_entityID = e->GetID();
            comp->m_entity   = e;
            comp->OnComponentCreated();
            m_components.AddItem(comp);
            return comp;
        }

        inline T* GetComponent(Entity* e)
        {
            for (auto* comp : m_components)
            {
                if (comp != nullptr && comp->m_entityID == e->GetID())
                    return comp;
            }
            return nullptr;
        }

        inline void DestroyComponent(Entity* e)
        {
            uint32 index = 0;
            for (auto* comp : m_components)
            {
                if (comp != nullptr && comp->m_entityID == e->GetID())
                {
                    comp->OnComponentDestroyed();
                    comp->~T();
                    m_components.RemoveItem(index);
                    m_allocatorPool->Free(comp);
                    break;
                }
                index++;
            }
        }

        virtual void OnEntityDestroyed(Entity* e) override
        {
            DestroyComponent(e);
        }

        virtual void Destroy() override
        {
            for (auto* comp : m_components)
            {
                if (comp != nullptr)
                {
                    comp->OnComponentDestroyed();
                    comp->~T();
                    m_allocatorPool->Free(comp);
                }
            }

            m_components.Clear();
        }

        // Inherited via ComponentCacheBase
        virtual void SaveToStream(OStream& stream) override
        {
            m_components.SaveToStream(stream);

            HashMap<uint32, uint32> compEntityIDs;

            int i = 0;
            for (auto* comp : m_components)
            {
                if (comp != nullptr)
                    compEntityIDs[i] = comp->m_entityID;

                i++;
            }

            HashMapSerialization::SaveToStream_PT(stream, compEntityIDs);

            for (auto [compID, entityID] : compEntityIDs)
            {
                auto comp = m_components.GetItem(compID);
                stream << comp->m_entityID;
                comp->SaveToStream(stream);
            }
        }

        virtual void LoadFromStream(IStream& stream) override
        {
            m_components.LoadFromStream(stream);

            HashMap<uint32, uint32> compEntityIDs;
            HashMapSerialization::LoadFromStream_PT(stream, compEntityIDs);

            for (auto [compID, entityID] : compEntityIDs)
            {
                T* comp = new (m_allocatorPool->Allocate(sizeof(T))) T();
                stream >> comp->m_entityID;
                comp->LoadFromStream(stream);
                comp->m_entity = m_entities[comp->m_entityID];
                comp->OnComponentCreated();
                m_components.AddItem(comp, compID);
            }
        }

    private:
        friend class EntityWorld;
        IDList<T*>           m_components;
        MemoryAllocatorPool* m_allocatorPool = nullptr;
    };

} // namespace Lina

#endif
