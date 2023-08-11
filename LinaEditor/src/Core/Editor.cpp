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

#include "Core/Editor.hpp"
#include "Resources/Core/CommonResources.hpp"
#include "Resources/Core/ResourceManager.hpp"
#include "Resources/Core/IResource.hpp"
#include "Graphics/Core/GfxManager.hpp"
#include "Graphics/Core/SurfaceRenderer.hpp"
#include "World/Level/LevelManager.hpp"
#include "World/Level/Level.hpp"
#include "Serialization/Serialization.hpp"
#include "Serialization/Compressor.hpp"
#include "FileSystem/FileSystem.hpp"
#include "Reflection/ReflectionSystem.hpp"
#include "System/ISystem.hpp"
#include "GUI/Drawers/GUIDrawerSplashScreen.hpp"
#include "GUI/Drawers/GUIDrawerMainWindow.hpp"
#include "GUI/Drawers/GUIDrawerBase.hpp"
#include "GUI/Drawers/GUIDrawerChildWindow.hpp"
#include "GUI/Nodes/Panels/GUINodePanel.hpp"
#include "GUI/Nodes/Panels/GUIPanelFactory.hpp"
#include "GUI/Nodes/Docking/GUINodeDockPreview.hpp"
#include "GUI/Nodes/Docking/GUINodeDockArea.hpp"
#include "Core/Theme.hpp"
#include "Core/PlatformProcess.hpp"
#include "GUI/Utility/GUIUtility.hpp"
#include "Graphics/Core/LGXWrapper.hpp"

namespace Lina::Editor
{
	uint32 Editor::s_childWindowCtr = 0;

	void Editor::Initialize(const SystemInitializationInfo& initInfo)
	{
		m_resourceManager			 = m_system->CastSubsystem<ResourceManager>(SubsystemType::ResourceManager);
		m_gfxManager				 = m_system->CastSubsystem<GfxManager>(SubsystemType::GfxManager);
		m_lgxWrapper				 = m_system->CastSubsystem<LGXWrapper>(SubsystemType::LGXWrapper);
		m_levelManager				 = m_system->CastSubsystem<LevelManager>(SubsystemType::LevelManager);
		Theme::s_resourceManagerInst = m_resourceManager;

		m_payloadManager.Initialize();
		m_system->AddListener(this);
	}

	void Editor::Shutdown()
	{
		m_system->RemoveListener(this);

		for (auto [sid, drawer] : m_guiDrawers)
			delete drawer;

		m_payloadManager.Shutdown();
	}

	void Editor::PackageResources(const Vector<ResourceIdentifier>& identifiers)
	{
		ResourceManager* rm = static_cast<ResourceManager*>(m_system->CastSubsystem(SubsystemType::ResourceManager));

		HashMap<PackageType, Vector<ResourceIdentifier>> resourcesPerPackage;

		for (const auto& ident : identifiers)
		{
			PackageType pt = rm->GetPackageType(ident.tid);
			resourcesPerPackage[pt].push_back(ident);
		}

		for (auto [packageType, resources] : resourcesPerPackage)
		{
			const String packagePath = GGetPackagePath(packageType);
			OStream		 stream;
			stream.CreateReserve(1000);

			for (auto r : resources)
			{
				// Header
				stream << r.tid;
				stream << r.sid;

				const String metacachePath = ResourceManager::GetMetacachePath(r.path, r.sid);
				if (FileSystem::FileExists(metacachePath))
				{
					IStream		 cache = Serialization::LoadFromFile(metacachePath.c_str());
					const uint32 size  = static_cast<uint32>(cache.GetSize());
					stream << size;
					stream.WriteEndianSafe(cache.GetDataRaw(), cache.GetSize());
					cache.Destroy();
				}
				else
				{
					OStream outStream;
					outStream.CreateReserve(512);

					// Load into stream & destroy.
					IResource* res = static_cast<IResource*>(ReflectionSystem::Get().Resolve(r.tid).GetFunction<void*()>("CreateMock"_hs)());
					// res->LoadFromFile(r.path.c_str());
					// res->SaveToStream(outStream);
					// res->Flush();
					ReflectionSystem::Get().Resolve(r.tid).GetFunction<void(void*)>("DestroyMock"_hs)(static_cast<void*>(res));

					// Write stream to package & destroy.
					const uint32 size = static_cast<uint32>(outStream.GetCurrentSize());
					stream << size;
					stream.WriteRaw(outStream.GetDataRaw(), outStream.GetCurrentSize());
					outStream.Destroy();
				}
			}

			if (!FileSystem::FileExists("Resources/Packages"))
				FileSystem::CreateFolderInPath("Resources/Packages");

			Serialization::SaveToFile(packagePath.c_str(), stream);
		}
	}

	void Editor::BeginSplashScreen()
	{
		auto sf				  = m_gfxManager->GetSurfaceRenderer(LINA_MAIN_SWAPCHAIN);
		m_guiDrawerMainWindow = new GUIDrawerSplashScreen(this, sf->GetSwapchain());
		sf->SetGUIDrawer(m_guiDrawerMainWindow);
	}

