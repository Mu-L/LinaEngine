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

#include "Editor/Widgets/EditorRoot.hpp"
#include "Editor/Widgets/Docking/DockArea.hpp"
#include "Editor/CommonEditor.hpp"
#include "Editor/EditorLocale.hpp"
#include "Editor/Widgets/CommonWidgets.hpp"
#include "Editor/Widgets/Panel/Panel.hpp"
#include "Editor/Editor.hpp"
#include "Core/Meta/ProjectData.hpp"
#include "Core/GUI/Widgets/Primitives/Icon.hpp"
#include "Core/GUI/Widgets/Primitives/Text.hpp"
#include "Core/GUI/Widgets/Primitives/ShapeRect.hpp"
#include "Core/GUI/Widgets/Layout/DirectionalLayout.hpp"
#include "Core/GUI/Widgets/WidgetUtility.hpp"
#include "Core/Resources/ResourceManager.hpp"
#include "Core/Graphics/Resource/Texture.hpp"
#include "Core/Platform/PlatformProcess.hpp"
#include "Core/World/WorldManager.hpp"
#include "Core/World/EntityWorld.hpp"
#include "Common/System/System.hpp"
#include "Common/Math/Math.hpp"
#include "Common/FileSystem/FileSystem.hpp"
#include "Core/GUI/Widgets/WidgetManager.hpp"
#include <LinaGX/Core/InputMappings.hpp>

