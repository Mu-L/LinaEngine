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

#include "Core/ApplicationDelegate.hpp"
#include "Core/World/WorldManager.hpp"
#include "Common/Data/HashMap.hpp"
#include "Meta/EditorSettings.hpp"
#include "Editor/CommonEditor.hpp"
#include "IO/FileManager.hpp"
#include "Atlas/AtlasManager.hpp"
#include "Core/Resources/ResourceManagerListener.hpp"
#include "Editor/WindowPanelManager.hpp"
#include "Editor/Project/ProjectManager.hpp"
#include "Editor/Graphics/EditorRenderer.hpp"

namespace Lina
{
	class Application;
	class GfxManager;
	class WidgetManager;
	class EntityWorld;
	class Widget;
	class GUIWidget;
	class ResourceManager;
	class SurfaceRenderer;
	class WorldRenderer;
} // namespace Lina

namespace LinaGX
{
	class Window;
}

namespace Lina::Editor
{
	class SplashScreen;
	class EditorRoot;
	class DockArea;
	class WorldRendererExtEditor;

	class Editor : public ApplicationDelegate, public WorldManagerListener, public ResourceManagerListener
	{

	private:
	public:
		Editor(){};
		virtual ~Editor() = default;
		static Editor* Get()
		{
			return s_editor;
		}

		// Application delegate
		virtual void PreInitialize() override;
		virtual void Initialize() override;
		virtual void PreTick() override;
		virtual void PreShutdown() override;
		virtual void Render(uint32 frameIndex) override;
		virtual bool FillResourceCustomMeta(StringID sid, OStream& stream) override;

		// Misc
		void SaveSettings();
		void RequestExit();

		// Resources
		virtual void OnResourceLoadEnded(int32 taskID, const Vector<ResourceIdentifier>& idents) override;

		// Renderers
		WorldRenderer* GetWorldRenderer(EntityWorld* world);

		// World
		virtual void OnWorldInstalled(EntityWorld* world) override;

		inline EntityWorld* GetCurrentWorld() const
		{
			return m_currentWorld;
		}

		inline EditorRoot* GetEditorRoot() const
		{
			return m_editorRoot;
		}

		inline EditorSettings& GetSettings()
		{
			return m_settings;
		}

		inline FileManager& GetFileManager()
		{
			return m_fileManager;
		}

		inline AtlasManager& GetAtlasManager()
		{
			return m_atlasManager;
		}

		inline WindowPanelManager& GetWindowPanelManager()
		{
			return m_windowPanelManager;
		}

		inline ProjectManager& GetProjectManager()
		{
			return m_projectManager;
		}

	private:
		void CreateWorldRenderer(EntityWorld* world);
		void DestroyWorldRenderer(EntityWorld* world);
		void CoreResourcesLoaded();

	private:
		EditorRenderer						  m_editorRenderer;
		WindowPanelManager					  m_windowPanelManager;
		AtlasManager						  m_atlasManager;
		ProjectManager						  m_projectManager;
		EditorSettings						  m_settings = {};
		FileManager							  m_fileManager;
		WorldManager*						  m_worldManager		 = nullptr;
		GfxManager*							  m_gfxManager			 = nullptr;
		WidgetManager*						  m_primaryWidgetManager = nullptr;
		ResourceManager*					  m_rm					 = nullptr;
		EntityWorld*						  m_currentWorld		 = nullptr;
		EditorRoot*							  m_editorRoot			 = nullptr;
		LinaGX::Window*						  m_mainWindow			 = nullptr;
		HashMap<EntityWorld*, WorldRenderer*> m_worldRenderers;
		static Editor*						  s_editor;
		bool								  m_coreResourcesOK = false;
	};

} // namespace Lina::Editor