	void Editor::EndSplashScreen()
	{
		auto sf = m_gfxManager->GetSurfaceRenderer(LINA_MAIN_SWAPCHAIN);
		delete m_guiDrawerMainWindow;
		m_guiDrawerMainWindow = new GUIDrawerMainWindow(this, sf->GetSwapchain());
		sf->SetGUIDrawer(m_guiDrawerMainWindow);
		m_guiDrawers[LINA_MAIN_SWAPCHAIN] = m_guiDrawerMainWindow;

		m_editorImages = m_system->CastSubsystem<ResourceManager>(SubsystemType::ResourceManager)->GetResource<Texture>("Resources/Editor/Textures/EditorImages.png"_hs)->GetSheetItems(EDITOR_IMAGES_SHEET_COLUMNS, EDITOR_IMAGES_SHEET_ROWS);

		m_layoutManager.LoadSavedLayout();
	}

	void Editor::Tick()
	{
		m_payloadManager.Tick();

		// Window create & destroy requests.
		{
			if (!m_createWindowRequests.empty() || !m_deleteWindowRequests.empty())
				m_gfxManager->Join();

			for (auto& req : m_deleteWindowRequests)
			{
				m_windowManager->DestroyAppWindow(req.sid);
				auto it = m_guiDrawers.find(req.sid);

				delete it->second;
				m_guiDrawers.erase(it);
			}

			for (auto& req : m_createWindowRequests)
			{
				auto window	   = CreateChildWindow(req.windowSid, req.title, Vector2i::Zero, Vector2i(500, 500));
				auto guiDrawer = m_guiDrawers.at(req.windowSid);

				auto targetDockArea = guiDrawer->GetFirstDockArea();

				if (req.panel)
					targetDockArea->AddPanel(req.panel);
				else
				{
					auto createdPanel = GUIPanelFactory::CreatePanel(req.panelType, targetDockArea, req.title, req.panelSid);
					targetDockArea->AddPanel(static_cast<GUINodePanel*>(createdPanel));
				}

				if (req.byDetach)
				{
					window->SetPos(m_input->GetMousePositionAbs() - Vector2i(15, 10));
					window->SetForceIsDragged(true);
				}
				else
					window->SetToCenter();
			}

			m_createWindowRequests.clear();
			m_deleteWindowRequests.clear();
		}

		// Controlling alpha of the current dragged window according to dock-hovers
		{
			if (m_draggedWindow)
			{
				bool dockPreviewActive = false;
				for (auto [sid, drawer] : m_guiDrawers)
				{
					if (drawer->GetDockPreview()->GetIsActive())
					{
						dockPreviewActive = true;
						break;
					}

					const auto& dockAreas = drawer->GetDockAreas();
					for (auto d : dockAreas)
					{
						if (d->GetDockPreview()->GetIsActive())
						{
							dockPreviewActive = true;
							break;
						}
					}
				}

				if (dockPreviewActive && !m_draggedWindow->GetIsTransparent())
					m_draggedWindow->SetAlpha(0.3f);
				else if (!dockPreviewActive && m_draggedWindow->GetIsTransparent())
					m_draggedWindow->SetAlpha(1.0f);
			}
		}
	}

	void Editor::OnShortcut(Shortcut sc, void* windowHandle)
	{
		if (m_payloadManager.GetCurrentPayloadMeta().type != PayloadType::EPL_None)
			return;

		bool consumed = false;

		for (auto [sid, drawer] : m_guiDrawers)
		{
			if (drawer->GetWindow() == windowHandle)
			{
				if (drawer->GetRoot()->OnShortcut(sc))
				{
					consumed = true;
					break;
				}
			}
		}

		if (!consumed)
		{
			if (sc == Shortcut::CTRL_S && m_levelManager->GetCurrentLevel())
			{
				SaveCurrentLevel();
			}
			else if (sc == Shortcut::CTRL_SHIFT_S && m_levelManager->GetCurrentLevel())
			{
				String savePath = PlatformProcess::SaveDialog(L"Lina Engine Level", L"*.linalevel");

				if (!savePath.empty())
					SaveCurrentLevelAs(savePath.c_str());
			}
			else if (sc == Shortcut::CTRL_Z)
			{
				m_commandManager.Undo();
			}
		}
	}

	void Editor::OpenPanel(EditorPanel panel, const String& title, StringID sid, bool byDetach, GUINodePanel* srcPanel)
	{
		auto wm		= m_system->CastSubsystem<WindowManager>(SubsystemType::WindowManager);
		auto gfxMan = m_system->CastSubsystem<GfxManager>(SubsystemType::GfxManager);

		const String   panelName = title;
		const StringID panelSID	 = sid;

		for (auto& [sid, guiDrawer] : m_guiDrawers)
		{
			const auto& dockAreas = guiDrawer->GetDockAreas();

			for (auto area : dockAreas)
			{
				GUINode* node = area->FindChildren(panelSID);
				if (node)
				{
					wm->GetWindow(sid)->BringToFront();
					area->FocusPanel(panelSID);
					return;
				}
			}
		}

		const String windowSidStr = "EditorChildWindow_" + s_childWindowCtr++;

		CreateWindowRequest req = CreateWindowRequest{
			.panelType = panel,
			.panelSid  = sid,
			.windowSid = TO_SID(windowSidStr),
			.title	   = title,
			.panel	   = srcPanel,
			.byDetach  = byDetach,
		};

		m_createWindowRequests.push_back(req);
	}