namespace Lina::Editor
{
	void EditorRoot::Construct()
	{
		const String tooltip = "Lina Engine v." + TO_STRING(LINA_MAJOR) + "." + TO_STRING(LINA_MINOR) + "." + TO_STRING(LINA_PATCH) + " - b: " + TO_STRING(LINA_BUILD);
		SetTooltip(tooltip);

		GetProps().direction = DirectionOrientation::Vertical;

		GetFlags().Set(WF_POS_ALIGN_X | WF_POS_ALIGN_Y | WF_SIZE_ALIGN_X | WF_SIZE_ALIGN_Y);
		SetAlignedPos(Vector2::Zero);
		SetAlignedSize(Vector2::One);

		DirectionalLayout* titleBar = m_manager->Allocate<DirectionalLayout>("TitleBar");
		titleBar->GetFlags().Set(WF_SIZE_ALIGN_X | WF_USE_FIXED_SIZE_Y);
		titleBar->GetProps().direction = DirectionOrientation::Horizontal;
		titleBar->SetAlignedSizeX(1.0f);
		titleBar->SetFixedSizeY(Theme::GetDef().baseItemHeight);
		titleBar->SetChildPadding(Theme::GetDef().baseIndent);
		titleBar->GetProps().backgroundStyle	  = BackgroundStyle::CentralGradient;
		titleBar->GetProps().colorBackgroundStart = Theme::GetDef().background0;
		titleBar->GetProps().colorBackgroundEnd	  = Theme::GetDef().accentPrimary0;
		titleBar->GetChildMargins().left		  = Theme::GetDef().baseIndent;
		AddChild(titleBar);

		Icon* lina			  = m_manager->Allocate<Icon>("Lina");
		lina->GetProps().icon = ICON_LINA_LOGO;
		lina->GetFlags().Set(WF_POS_ALIGN_Y);
		lina->SetAlignedPosY(0.5f);
		lina->SetPosAlignmentSourceY(PosAlignmentSource::Center);
		lina->GetProps().colorStart = Theme::GetDef().accentPrimary0;
		lina->GetProps().colorEnd	= Theme::GetDef().accentPrimary1;
		lina->GetProps().isDynamic	= false;
		titleBar->AddChild(lina);

		FileMenu* fm = m_manager->Allocate<FileMenu>("FileMenu");
		fm->GetFlags().Set(WF_SIZE_ALIGN_Y | WF_POS_ALIGN_Y);
		fm->SetAlignedPosY(0.0f);
		fm->SetAlignedSizeY(1.0f);

		fm->GetFileMenuProps().buttons = {
			Locale::GetStr(LocaleStr::File),
			Locale::GetStr(LocaleStr::Edit),
			Locale::GetStr(LocaleStr::View),
			Locale::GetStr(LocaleStr::Panels),
			Locale::GetStr(LocaleStr::About),
		};

		fm->SetListener(this);
		titleBar->AddChild(fm);

		Widget* filler = m_manager->Allocate<Widget>("Filler");
		filler->GetFlags().Set(WF_POS_ALIGN_Y | WF_SIZE_ALIGN_X | WF_SIZE_ALIGN_Y);
		filler->SetAlignedSize(Vector2(0.0f, 1.0f));
		filler->SetAlignedPosY(0.0f);
		titleBar->AddChild(filler);

		DirectionalLayout* projectName = m_manager->Allocate<DirectionalLayout>("Project Name");
		projectName->GetFlags().Set(WF_SIZE_X_TOTAL_CHILDREN | WF_USE_FIXED_SIZE_Y | WF_POS_ALIGN_Y);
		projectName->SetAlignedSizeX(1.0f);
		projectName->SetAlignedPosY(0.5f);
		projectName->SetPosAlignmentSourceY(PosAlignmentSource::Center);
		projectName->SetFixedSizeY(Theme::GetDef().baseItemHeight);
		projectName->GetChildMargins() = {.left = Theme::GetDef().baseIndent, .right = Theme::GetDef().baseIndent};
		projectName->SetChildPadding(Theme::GetDef().baseIndent);
		projectName->GetProps().backgroundStyle		 = BackgroundStyle::Default;
		projectName->GetProps().colorBackgroundStart = Theme::GetDef().background0;
		projectName->GetProps().colorBackgroundEnd	 = Theme::GetDef().background0;
		projectName->GetProps().outlineThickness	 = 0.0f;
		projectName->GetProps().rounding			 = 0.0f;
		titleBar->AddChild(projectName);

		Icon* saveIcon			  = m_manager->Allocate<Icon>("SaveIcon");
		saveIcon->GetProps().icon = ICON_SAVE;
		saveIcon->GetFlags().Set(WF_POS_ALIGN_Y);
		saveIcon->SetAlignedPosY(0.5f);
		saveIcon->SetPosAlignmentSourceY(PosAlignmentSource::Center);
		saveIcon->GetProps().colorStart = saveIcon->GetProps().colorEnd = Theme::GetDef().accentWarn;
		saveIcon->GetProps().colorDisabled								= Theme::GetDef().silent0;
		projectName->AddChild(saveIcon);

		Text* projectNameText = m_manager->Allocate<Text>("ProjectNameText");
		projectNameText->GetFlags().Set(WF_POS_ALIGN_Y);
		projectNameText->SetAlignedPosY(0.5f);
		projectNameText->SetPosAlignmentSourceX(PosAlignmentSource::Center);
		projectNameText->SetPosAlignmentSourceY(PosAlignmentSource::Center);
		projectNameText->GetProps().text = Locale::GetStr(LocaleStr::NoProject);
		projectName->AddChild(projectNameText);

		Text* worldText = m_manager->Allocate<Text>("WorldText");
		worldText->GetFlags().Set(WF_POS_ALIGN_Y);
		worldText->SetAlignedPosY(0.5f);
		worldText->SetPosAlignmentSourceX(PosAlignmentSource::Center);
		worldText->SetPosAlignmentSourceY(PosAlignmentSource::Center);
		worldText->GetProps().text = Locale::GetStr(LocaleStr::NoWorld);
		projectName->AddChild(worldText);

		DirectionalLayout* wb = CommonWidgets::BuildWindowButtons(this);
		wb->GetFlags().Set(WF_POS_ALIGN_Y | WF_SIZE_ALIGN_Y | WF_USE_FIXED_SIZE_X | WF_POS_ALIGN_X);
		wb->SetPosAlignmentSourceX(PosAlignmentSource::End);
		wb->SetAlignedPos(Vector2(1.0f, 0.0f));
		wb->SetAlignedSizeY(1.0f);
		wb->SetFixedSizeX(Theme::GetDef().baseItemHeight * 6.0f);
		titleBar->AddChild(wb);

		Widget* panelArea = m_manager->Allocate<Widget>("Panel Area");
		panelArea->GetFlags().Set(WF_POS_ALIGN_X | WF_SIZE_ALIGN_X | WF_SIZE_ALIGN_Y);
		panelArea->SetAlignedPosX(0.0f);
		panelArea->SetAlignedSize(Vector2(1.0f, 0.0f));
		// panelArea->GetChildMargins()	 = TBLR::Eq(Theme::GetDef().baseIndentInner);
		// panelArea->GetChildMargins().top = 0.0f;
		AddChild(panelArea);

		DirectionalLayout::Construct();

		m_panelArea		  = panelArea;
		m_saveIcon		  = saveIcon;
		m_fileMenu		  = fm;
		m_windowButtons	  = wb;
		m_projectNameText = projectNameText;
		m_worldNameText	  = worldText;
		m_titleBar		  = titleBar;
		m_linaIcon		  = lina;

		m_worldManager = Editor::Get()->GetSystem()->CastSubsystem<WorldManager>(SubsystemType::WorldManager);
	}

