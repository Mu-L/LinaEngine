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

#include "Editor/Actions/EditorActionEntity.hpp"
#include "Editor/Editor.hpp"
#include "Editor/Widgets/World/WorldController.hpp"
#include "Editor/World/EditorWorldUtility.hpp"
#include "Core/World/WorldUtility.hpp"
#include "Core/World/Entity.hpp"
#include "Core/World/Component.hpp"
#include "Core/Physics/PhysicsWorld.hpp"

namespace Lina::Editor
{

	EditorActionEntitySelection* EditorActionEntitySelection::Create(Editor* editor, uint64 worldId, const Vector<Entity*>& entities, bool select, bool clearOthers, StringID src)
	{
		EditorActionEntitySelection* action = new EditorActionEntitySelection();

		action->m_select = select;
		action->m_entities.reserve(entities.size());
		action->m_worldId = worldId;
		action->m_clear	  = clearOthers;
		action->m_src	  = src;

		for (Entity* e : entities)
		{
			if (e != nullptr)
				action->m_entities.push_back(e->GetGUID());
		}

		EditorWorldManager::WorldData& wd = editor->GetWorldManager().GetWorldData(editor->GetWorldManager().GetWorld(worldId));
		for (Entity* e : wd.selectedEntities)
			action->m_selection.push_back(e->GetGUID());

		editor->GetEditorActionManager().AddToStack(action);
		return action;
	}

	void EditorActionEntitySelection::Execute(Editor* editor, ExecType type)
	{
		EntityWorld* world = editor->GetWorldManager().GetWorld(m_worldId);
		if (!world)
			return;

		EditorWorldManager::WorldData& wd = editor->GetWorldManager().GetWorldData(world);

		if (type == ExecType::Undo)
		{
			wd.selectedEntities.resize(0);
			for (EntityID id : m_selection)
			{
				Entity* e = world->GetEntity(id);
				wd.selectedEntities.push_back(e);
			}
		}
		else if (type == ExecType::Redo || type == ExecType::Create)
		{
			if (m_clear)
				wd.selectedEntities.resize(0);

			for (EntityID id : m_entities)
			{
				Entity* e = world->GetEntity(id);

				if (e == nullptr)
					continue;

				if (m_select)
				{
					auto it = linatl::find_if(wd.selectedEntities.begin(), wd.selectedEntities.end(), [e](Entity* ent) -> bool { return ent == e; });
					if (it == wd.selectedEntities.end())
						wd.selectedEntities.push_back(e);
				}
				else
				{
					auto it = linatl::find_if(wd.selectedEntities.begin(), wd.selectedEntities.end(), [e](Entity* ent) -> bool { return ent == e; });
					if (it != wd.selectedEntities.end())
						wd.selectedEntities.erase(it);
				}
			}
		}

		editor->GetWorldManager().BroadcastEntitySelectionChanged(world, wd.selectedEntities, m_src);
	}

	EditorActionEntityTransform* EditorActionEntityTransform::Create(Editor* editor, uint64 worldId, const Vector<Entity*>& entities, const Vector<Transformation>& previousTransforms)
	{
		EditorActionEntityTransform* action = new EditorActionEntityTransform();

		for (Entity* e : entities)
		{
			action->m_entities.push_back(e->GetGUID());
			action->m_currentTransforms.push_back(e->GetTransform());
		}

		action->m_prevTransforms = previousTransforms;
		action->m_worldId		 = worldId;

		editor->GetEditorActionManager().AddToStack(action);
		return action;
	}

	void EditorActionEntityTransform::Execute(Editor* editor, ExecType type)
	{
		EntityWorld* world = editor->GetWorldManager().GetWorld(m_worldId);
		if (world == nullptr)
			return;

		if (type == ExecType::Undo)
		{
			size_t i = 0;
			for (EntityID id : m_entities)
			{
				Entity* e = world->GetEntity(id);
				e->SetTransform(m_prevTransforms.at(i));
				i++;
			}
		}
		else if (type == ExecType::Redo)
		{
			size_t i = 0;
			for (EntityID id : m_entities)
			{
				Entity* e = world->GetEntity(id);
				e->SetTransform(m_currentTransforms.at(i));
				i++;
			}
		}
	}

	EditorActionEntityNames* EditorActionEntityNames::Create(Editor* editor, uint64 worldId, const Vector<Entity*>& entities, const Vector<String>& previousNames)
	{
		EditorActionEntityNames* action = new EditorActionEntityNames();

		for (Entity* e : entities)
		{
			action->m_entities.push_back(e->GetGUID());
			action->m_currentNames.push_back(e->GetName());
		}

		action->m_prevNames = previousNames;
		action->m_worldId	= worldId;

		editor->GetEditorActionManager().AddToStack(action);
		return action;
	}