	IWindow* Editor::CreateChildWindow(StringID sid, const String& title, const Vector2i& pos, const Vector2i& size)
	{
		auto window = m_windowManager->CreateAppWindow(sid, title.c_str(), pos, size, SRM_DrawGUI);
		window->SetStyle(WindowStyle::Borderless);
		window->SetVisible(true);

		auto surfaceRenderer = m_gfxManager->GetSurfaceRenderer(sid);
		auto guiDrawer		 = new GUIDrawerChildWindow(this, surfaceRenderer->GetSwapchain());
		surfaceRenderer->SetGUIDrawer(guiDrawer);
		m_guiDrawers[sid] = guiDrawer;

		return window;
	}

	void Editor::CloseWindow(StringID sid)
	{
		DeleteWindowRequest req = {sid};
		m_deleteWindowRequests.push_back(req);
	}

	void Editor::OnWindowDrag(GUIDrawerBase* owner, bool isDragging)
	{
		if (!isDragging && m_draggedWindow && m_draggedWindow->GetIsTransparent())
			m_draggedWindow->SetAlpha(1.0f);

		// Only single dock area windows can be docked to others.
		if (!owner->GetDockAreas()[0]->GetIsAlone())
		{
			m_draggedWindow = isDragging ? owner->GetWindow() : nullptr;
			return;
		}

		GUIDrawerBase*	 drawerToDock	  = nullptr;
		GUINodeDockArea* splittedDockArea = nullptr;
		DockSplitType	 split			  = DockSplitType::None;

		for (auto [sid, drawer] : m_guiDrawers)
		{
			if (owner == drawer)
				continue;

			if (drawerToDock == nullptr)
			{
				split = drawer->GetDockPreview()->GetCurrentSplitType();
				if (split != DockSplitType::None)
				{
					drawerToDock = drawer;
				}
				else
				{
					const auto& dockAreas = drawer->GetDockAreas();
					for (auto d : dockAreas)
					{
						split = d->GetDockPreview()->GetCurrentSplitType();
						if (split != DockSplitType::None)
						{
							drawerToDock	 = drawer;
							splittedDockArea = d;
							break;
						}
					}
				}
			}

			drawer->SetDockPreviewEnabled(isDragging);
		}

		if (drawerToDock)
		{
			const auto panels = owner->GetFirstDockArea()->GetPanels();

			for (auto p : panels)
				owner->GetFirstDockArea()->RemovePanel(p, false);

			drawerToDock->SplitDockArea(splittedDockArea, split, panels);
			CloseWindow(m_draggedWindow->GetSID());
			m_draggedWindow = nullptr;
		}
		else
			m_draggedWindow = isDragging ? owner->GetWindow() : nullptr;
	}

	void Editor::CloseAllChildWindows()
	{
		const auto& windows = m_windowManager->GetWindows();

		Vector<StringID> sids;

		for (auto [sid, w] : windows)
		{
			if (sid != LINA_MAIN_SWAPCHAIN && sid != EDITOR_PAYLOAD_WINDOW_SID)
				sids.push_back(sid);
		}

		for (auto sid : sids)
			CloseWindow(sid);
	}

	void Editor::CreateNewLevel(const char* path)
	{
		const StringID pathSID = TO_SIDC(path);
		Level*		   level   = new Level(m_resourceManager, true, path, pathSID);
		level->SaveToFile(path);
		delete level;

		if (m_levelManager->GetCurrentLevel())
		{
			m_levelManager->UninstallLevel(false);
			m_levelManager->QueueLevel(path);
			return;
		}

		m_levelManager->InstallLevel(path);
	}

	void Editor::LoadLevel(const char* path)
	{
		if (m_levelManager->GetCurrentLevel())
		{
			m_levelManager->UninstallLevel(false);
			m_levelManager->QueueLevel(path);
			return;
		}

		m_levelManager->InstallLevel(path);
	}

	void Editor::SaveCurrentLevel()
	{
		DispatchEvent(EVE_PreSavingCurrentLevel, {});
		auto level = m_levelManager->GetCurrentLevel();
		level->SaveToFile(level->GetPath().c_str());
	}

	void Editor::SaveCurrentLevelAs(const char* path)
	{
		DispatchEvent(EVE_PreSavingCurrentLevel, {});
		auto level = m_levelManager->GetCurrentLevel();
		level->SaveToFile(path);
	}

	void Editor::UninstallCurrentLevel()
	{
		m_levelManager->UninstallLevel(false);
	}
	void Editor::OnSystemEvent(SystemEvent eventType, const Event& ev)
	{
		if (eventType & EVS_StartFrame)
		{
			GUIUtility::PrepareClipStack(ev.iParams[0]);
		}
	}
} // namespace Lina::Editor