	void EditorRoot::PreTick()
	{
		m_dragRect				  = Rect(Vector2(m_fileMenu->GetRect().GetEnd().x, 0.0f), Vector2(m_windowButtons->GetPos().x - m_fileMenu->GetRect().GetEnd().x, m_fileMenu->GetParent()->GetSizeY()));
		LinaGX::LGXRectui lgxRect = {};
		lgxRect.pos				  = LinaGX::LGXVector2ui{static_cast<uint32>(m_dragRect.pos.x), static_cast<uint32>(m_dragRect.pos.y)};
		lgxRect.size			  = LinaGX::LGXVector2ui{static_cast<uint32>(m_dragRect.size.x), static_cast<uint32>(m_dragRect.size.y)};
		m_lgxWindow->SetDragRect(lgxRect);
	}

	void EditorRoot::Tick(float delta)
	{
		auto* projectData = Editor::Get()->GetProjectManager().GetProjectData();

		if (projectData)
		{
			if (m_saveIcon->GetIsDisabled() && projectData->GetIsDirty())
				m_saveIcon->SetIsDisabled(false);
			else if (!m_saveIcon->GetIsDisabled() && !projectData->GetIsDirty())
				m_saveIcon->SetIsDisabled(true);
		}

		auto* loadedWorld = m_worldManager->GetMainWorld();

		if (loadedWorld != m_currentWorld)
		{
			m_currentWorld					 = loadedWorld;
			m_worldNameText->GetProps().text = FileSystem::GetFilenameOnlyFromPath(m_currentWorld->GetPath());
			m_worldNameText->CalculateTextSize();
		}

		const Color targetColor					  = m_lgxWindow->HasFocus() ? Theme::GetDef().accentPrimary0 : Theme::GetDef().background2;
		m_titleBar->GetProps().colorBackgroundEnd = Math::Lerp(m_titleBar->GetProps().colorBackgroundEnd, targetColor, delta * COLOR_SPEED);

		DirectionalLayout::Tick(delta);
	}

	void EditorRoot::Draw()
	{
		DirectionalLayout::Draw();

		if (m_linaIcon->GetIsHovered())
		{
			Widget::DrawTooltip();
		}
	}

	bool EditorRoot::OnMouse(uint32 button, LinaGX::InputAction act)
	{
		if (button == LINAGX_MOUSE_0 && act == LinaGX::InputAction::Repeated && m_dragRect.IsPointInside(m_lgxWindow->GetMousePosition()))
		{
			if (m_lgxWindow->GetIsMaximized())
				m_lgxWindow->Restore();
			else
				m_lgxWindow->Maximize();
			return true;
		}

		return false;
	}

	void EditorRoot::SetProjectName(const String& name)
	{
		m_projectNameText->GetProps().text = name;
		m_projectNameText->Initialize();
	}