	void EditorActionEntityNames::Execute(Editor* editor, ExecType type)
	{
		EntityWorld* world = editor->GetWorldManager().GetWorld(m_worldId);
		if (world == nullptr)
			return;

		if (type == ExecType::Undo)
		{
			size_t i = 0;
			for (EntityID id : m_entities)
			{
				Entity* e = world->GetEntity(id);
				e->SetName(m_prevNames.at(i));
				i++;
			}
		}
		else if (type == ExecType::Redo)
		{
			size_t i = 0;
			for (EntityID id : m_entities)
			{
				Entity* e = world->GetEntity(id);
				e->SetName(m_currentNames.at(i));
				i++;
			}
		}

		editor->GetWorldManager().BroadcastEntityHierarchyChanged(world);
	}

	EditorActionEntitiesCreated* EditorActionEntitiesCreated::Create(Editor* editor, EntityWorld* world, const Vector<Entity*>& entities)
	{
		EditorActionEntitiesCreated* action = new EditorActionEntitiesCreated();
		action->m_worldId					= world->GetID();

		Vector<Entity*> roots;
		WorldUtility::ExtractRoots(entities, roots);
		WorldUtility::SaveEntitiesToStream(action->m_stream, world, roots);

		for (Entity* e : roots)
			action->m_guids.push_back(e->GetGUID());

		editor->GetEditorActionManager().AddToStack(action);
		return action;
	}

	void EditorActionEntitiesCreated::Execute(Editor* editor, ExecType type)
	{
		EntityWorld* world = editor->GetWorldManager().GetWorld(m_worldId);
		if (!world)
			return;

		if (type == ExecType::Undo)
		{
			Vector<Entity*> entities;
			for (EntityID id : m_guids)
			{
				Entity* e = world->GetEntity(id);
				world->DestroyEntity(e);
			}
		}
		else if (type == ExecType::Redo)
		{
			Vector<Entity*> entities;
			IStream			stream;
			stream.Create(m_stream.GetDataRaw(), m_stream.GetCurrentSize());
			WorldUtility::LoadEntitiesFromStream(stream, world, entities);
			world->RefreshComponentReferences(entities);
			stream.Destroy();
		}

		editor->GetWorldManager().BroadcastEntityHierarchyChanged(world);
	}

	EditorActionEntityDelete* EditorActionEntityDelete::Create(Editor* editor, EntityWorld* world, const Vector<Entity*>& entities)
	{
		EditorActionEntityDelete* action = new EditorActionEntityDelete();
		action->m_worldId				 = world->GetID();

		Vector<Entity*> roots;
		WorldUtility::ExtractRoots(entities, roots);
		WorldUtility::SaveEntitiesToStream(action->m_stream, world, roots);

		for (Entity* e : roots)
			action->m_guids.push_back(e->GetGUID());
		editor->GetEditorActionManager().AddToStack(action);
		return action;
	}

	void EditorActionEntityDelete::Execute(Editor* editor, ExecType type)
	{
		EntityWorld* world = editor->GetWorldManager().GetWorld(m_worldId);
		if (!world)
			return;

		if (type == ExecType::Redo || type == ExecType::Create)
		{
			Vector<Entity*> entities;

			for (EntityID id : m_guids)
			{
				Entity* e = world->GetEntity(id);
				world->DestroyEntity(e);
			}
		}
		else if (type == ExecType::Undo)
		{
			Vector<Entity*> entities;
			IStream			stream;
			stream.Create(m_stream.GetDataRaw(), m_stream.GetCurrentSize());
			WorldUtility::LoadEntitiesFromStream(stream, world, entities);
			world->RefreshComponentReferences(entities);
			stream.Destroy();
		}

		editor->GetWorldManager().BroadcastEntityHierarchyChanged(world);
	}

	EditorActionEntityParenting* EditorActionEntityParenting::Create(Editor* editor, EntityWorld* world, const Vector<Entity*>& entities, Entity* newParent)
	{
		auto it = linatl::find_if(entities.begin(), entities.end(), [newParent](Entity* e) -> bool { return e == newParent; });
		if (it != entities.end())
			return nullptr;

		if (newParent != nullptr)
		{
			for (Entity* e : entities)
			{
				Entity* isAlreadyChild = e->FindInChildHierarchy(newParent->GetGUID());
				if (isAlreadyChild)
					return nullptr;
			}
		}

		EditorActionEntityParenting* action = new EditorActionEntityParenting();
		action->m_worldId					= world->GetID();
		for (Entity* e : entities)
		{
			action->m_guids.push_back(e->GetGUID());
			action->m_oldParentGUIDs.push_back(e->GetParent() ? e->GetParent()->GetGUID() : 0);
		}

		action->m_newParentGUID = newParent ? newParent->GetGUID() : 0;

		editor->GetEditorActionManager().AddToStack(action);
		return action;
	}

