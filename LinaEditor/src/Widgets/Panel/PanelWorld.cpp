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

#include "Editor/Widgets/Panel/PanelWorld.hpp"
#include "Editor/Editor.hpp"
#include "Editor/EditorLocale.hpp"
#include "Core/GUI/Widgets/WidgetManager.hpp"
#include "Editor/Widgets/World/WorldDisplayer.hpp"
#include "Common/Platform/LinaVGIncl.hpp"

#include "Common/Math/Math.hpp"
#include "Core/World/EntityWorld.hpp"
#include "Core/Graphics/Resource/Texture.hpp"
#include "Core/Graphics/Renderers/WorldRenderer.hpp"
#include "Core/Application.hpp"

namespace Lina::Editor
{
	void PanelWorld::Construct()
	{
		Panel::Construct();
		m_editor = Editor::Get();

		GetWidgetProps().childMargins = TBLR::Eq(Theme::GetDef().baseIndent);

		m_worldDisplayer = m_manager->Allocate<WorldDisplayer>("WorldDisplayer");
		m_worldDisplayer->GetFlags().Set(WF_POS_ALIGN_X | WF_POS_ALIGN_Y | WF_SIZE_ALIGN_X | WF_SIZE_ALIGN_Y);
		m_worldDisplayer->SetAlignedPos(Vector2::Zero);
		m_worldDisplayer->SetAlignedSize(Vector2::One);
		m_worldDisplayer->GetProps().noWorldText = Locale::GetStr(LocaleStr::NoWorldInstalled);

		AddChild(m_worldDisplayer);
	}

	void PanelWorld::Destruct()
	{
		m_editor->GetWorldManager().CloseWorld();
	}

	void PanelWorld::Tick(float delta)
	{
		// m_world = m_wm->GetMainWorld();
		// if (!m_world)
		// 	return;
		//
		// if (m_worldRenderer == nullptr)
		// 	m_worldRenderer = Editor::Get()->GetWorldRenderer(m_world);
		//
		// const Vector2ui size = Vector2ui(static_cast<uint32>(Math::CeilToInt(GetSizeX())), static_cast<uint32>(Math::CeilToInt(GetSizeY())));
		// m_worldRenderer->Resize(size);
	}

	void PanelWorld::SetWorld(EntityWorld* world, WorldRenderer* worldRenderer)
	{
		m_world			= world;
		m_worldRenderer = worldRenderer;
		m_worldDisplayer->DisplayWorld(m_worldRenderer, WorldDisplayer::WorldCameraType::Orbit);
	}

} // namespace Lina::Editor