	bool EditorRoot::OnFileMenuItemClicked(FileMenu* filemenu, StringID sid, void* userData)
	{
		if (sid == TO_SID(Locale::GetStr(LocaleStr::NewProject)))
		{
			Editor::Get()->GetProjectManager().OpenPopupProjectSelector(true, true);
			return true;
		}

		if (sid == TO_SID(Locale::GetStr(LocaleStr::LoadProject)))
		{
			Editor::Get()->GetProjectManager().OpenPopupProjectSelector(true, false);
			return true;
		}

		if (sid == TO_SID(Locale::GetStr(LocaleStr::SaveProject)))
		{
			Editor::Get()->GetProjectManager().SaveProjectChanges();
			return true;
		}

		if (sid == TO_SID(Locale::GetStr(LocaleStr::Exit)))
		{
			Editor::Get()->RequestExit();
			return true;
		}

		if (sid == TO_SID(Locale::GetStr(LocaleStr::Resources)))
		{
			Editor::Get()->GetWindowPanelManager().OpenPanel(PanelType::Resources, 0, this);
			return true;
		}

		if (sid == TO_SID(Locale::GetStr(LocaleStr::Entities)))
		{
			Editor::Get()->GetWindowPanelManager().OpenPanel(PanelType::Entities, 0, this);
			return true;
		}

		if (sid == TO_SID(Locale::GetStr(LocaleStr::World)))
		{
			Editor::Get()->GetWindowPanelManager().OpenPanel(PanelType::World, 0, this);
			return true;
		}

		if (sid == TO_SID(Locale::GetStr(LocaleStr::Performance)))
		{
			Editor::Get()->GetWindowPanelManager().OpenPanel(PanelType::Performance, 0, this);
			return true;
		}

		if (sid == TO_SID(Locale::GetStr(LocaleStr::ResetLayout)))
		{
			for (auto* w : m_panelArea->GetChildren())
				m_manager->Deallocate(w);

			m_panelArea->RemoveAllChildren();

			Editor::Get()->GetWindowPanelManager().CloseAllSubwindows();
			Editor::Get()->GetSettings().GetLayout().StoreDefaultLayout();
			Editor::Get()->GetSettings().GetLayout().ApplyStoredLayout();
			return true;
		}

		if (sid == TO_SID(Locale::GetStr(LocaleStr::Website)))
		{
			PlatformProcess::OpenURL("https://inanevin.com");
			return true;
		}

		if (sid == TO_SID(Locale::GetStr(LocaleStr::Github)))
		{
			PlatformProcess::OpenURL("https://github.com/inanevin");
			return true;
		}

		if (sid == TO_SID(Locale::GetStr(LocaleStr::More)))
		{
			return true;
		}

		return false;
	}

	void EditorRoot::OnFileMenuGetItems(FileMenu* filemenu, StringID sid, Vector<FileMenuItem::Data>& outData, void* userData)
	{
		if (sid == TO_SID(Locale::GetStr(LocaleStr::File)))
		{
			outData = {
				FileMenuItem::Data{.text = Locale::GetStr(LocaleStr::NewProject)},
				FileMenuItem::Data{
					.text = Locale::GetStr(LocaleStr::LoadProject),
				},
				FileMenuItem::Data{.text = Locale::GetStr(LocaleStr::SaveProject)},
				FileMenuItem::Data{.isDivider = true},
				FileMenuItem::Data{.text = Locale::GetStr(LocaleStr::Exit)},
			};
			return;
		}

		if (sid == TO_SID(Locale::GetStr(LocaleStr::Panels)))
		{
			outData = {
				FileMenuItem::Data{.text = Locale::GetStr(LocaleStr::Entities)},
				FileMenuItem::Data{.text = Locale::GetStr(LocaleStr::World)},
				FileMenuItem::Data{.text = Locale::GetStr(LocaleStr::Resources)},
				FileMenuItem::Data{.text = Locale::GetStr(LocaleStr::Performance)},
				FileMenuItem::Data{.isDivider = true},
				FileMenuItem::Data{.text = Locale::GetStr(LocaleStr::ResetLayout)},
			};
			return;
		}

		if (sid == TO_SID(Locale::GetStr(LocaleStr::About)))
		{
			outData = {
				FileMenuItem::Data{.text = Locale::GetStr(LocaleStr::Website)},
				FileMenuItem::Data{.text = Locale::GetStr(LocaleStr::Github)},
				FileMenuItem::Data{.text = Locale::GetStr(LocaleStr::More)},
			};
			return;
		}
	}

} // namespace Lina::Editor