	void EditorActionEntityParenting::Execute(Editor* editor, ExecType type)
	{
		EntityWorld* world = editor->GetWorldManager().GetWorld(m_worldId);
		if (!world)
			return;

		Vector<Entity*> entities;
		entities.reserve(m_guids.size());

		for (EntityID id : m_guids)
			entities.push_back(world->GetEntity(id));

		if (type == ExecType::Redo || type == ExecType::Create)
		{
			Entity* parent = world->GetEntity(m_newParentGUID);

			for (Entity* e : entities)
			{
				if (parent)
					parent->AddChild(e);
				else
					e->RemoveFromParent();
			}
		}
		else if (type == ExecType::Undo)
		{
			size_t i = 0;
			for (Entity* e : entities)
			{
				Entity* oldParent = world->GetEntity(m_oldParentGUIDs.at(i));

				if (oldParent)
					oldParent->AddChild(e);
				else
					e->RemoveFromParent();

				i++;
			}
		}

		editor->GetWorldManager().BroadcastEntityHierarchyChanged(world);
	}

	EditorActionEntityPhysicsSettingsChanged* EditorActionEntityPhysicsSettingsChanged::Create(Editor* editor, EntityWorld* world, const Vector<Entity*>& entities, const Vector<EntityPhysicsSettings>& previousSettings)
	{
		EditorActionEntityPhysicsSettingsChanged* action = new EditorActionEntityPhysicsSettingsChanged();

		for (Entity* e : entities)
		{
			action->m_guids.push_back(e->GetGUID());
			action->m_newSettings.push_back(e->GetPhysicsSettings());
		}

		action->m_worldId	  = world->GetID();
		action->m_oldSettings = previousSettings;
		editor->GetEditorActionManager().AddToStack(action);
		return action;
	}

	void EditorActionEntityPhysicsSettingsChanged::Execute(Editor* editor, ExecType type)
	{
		EntityWorld* world = editor->GetWorldManager().GetWorld(m_worldId);
		if (!world)
			return;

		Vector<Entity*> entities;
		entities.reserve(m_guids.size());

		for (EntityID id : m_guids)
			entities.push_back(world->GetEntity(id));

		if (type == ExecType::Redo)
		{
			size_t i = 0;
			for (Entity* e : entities)
			{
				e->GetPhysicsSettings() = m_newSettings.at(i);
				i++;
			}
		}
		else if (type == ExecType::Undo)
		{
			size_t i = 0;
			for (Entity* e : entities)
			{
				e->GetPhysicsSettings() = m_oldSettings.at(i);
				i++;
			}
		}

		if (type != ExecType::Create)
			editor->GetWorldManager().BroadcastEntityPhysicsSettingsChanged(world);
	}

	EditorActionEntityParamsChanged* EditorActionEntityParamsChanged::Create(Editor* editor, EntityWorld* world, const Vector<Entity*>& entities, const Vector<EntityParameters>& previousParams)
	{
		EditorActionEntityParamsChanged* action = new EditorActionEntityParamsChanged();

		for (Entity* e : entities)
		{
			action->m_guids.push_back(e->GetGUID());
			action->m_newParams.push_back(e->GetParams());
		}

		action->m_worldId	= world->GetID();
		action->m_oldParams = previousParams;
		editor->GetEditorActionManager().AddToStack(action);
		return action;
	}

	void EditorActionEntityParamsChanged::Execute(Editor* editor, ExecType type)
	{
		EntityWorld* world = editor->GetWorldManager().GetWorld(m_worldId);
		if (!world)
			return;

		Vector<Entity*> entities;
		entities.reserve(m_guids.size());

		for (EntityID id : m_guids)
			entities.push_back(world->GetEntity(id));

		if (type == ExecType::Redo)
		{
			size_t i = 0;
			for (Entity* e : entities)
			{
				e->GetParams() = m_newParams.at(i);
				i++;
			}
		}
		else if (type == ExecType::Undo)
		{
			size_t i = 0;
			for (Entity* e : entities)
			{
				e->GetParams() = m_oldParams.at(i);
				i++;
			}
		}

		if (type != ExecType::Create)
			editor->GetWorldManager().BroadcastEntityParamsChanged(world);

	}

} // namespace Lina::Editor
