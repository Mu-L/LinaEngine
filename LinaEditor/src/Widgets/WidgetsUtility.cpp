/*
This file is a part of: Lina Engine
https://github.com/inanevin/LinaEngine

Author: Inan Evin
http://www.inanevin.com

Copyright (c) [2018-2020] [Inan Evin]

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

#include "Widgets/WidgetsUtility.hpp"
#include "Core/GUILayer.hpp"
#include "EventSystem/EventSystem.hpp"
#include "Core/WindowBackend.hpp"
#include "Core/EditorCommon.hpp"
#include "Math/Math.hpp"
#include "Math/Quaternion.hpp"
#include "Rendering/Material.hpp"
#include "Rendering/Model.hpp"
#include "Rendering/Shader.hpp"
#include "Physics/PhysicsMaterial.hpp"
#include "Drawers/ComponentDrawer.hpp"
#include "Utility/UtilityFunctions.hpp"
#include "Core/CustomFontIcons.hpp"
#include "Core/PhysicsCommon.hpp"
#include "Core/InputBackend.hpp"
#include "IconsFontAwesome5.h"

namespace Lina::Editor
{
#define COMBOBOX_WIDTH_1 12.4f
#define COMBOBOX_WIDTH_2 28.0f
#define COMBOBOX_WIDTH_3 22.4f
#define DRAG_POWER 0.01f
#define VALUE_OFFSET_FROM_WINDOW 10
#define HEADER_WIDGET_HEIGHT 25
#define DEFAULT_TOGGLE_SIZE ImVec2(35.0f, 15.0f)

	static bool s_isDraggingWidgetInput = false;
	static std::string s_draggedInput = "";
	static float s_valueOnDragStart = 0.0f;
	static int s_valueOnDragStartInt = 0;

	void WidgetsUtility::Tooltip(const char* tooltip)
	{
		if (s_isDraggingWidgetInput) return;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(9, 2));
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::BeginTooltip();
		ImGui::Text(tooltip);
		ImGui::EndTooltip();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
	}

	bool WidgetsUtility::CustomButton(const char* id, ImVec2 size, bool* isHovered, bool locked, const char* icon, float rounding, const char* tooltip)
	{
		const ImVec2 currentCursor = ImGui::GetCursorPos();
		const ImVec2 currentPos = ImVec2(ImGui::GetWindowPos().x + ImGui::GetCursorPosX(), ImGui::GetWindowPos().y + ImGui::GetCursorPos().y);
		const ImRect absoluteRect = ImRect(ImVec2(currentPos.x, currentPos.y), ImVec2(size.x + currentPos.x, size.y + currentPos.y));

		ImGui::ItemAdd(absoluteRect, ImGuiID(id));
		ImGui::ItemSize(size);

		bool hovered = ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(absoluteRect.Min, absoluteRect.Max) && !ImGui::IsAnyItemHovered();
		if (isHovered != nullptr)
			*isHovered = hovered;

		const bool pressing = hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left);

		const ImVec4 normalColor = ImGui::GetStyleColorVec4(ImGuiCol_Button);
		const ImVec4 lockedColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonLocked);
		const ImVec4 hoverColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
		const ImVec4 rectCol = locked ? lockedColor : pressing ? normalColor : (hovered ? hoverColor : normalColor);
		const ImU32 borderColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.3f, 0.3f, 0.3f, 0.55f));
		ImGui::GetWindowDrawList()->AddRectFilled(absoluteRect.Min, absoluteRect.Max, ImGui::ColorConvertFloat4ToU32(rectCol), rounding);

		if (icon != nullptr)
		{
			const float iconScale = size.y * 0.035f;
			PushScaledFont(iconScale);
			ImVec2 textSize = ImGui::CalcTextSize(icon);

			ImGui::SameLine();
			ImGui::SetCursorPosX(currentCursor.x + size.x / 2.0f - textSize.x / 2.0f);
			ImGui::SetCursorPosY(currentCursor.y + size.y / 2.0f - textSize.y / 2.0f + 0.6f);
			ImGui::Text(icon);
			PopScaledFont();
		}

		if (tooltip != nullptr && hovered)
			Tooltip(tooltip);

		if (!locked && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && hovered)
			return true;

		return false;
	}

	bool WidgetsUtility::CustomToggle(const char* id, ImVec2 size, bool toggled, bool* hoveredPtr, const char* icon, float rounding, const char* tooltip)
	{
		const ImVec2 currentCursor = ImGui::GetCursorPos();
		const ImVec2 currentPos = ImVec2(ImGui::GetWindowPos().x + ImGui::GetCursorPosX(), ImGui::GetWindowPos().y + ImGui::GetCursorPos().y);
		const ImRect absoluteRect = ImRect(ImVec2(currentPos.x, currentPos.y), ImVec2(size.x + currentPos.x, size.y + currentPos.y));

		ImGui::ItemAdd(absoluteRect, ImGuiID(id));
		ImGui::ItemSize(size);

		bool hovered = ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(absoluteRect.Min, absoluteRect.Max) && !ImGui::IsAnyItemHovered();
		const bool pressing = hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left);

		if (hoveredPtr != nullptr)
			*hoveredPtr = hovered;
		const ImVec4 normalColor = ImGui::GetStyleColorVec4(ImGuiCol_Button);
		const ImVec4 lockedColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonLocked);
		const ImVec4 hoverColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
		const ImVec4 rectCol = pressing ? normalColor : (hovered ? hoverColor : toggled ? lockedColor : normalColor);
		const ImU32 borderColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.3f, 0.3f, 0.3f, 0.55f));
		ImGui::GetWindowDrawList()->AddRectFilled(absoluteRect.Min, absoluteRect.Max, ImGui::ColorConvertFloat4ToU32(rectCol), rounding);
		IncrementCursorPosY(size.y / 2.0f);

		if (icon != nullptr)
		{
			const float iconScale = size.y * 0.035f;
			PushScaledFont(iconScale);
			ImVec2 textSize = ImGui::CalcTextSize(icon);

			ImGui::SameLine();
			ImGui::SetCursorPosX(currentCursor.x + size.x / 2.0f - textSize.x / 2.0f);
			ImGui::SetCursorPosY(currentCursor.y + size.y / 2.0f - textSize.y / 2.0f);
			ImGui::Text(icon);
			PopScaledFont();
		}

		if (tooltip != nullptr && hovered)
			Tooltip(tooltip);

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && hovered)
			return true;

		return false;
	}

	bool WidgetsUtility::ColorsEqual(ImVec4 col1, ImVec4 col2)
	{
		return (col1.x == col2.x && col1.y == col2.y && col1.z == col2.z && col1.w == col2.w);
	}

	void WidgetsUtility::DrawTreeFolder(Utility::Folder& folder, Utility::Folder*& selectedFolder, Utility::Folder*& hoveredFolder, float height, float offset, ImVec4 defaultBackground, ImVec4 hoverBackground, ImVec4 selectedBackground)
	{

		if (ColorsEqual(hoverBackground, ImVec4(0, 0, 0, 0)))
			hoverBackground = ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered);

		if (ColorsEqual(selectedBackground, ImVec4(0, 0, 0, 0)))
			selectedBackground = ImGui::GetStyleColorVec4(ImGuiCol_Header);


		ImVec2 pos = ImGui::GetCurrentWindow()->Pos;
		ImVec2 size = ImGui::GetCurrentWindow()->Size;
		ImVec2 min = ImVec2(ImGui::GetCurrentWindow()->Pos.x, -ImGui::GetScrollY() + ImGui::GetCursorPosY() + ImGui::GetCurrentWindow()->Pos.y);
		ImVec2 max = ImVec2(min.x + size.x, min.y + height);

		// Hover state.
		if (ImGui::IsMouseHoveringRect(min, max) && ImGui::IsWindowFocused())
			hoveredFolder = &folder;

		bool hovered = hoveredFolder == &folder;
		bool selected = selectedFolder == &folder;
		bool open = folder.m_isOpen;

		// Color & selection
		ImVec4 background = selected ? selectedBackground : (hovered ? hoverBackground : defaultBackground);
		ImVec4 usedBackground = background;
		if (hovered)
		{
			if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				usedBackground = ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive);
			}
			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				selectedFolder = &folder;
			}
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				selectedFolder = &folder;
			}
		}

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered())
				selectedFolder = nullptr;
		}


		// Background
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImGui::ColorConvertFloat4ToU32(usedBackground));

		if (hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			folder.m_isOpen = !folder.m_isOpen;

		// Draw the folder data
		PushScaledFont(0.7f);

		IncrementCursorPosY(6);
		ImGui::SetCursorPosX(4 + offset);

		if (folder.m_folders.size() != 0)
		{
			if (selected)
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
			else
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));

			if (open)
				ImGui::Text(ICON_FA_CARET_DOWN);
			else
				ImGui::Text(ICON_FA_CARET_RIGHT);

			ImGui::PopStyleColor();


			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				folder.m_isOpen = !folder.m_isOpen;

			ImGui::SameLine();
		}

		IncrementCursorPosY(0);
		PopScaledFont();
		ImGui::SetCursorPosX(4 + offset + 12);

		// color , ImVec4(0.7f, 0.7f, 0.7f, 1.0f)
		Icon(ICON_FA_FOLDER, 0.7f);

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
			folder.m_isOpen = !folder.m_isOpen;

		ImGui::SameLine();
		IncrementCursorPosY(-5);
		ImGui::Text(folder.m_name.c_str());



		//	ImGui::EndChild();
		//	ImGui::PopStyleVar();
	}

	void WidgetsUtility::ColorButton(const char* id, float* colorX)
	{
		static bool alpha_preview = true;
		static bool alpha_half_preview = false;
		static bool drag_and_drop = true;
		static bool options_menu = true;
		static bool hdr = true;
		ImGuiColorEditFlags misc_flags = (hdr ? (ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float) : 0) | (drag_and_drop ? 0 : ImGuiColorEditFlags_NoDragDrop) | (alpha_half_preview ? ImGuiColorEditFlags_AlphaPreviewHalf : (alpha_preview ? ImGuiColorEditFlags_AlphaPreview : 0)) | (options_menu ? 0 : ImGuiColorEditFlags_NoOptions);
		// Generate a dummy default palette. The palette will persist and can be edited.
		static bool saved_palette_init = true;
		static ImVec4 saved_palette[32] = {};
		if (saved_palette_init)
		{
			for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++)
			{
				ImGui::ColorConvertHSVtoRGB(n / 31.0f, 0.8f, 0.8f,
					saved_palette[n].x, saved_palette[n].y, saved_palette[n].z);
				saved_palette[n].w = 1.0f; // Alpha
			}
			saved_palette_init = false;
		}

		std::string buf(id);
		static ImVec4 backup_color;
		bool open_popup = ImGui::ColorButton(buf.c_str(), ImVec4(colorX[0], colorX[1], colorX[2], colorX[3]), misc_flags);
		buf.append("##p");
		if (open_popup)
		{
			ImGui::OpenPopup(buf.c_str());
			backup_color = ImVec4(colorX[0], colorX[1], colorX[2], colorX[3]);
		}
		if (ImGui::BeginPopup(buf.c_str()))
		{
			ImGui::PushItemWidth(160);

			buf.append("##picker");
			ImGui::Separator();
			ImGui::ColorPicker4(buf.c_str(), colorX, misc_flags | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
			ImGui::SameLine();

			buf.append("##current");
			ImGui::BeginGroup(); // Lock X position
			ImGui::Text("Current");
			ImGui::ColorButton(buf.c_str(), ImVec4(colorX[0], colorX[1], colorX[2], colorX[3]), ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40));
			ImGui::Text("Previous");

			if (ImGui::ColorButton("##previous", backup_color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40)))
			{
				colorX[0] = backup_color.x;
				colorX[1] = backup_color.y;
				colorX[2] = backup_color.z;
				colorX[3] = backup_color.w;
			}

			ImGui::EndGroup();
			ImGui::EndPopup();
		}
	}

	bool WidgetsUtility::ToggleButton(const char* label, bool* v, ImVec2 size)
	{
		const ImVec4 inactiveColor = ImVec4(0.85f, 0.85f, 0.85f, 1.0f);
		const ImVec4 inactiveHoveredColor = ImVec4(0.78f, 0.78f, 0.78f, 1.0f);
		const ImVec4 activeColor = ImGui::GetStyleColorVec4(ImGuiCol_Header);
		const ImVec4 activeHoveredColor = ImGui::GetStyleColorVec4(ImGuiCol_Header);

		ImVec2 p = ImGui::GetCursorScreenPos();
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		if (size.x == 0.0f && size.y == 0.0f)
			size = DEFAULT_TOGGLE_SIZE;

		float radius = size.y * 0.50f;

		ImGui::InvisibleButton(label, size);
		if (v != nullptr && ImGui::IsItemClicked())
			*v = !*v;

		float t = v == nullptr ? 1.0f : *v ? 1.0f : 0.0f;

		ImVec4 usedActiveColor = activeColor;
		if (v == nullptr)
			usedActiveColor = ImVec4(activeColor.x - 0.15f, activeColor.y - 0.15f, activeColor.z - 0.15f, activeColor.w);

		ImGuiContext& g = *GImGui;
		float ANIM_SPEED = 0.08f;
		if (v != nullptr && g.LastActiveId == g.CurrentWindow->GetID(label))// && g.LastActiveIdTimer < ANIM_SPEED)
		{
			float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
			t = *v ? (t_anim) : (1.0f - t_anim);
		}

		ImU32 col_bg;
		if (ImGui::IsItemHovered() && v != nullptr)
			col_bg = ImGui::GetColorU32(ImLerp(inactiveHoveredColor, activeHoveredColor, t));
		else
			col_bg = ImGui::GetColorU32(ImLerp(inactiveColor, usedActiveColor, t));

		draw_list->AddRectFilled(p, ImVec2(p.x + size.x, p.y + size.y), col_bg, size.y * 0.5f);
		draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (size.x - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));

		return v != nullptr ? *v : false;
	}

	void WidgetsUtility::HorizontalDivider(float yOffset, float thickness)
	{
		ImVec2 min = ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetCursorPosY() + yOffset);
		ImVec2 max = ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowWidth(), min.y);
		ImGui::GetWindowDrawList()->AddLine(min, max, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyleColorVec4(ImGuiCol_Text)), thickness);
	}

	void WidgetsUtility::WindowButtons(const char* windowID, float yOffset, bool isAppWindow)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));

		ImVec2 windowPos = ImGui::GetWindowPos();
		float windowWidth = ImGui::GetWindowWidth();

		static float offset1 = 28.0f;
		static float gap = 25;

		ImGui::SetCursorPosY(yOffset);
		const float cursorY = ImGui::GetCursorPosY();
		ImGui::SetCursorPosX(windowWidth - offset1 - gap * 2);
		if (WidgetsUtility::CustomButton("##header_minimize", ImVec2(20, 20), nullptr, false, ICON_FA_WINDOW_MINIMIZE, 2.0f))
		{
			if (isAppWindow)
				Lina::Graphics::WindowBackend::Get()->Iconify();
			else
				GUILayer::s_editorPanels[windowID]->ToggleCollapse();
		}

		ImGui::SameLine();
		ImGui::SetCursorPosX(windowWidth - offset1 - gap);
		ImGui::SetCursorPosY(cursorY);
		if (WidgetsUtility::CustomButton("##header_maximize", ImVec2(20, 20), nullptr, false, ICON_FA_WINDOW_MAXIMIZE, 2.0f))
		{
			if (isAppWindow)
				Lina::Graphics::WindowBackend::Get()->Maximize();
			else
				GUILayer::s_editorPanels[windowID]->ToggleMaximize();
		}

		ImGui::SameLine();
		ImGui::SetCursorPosX(windowWidth - offset1);
		ImGui::SetCursorPosY(cursorY);
		if (WidgetsUtility::CustomButton("##header_minimize", ImVec2(20, 20), nullptr, false, ICON_FA_TIMES, 2.0f))
		{
			if (isAppWindow)
				Lina::Graphics::WindowBackend::Get()->Close();
			else
				GUILayer::s_editorPanels[windowID]->Close();
		}

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
	}

	void WidgetsUtility::WindowTitlebar(const char* label)
	{
		if (ImGui::IsWindowDocked()) return;

		const ImVec2 windowPos = ImGui::GetWindowPos();
		const ImVec2 windowSize = ImGui::GetWindowSize();
		const float height = 30.0f;
		const float lineOffset = 2.0f;

		// Draw title rect & line.
		ImRect titleRect = ImRect(windowPos, ImVec2(windowPos.x + windowSize.x, windowPos.y + height));
		ImVec4 titleRectColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
		ImGui::GetWindowDrawList()->AddRectFilled(titleRect.Min, titleRect.Max, ImGui::ColorConvertFloat4ToU32(titleRectColor));
		ImGui::GetWindowDrawList()->AddLine(ImVec2(windowPos.x, windowPos.y + lineOffset), ImVec2(windowPos.x + windowSize.x, windowPos.y + lineOffset), ImGui::ColorConvertFloat4ToU32(ImGui::GetStyleColorVec4(ImGuiCol_Header)), 4.0f);

		// Draw title
		ImGui::SetCursorPosX(12.5f);
		ImGui::Text(label);

		// Draw Buttons
		WindowButtons(label, 5.0f);
	}

	bool WidgetsUtility::ComponentHeader(Lina::ECS::TypeID tid, bool* foldoutOpen, const char* componentLabel, const char* componentIcon, bool* toggled, bool* removed, bool* copied, bool* pasted, bool* resetted, bool moveButton)
	{
		// Draw header.
		ImVec2 cursorPos = ImVec2(0, 0);
		Header(componentLabel, foldoutOpen, &cursorPos);
		const ImVec2 cursorPosAfterHeader = ImGui::GetCursorPos();

		// Title is the drag and drop target.
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			// Set payload to carry the type id.
			ImGui::SetDragDropPayload("COMP_MOVE_PAYLOAD", &tid, sizeof(int));

			// Display preview 
			ImGui::Text("Move ");
			ImGui::EndDragDropSource();
		}

		// Dropped on another title, swap component orders.
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("COMP_MOVE_PAYLOAD"))
			{
				IM_ASSERT(payload->DataSize == sizeof(Lina::ECS::TypeID));
				Lina::ECS::TypeID payloadID = *(const Lina::ECS::TypeID*)payload->Data;
				Lina::Event::EventSystem::Get()->Trigger<EComponentOrderSwapped>(EComponentOrderSwapped{ payloadID, tid });
			}

			ImGui::EndDragDropTarget();
		}

		// Draw component icon.
		ImGui::SetCursorPos(cursorPos);
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_Header));
		IncrementCursorPosY(10.2f);
		IncrementCursorPosX(ImGui::GetStyle().ItemSpacing.x);
		Icon(componentIcon, true, 0.7f);
		ImGui::PopStyleColor();
		ImGui::SameLine();


		ImGui::SetCursorPos(cursorPos);
		const float cursorPosX = ImGui::GetWindowWidth() - VALUE_OFFSET_FROM_WINDOW;
		const float buttonWidth = 26.0f;
		const float buttonHeight = 20.0f;

		std::vector<std::pair<const char*, bool*>> buttons;
		std::vector<std::string> buttonNames{ "Remove", "Paste", "Copy", "Reset" };
		buttons.push_back(std::make_pair(ICON_FA_TIMES, removed));
		buttons.push_back(std::make_pair(ICON_FA_PASTE, pasted));
		buttons.push_back(std::make_pair(ICON_FA_COPY, copied));
		buttons.push_back(std::make_pair(ICON_FA_SYNC_ALT, resetted));

		float lastCursorX = 0.0f;

		for (int i = 0; i < buttons.size(); i++)
		{
			lastCursorX = cursorPosX - buttonWidth * (i + 1) - (ImGui::GetStyle().ItemSpacing.x * i);
			ImGui::SetCursorPosX(lastCursorX);

			if (buttons[i].second == nullptr)
				ImGui::BeginDisabled();

			if (Button(buttons[i].first, ImVec2(buttonWidth, buttonHeight), 0.6f, 2.0f))
				*buttons[i].second = !*buttons[i].second;

			if (buttons[i].second == nullptr)
				ImGui::EndDisabled();


			ImGui::SameLine();
		}

		const ImVec2 toggleSize = DEFAULT_TOGGLE_SIZE;

		static float w = 3.5f;
		if (Input::InputEngineBackend::Get()->GetKey(LINA_KEY_Q))
			w += 0.1f;
		if (Input::InputEngineBackend::Get()->GetKey(LINA_KEY_E))
		w-= 0.1f;
		
		// Toggle
		const std::string toggleID = "##_toggle_" + std::string(componentLabel);
		ImGui::SetCursorPosX(lastCursorX - toggleSize.x - ImGui::GetStyle().ItemSpacing.x);
		IncrementCursorPosY(2.8f);
		ToggleButton(toggleID.c_str(), toggled);
		ImGui::SameLine();
		lastCursorX = ImGui::GetCursorPosX();
	
		ImGui::SetCursorPos(cursorPosAfterHeader);
		return *foldoutOpen;
	
	}

	bool WidgetsUtility::Header(const char* label, bool* foldoutOpen, ImVec2* outCursorPos)
	{
		ImGui::BeginGroup();

		const ImVec2 windowSize = ImGui::GetWindowSize();
		const ImVec2 rectSize = ImVec2(windowSize.x, HEADER_WIDGET_HEIGHT);
		const ImVec4 normalColor = ImVec4(0.03f, 0.03f, 0.03f, 1.0f);
		const ImVec4 hoverColor = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
		const ImVec2 cursorPosBeforeButton = ImGui::GetCursorPos();
		const float iconScale = 0.65f;

		ImGui::PushStyleColor(ImGuiCol_Button, normalColor);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverColor);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, normalColor);

		const std::string id = "##_" + std::string(label);
		if (Button(id.c_str(), rectSize))
			*foldoutOpen = !*foldoutOpen;

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		const ImVec2 cursorPosAfterButton = ImGui::GetCursorPos();
		const ImVec2 textSize = ImGui::CalcTextSize(label);
		const ImVec2 cursorPosInside = ImVec2(cursorPosBeforeButton.x + VALUE_OFFSET_FROM_WINDOW, cursorPosBeforeButton.y + rectSize.y / 2.0f - textSize.y / 2.0f);
		ImGui::SetCursorPos(cursorPosInside);
		Icon(*foldoutOpen ? ICON_FA_CARET_DOWN : ICON_FA_CARET_RIGHT, false, 1.0f);
		ImGui::SameLine();
		ImGui::Text(label);
		ImGui::SameLine();
		const ImVec2 cursorPosFinal = ImGui::GetCursorPos();

		if (outCursorPos != nullptr)
			*outCursorPos = cursorPosFinal;

		ImGui::SetCursorPos(cursorPosAfterButton);

		ImGui::EndGroup();
		return *foldoutOpen;
	}

	void WidgetsUtility::DropShadow()
	{
		static int countS = 4;
		static float thicknessS = 4.0f;
		static float yOffsetS = 3.0f;

		ImVec4 color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
		int dividerCount = 4;
		float dividerSize = 4.0f;
		float yOffset = 3.0f;
		float colorFade = 1.0f / (float)dividerCount;

		for (int i = 0; i < dividerCount; i++)
		{
			color.w = 1.0f - i * colorFade;
			ImGui::PushStyleColor(ImGuiCol_Text, color);
			HorizontalDivider(i * yOffset, thicknessS);
			ImGui::PopStyleColor();
		}
	}

	bool WidgetsUtility::CaretTitle(const char* title, bool* caretOpen)
	{
		bool clicked = false;
		bool hovered = false;
		ImVec4 caretNormalColor = ImVec4(0.75f, 0.75f, 0.75f, 1.0f);
		ImVec4 caretHoverColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		ImVec4 caretColor = hovered ? caretHoverColor : caretNormalColor;

		ImGui::SetCursorPosX(CURSOR_X_LABELS);

		ImGui::PushStyleColor(ImGuiCol_Text, caretColor);
		ImGui::Text(*caretOpen ? ICON_FA_CARET_DOWN : ICON_FA_CARET_RIGHT);
		ImGui::PopStyleColor();

		if (ImGui::IsItemClicked())
			clicked = true;

		ImGui::SameLine();
		IncrementCursorPosY(0);
		ImGui::Text(title);

		if (ImGui::IsItemClicked())
			clicked = true;

		if (clicked)
			*caretOpen = !*caretOpen;

		return *caretOpen;
	}

	void WidgetsUtility::PropertyLabel(const char* label, bool sameLine)
	{
		ImGui::SetCursorPosX(CURSOR_X_LABELS);
		ImGui::AlignTextToFramePadding();
		ImGui::Text(label);

		if (sameLine)
		{
			ImGui::SameLine();
			ImGui::SetCursorPosX(CURSOR_X_VALUES);
		}
	}

	Lina::Graphics::Material* WidgetsUtility::MaterialComboBox(const char* comboID, const std::string& currentPath, bool* removed)
	{
		Graphics::Material* materialToReturn = nullptr;

		std::string materialLabel = "";
		if (Graphics::Material::MaterialExists(currentPath))
		{
			materialLabel = Utility::GetFileWithoutExtension(Utility::GetFileNameOnly(currentPath));
		}


		float currentCursor = ImGui::GetCursorPosX();
		float windowWidth = ImGui::GetWindowWidth();
		float remaining = windowWidth - currentCursor;
		float comboWidth = removed == nullptr ? remaining - COMBOBOX_WIDTH_1 : remaining - COMBOBOX_WIDTH_2;
		ImGui::SetNextItemWidth(comboWidth);

		if (ImGui::BeginCombo(comboID, materialLabel.c_str()))
		{
			auto& loadedMaterials = Graphics::Material::GetLoadedMaterials();

			for (auto& material : loadedMaterials)
			{
				const bool selected = currentPath == material.second.GetPath();

				std::string label = material.second.GetPath();
				label = Utility::GetFileWithoutExtension(Utility::GetFileNameOnly(label));
				if (ImGui::Selectable(label.c_str(), selected))
				{
					materialToReturn = &material.second;
				}

				if (selected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		if (removed != nullptr)
		{
			//ImGui::SameLine();
			//ImGui::SetCursorPosX(windowWidth - COMBOBOX_WIDTH_3);
			//WidgetsUtility::IncrementCursorPosY(6);

			// Remove Model
			const std::string removeID = "##removeMaterial_" + std::string(comboID);
			//if (WidgetsUtility::IconButton(removeID.c_str(), ICON_FA_MINUS_SQUARE, 0.0f, .7f, ImVec4(1, 1, 1, 0.8f), ImVec4(1, 1, 1, 1), ImGui::GetStyleColorVec4(ImGuiCol_Header)))
			//	*removed = true;

			if (ImGui::IsItemHovered())
				Tooltip("Remove");

		}

		return materialToReturn;
	}

	Lina::Graphics::Model* WidgetsUtility::ModelComboBox(const char* comboID, int currentModelID, bool* removed)
	{
		Lina::Graphics::Model* modelToReturn = nullptr;

		std::string modelLabel = "";
		if (Graphics::Model::ModelExists(currentModelID))
		{
			const std::string modelLabelFull = Graphics::Model::GetModel(currentModelID).GetPath();
			modelLabel = Utility::GetFileWithoutExtension(Utility::GetFileNameOnly(modelLabelFull));
		}

		float currentCursor = ImGui::GetCursorPosX();
		float windowWidth = ImGui::GetWindowWidth();
		float remaining = windowWidth - currentCursor;
		float comboWidth = removed == nullptr ? remaining - COMBOBOX_WIDTH_1 : remaining - COMBOBOX_WIDTH_2;

		ImGui::SetNextItemWidth(comboWidth);
		if (ImGui::BeginCombo(comboID, modelLabel.c_str()))
		{
			auto& loadedModels = Graphics::Model::GetLoadedModels();

			for (auto& model : loadedModels)
			{
				const bool selected = currentModelID == model.second.GetID();

				std::string label = model.second.GetPath();
				label = Utility::GetFileWithoutExtension(Utility::GetFileNameOnly(label));
				if (ImGui::Selectable(label.c_str(), selected))
				{
					modelToReturn = &model.second;
				}

				if (selected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		if (removed != nullptr)
		{
			//ImGui::SameLine();
			//ImGui::SetCursorPosX(windowWidth - COMBOBOX_WIDTH_3);
			//WidgetsUtility::IncrementCursorPosY(6);

			// Remove
			const std::string removeID = "##removeModel_" + std::string(comboID);
			//if (WidgetsUtility::IconButton(removeID.c_str(), ICON_FA_MINUS_SQUARE, 0.0f, .7f, ImVec4(1, 1, 1, 0.8f), ImVec4(1, 1, 1, 1), ImGui::GetStyleColorVec4(ImGuiCol_Header)))
			//	*removed = true;

			if (ImGui::IsItemHovered())
				Tooltip("Remove");
		}

		return modelToReturn;
	}

	Lina::Graphics::Shader* WidgetsUtility::ShaderComboBox(const char* comboID, int currentShaderID, bool* removed)
	{
		Lina::Graphics::Shader* shaderToReturn = nullptr;

		std::string shaderLabel = "";
		if (Graphics::Shader::ShaderExists(currentShaderID))
		{
			const std::string shaderLabelFull = Graphics::Shader::GetShader(currentShaderID).GetPath();
			shaderLabel = Utility::GetFileWithoutExtension(Utility::GetFileNameOnly(shaderLabelFull));
		}


		float currentCursor = ImGui::GetCursorPosX();
		float windowWidth = ImGui::GetWindowWidth();
		float remaining = windowWidth - currentCursor;
		float comboWidth = removed == nullptr ? remaining - COMBOBOX_WIDTH_1 : remaining - COMBOBOX_WIDTH_2;
		if (ImGui::BeginCombo(comboID, shaderLabel.c_str()))
		{
			auto& loadedShaders = Lina::Graphics::Shader::GetLoadedShaders();

			for (auto& shader : loadedShaders)
			{
				const bool selected = currentShaderID == shader.second->GetSID();

				std::string label = shader.second->GetPath();
				label = Utility::GetFileWithoutExtension(Utility::GetFileNameOnly(label));
				if (ImGui::Selectable(label.c_str(), selected))
				{
					shaderToReturn = shader.second;
				}

				if (selected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		if (removed != nullptr)
		{
			//ImGui::SameLine();
			//ImGui::SetCursorPosX(windowWidth - COMBOBOX_WIDTH_3);
			WidgetsUtility::IncrementCursorPosY(6);

			// Remove Model
			const std::string removeID = "##removeShader_" + std::string(comboID);
			//if (WidgetsUtility::IconButton(removeID.c_str(), ICON_FA_MINUS_SQUARE, 0.0f, .7f, ImVec4(1, 1, 1, 0.8f), ImVec4(1, 1, 1, 1), ImGui::GetStyleColorVec4(ImGuiCol_Header)))
			//	*removed = true;


			if (ImGui::IsItemHovered())
				Tooltip("Remove");
		}


		return shaderToReturn;
	}

	Lina::Physics::PhysicsMaterial* Lina::Editor::WidgetsUtility::PhysicsMaterialComboBox(const char* comboID, const std::string& currentPath, bool* removed)
	{
		Physics::PhysicsMaterial* materialToReturn = nullptr;

		std::string materialLabel = "";
		if (Physics::PhysicsMaterial::MaterialExists(currentPath))
		{
			materialLabel = Utility::GetFileWithoutExtension(Utility::GetFileNameOnly(currentPath));
		}

		static float w = 0.0f;

		if (Lina::Input::InputEngineBackend::Get()->GetKey(LINA_KEY_Q))
			w += 0.01f;
		if (Lina::Input::InputEngineBackend::Get()->GetKey(LINA_KEY_E))
			w -= 0.01f;

		float currentCursor = ImGui::GetCursorPosX();
		float windowWidth = ImGui::GetWindowWidth();
		float remaining = windowWidth - currentCursor;
		float comboWidth = removed == nullptr ? remaining - COMBOBOX_WIDTH_1 - w : remaining - COMBOBOX_WIDTH_2;
		ImGui::SetNextItemWidth(comboWidth);

		if (ImGui::BeginCombo(comboID, materialLabel.c_str()))
		{
			auto& loadedMaterials = Physics::PhysicsMaterial::GetLoadedMaterials();

			for (auto& material : loadedMaterials)
			{
				const bool selected = currentPath == material.second.GetPath();

				std::string label = material.second.GetPath();
				label = Utility::GetFileWithoutExtension(Utility::GetFileNameOnly(label));
				if (ImGui::Selectable(label.c_str(), selected))
				{
					materialToReturn = &material.second;
				}

				if (selected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		if (removed != nullptr)
		{
			//ImGui::SameLine();
			//ImGui::SetCursorPosX(windowWidth - COMBOBOX_WIDTH_3);
			WidgetsUtility::IncrementCursorPosY(6);

			// Remove Model
			const std::string removeID = "##removePhysicsMat_" + std::string(comboID);
			//if (WidgetsUtility::IconButton(removeID.c_str(), ICON_FA_MINUS_SQUARE, 0.0f, .7f, ImVec4(1, 1, 1, 0.8f), ImVec4(1, 1, 1, 1), ImGui::GetStyleColorVec4(ImGuiCol_Header)))
			//	*removed = true;


			if (ImGui::IsItemHovered())
				Tooltip("Remove");

		}

		return materialToReturn;
	}

	int Lina::Editor::WidgetsUtility::SimulationTypeComboBox(const char* comboID, int currentShapeID)
	{
		int simTypeToReturn = currentShapeID;
		float currentCursor = ImGui::GetCursorPosX();
		float windowWidth = ImGui::GetWindowWidth();
		float remaining = windowWidth - currentCursor;
		float comboWidth = remaining - COMBOBOX_WIDTH_1;
		ImGui::SetNextItemWidth(comboWidth);
		if (ImGui::BeginCombo(comboID, Lina::Physics::SIMULATION_TYPES[currentShapeID].c_str()))
		{
			int counter = 0;
			for (auto& shape : Lina::Physics::SIMULATION_TYPES)
			{
				const bool selected = currentShapeID == counter;

				if (ImGui::Selectable(Lina::Physics::SIMULATION_TYPES[counter].c_str(), selected))
					simTypeToReturn = counter;

				if (selected)
					ImGui::SetItemDefaultFocus();

				counter++;
			}

			ImGui::EndCombo();
		}

		return simTypeToReturn;
	}

	int Lina::Editor::WidgetsUtility::CollisionShapeComboBox(const char* comboID, int currentShapeID)
	{
		int shapeToReturn = currentShapeID;

		float currentCursor = ImGui::GetCursorPosX();
		float windowWidth = ImGui::GetWindowWidth();
		float remaining = windowWidth - currentCursor;
		float comboWidth = remaining - COMBOBOX_WIDTH_1;
		ImGui::SetNextItemWidth(comboWidth);
		if (ImGui::BeginCombo(comboID, Lina::Physics::COLLISION_SHAPES[currentShapeID].c_str()))
		{
			int counter = 0;
			for (auto& shape : Lina::Physics::COLLISION_SHAPES)
			{
				const bool selected = currentShapeID == counter;

				if (ImGui::Selectable(Lina::Physics::COLLISION_SHAPES[counter].c_str(), selected))
					shapeToReturn = counter;

				if (selected)
					ImGui::SetItemDefaultFocus();

				counter++;
			}

			ImGui::EndCombo();
		}

		return shapeToReturn;
	}

	bool WidgetsUtility::Button(const char* label, const ImVec2& size, float textSize, float rounding)
	{
		FrameRounding(rounding);
		WidgetsUtility::PushScaledFont(textSize);
		bool button = ImGui::Button(label, size);
		ImGui::SetItemAllowOverlap();
		WidgetsUtility::PopScaledFont();
		PopStyleVar();
		return button;
	}

	bool WidgetsUtility::ToolbarToggleIcon(const char* label, const ImVec2 size, int imagePadding, bool toggled, float cursorPosY, const std::string& tooltip, ImVec4 color, float scale)
	{
		ImGui::SetCursorPosY(cursorPosY);
		ImVec2 windowPos = ImGui::GetWindowPos();
		ImVec2 min = ImVec2(windowPos.x + ImGui::GetCursorPosX(), windowPos.y + ImGui::GetCursorPosY());
		ImVec2 max = ImVec2(windowPos.x + ImGui::GetCursorPosX() + size.x, windowPos.y + ImGui::GetCursorPosY() + size.y);

		bool hovered = ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(min, max);
		bool pressed = hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left);

		ImVec4 toggledColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonLocked);
		ImVec4 hoveredColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
		ImVec4 pressedColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
		ImVec4 defaultColor = ImGui::GetStyleColorVec4(ImGuiCol_Button);
		ImVec4 col = toggled ? toggledColor : (pressed ? pressedColor : (hovered ? hoveredColor : defaultColor));
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImGui::ColorConvertFloat4ToU32(col), 4.0f);

		if (tooltip.compare("") != 0 && hovered)
		{
			Tooltip(tooltip.c_str());
		}

		const float yIncrement = size.y / 4.0f + 1;
		const ImVec2 currentCursor = ImGui::GetCursorPos();
		PushScaledFont(scale);
		ImVec2 textSize = ImGui::CalcTextSize(label);
		ImGui::SameLine();
		ImGui::SetCursorPosX(currentCursor.x + size.x / 2.0f - textSize.x / 2.0f);
		ImGui::SetCursorPosY(currentCursor.y + size.y / 2.0f - textSize.y / 2.0f);
		ImGui::Text(label);
		PopScaledFont();

		if (hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			return true;

		return false;
	}

	void Lina::Editor::WidgetsUtility::DragBehaviour(const char* id, float* var, ImRect rect)
	{

		if (ImGui::IsMouseHoveringRect(rect.Min, rect.Max))
		{
			if (!s_isDraggingWidgetInput)
			{
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
			}

			if (Lina::Input::InputEngineBackend::Get()->GetMouseButtonDown(0))
			{
				s_isDraggingWidgetInput = true;
				s_draggedInput = id;
				s_valueOnDragStart = *var;
			}
		}

		if (s_isDraggingWidgetInput)
			ImGui::SetMouseCursor(ImGuiMouseCursor_None);

		if (s_isDraggingWidgetInput && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			s_isDraggingWidgetInput = false;
			s_draggedInput = "";
		}

		if (s_isDraggingWidgetInput && id == s_draggedInput)
		{
			const float dragDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
			*var = s_valueOnDragStart + dragDelta * DRAG_POWER;
		}
	}

	void Lina::Editor::WidgetsUtility::DragBehaviour(const char* id, int* var)
	{

		if (ImGui::IsItemHovered())
		{
			if (!s_isDraggingWidgetInput)
			{
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
			}

			if (Lina::Input::InputEngineBackend::Get()->GetMouseButtonDown(0))
			{
				s_isDraggingWidgetInput = true;
				s_draggedInput = id;
				s_valueOnDragStartInt = *var;
			}

			if (s_isDraggingWidgetInput && s_draggedInput != id)
			{
				s_isDraggingWidgetInput = false;
				s_draggedInput = "";
			}
		}

		if (s_isDraggingWidgetInput)
			ImGui::SetMouseCursor(ImGuiMouseCursor_None);

		if (s_isDraggingWidgetInput && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			s_isDraggingWidgetInput = false;
			s_draggedInput = "";
		}

		if (s_isDraggingWidgetInput && id == s_draggedInput)
		{
			const int dragDelta = (int)ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
			*var = s_valueOnDragStartInt + dragDelta * DRAG_POWER;
		}
	}

	bool Lina::Editor::WidgetsUtility::DragFloat(const char* id, const char* label, float* var, float width)
	{
		bool isIcon = label == nullptr;

		if (isIcon)
			label = ICON_FA_ARROWS_ALT_H;

		if (width == -1.0f)
		{
			float windowWidth = ImGui::GetWindowWidth();
			float currentCursor = ImGui::GetCursorPosX();
			float remaining = (windowWidth - currentCursor);
			float comboWidth = remaining - VALUE_OFFSET_FROM_WINDOW;
			ImGui::SetNextItemWidth(comboWidth);
		}
		else
			ImGui::SetNextItemWidth(width);

		const float itemHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
		ImVec2 windowPos = ImGui::GetWindowPos();
		ImVec2 cursorPos = ImGui::GetCursorPos();
		ImVec2 rectMin = ImVec2(windowPos.x + cursorPos.x, windowPos.y + cursorPos.y + 1);
		ImVec2 rectMax = ImVec2(rectMin.x + itemHeight - 2, rectMin.y + itemHeight - 1);
		ImVec2 rectSize = ImVec2(rectMax.x - rectMin.x, rectMax.y - rectMin.y);
		ImVec4 rectCol = ImGui::GetStyleColorVec4(ImGuiCol_Button);
		ImVec4 textCol = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		const std::string rectID = std::string(id) + "_rect";

		if (ImGui::IsMouseHoveringRect(rectMin, rectMax))
			ImGui::SetHoveredID(ImGuiID(rectID.c_str()));

		FramePaddingX(itemHeight);
		bool result = ImGui::InputFloat(id, var);
		PopStyleVar();

		ImGui::GetWindowDrawList()->AddRectFilled(rectMin, rectMax, ImGui::ColorConvertFloat4ToU32(rectCol), 2);
		DragBehaviour(rectID.c_str(), var, ImRect(rectMin, rectMax));

		if (isIcon)
			PushScaledFont(0.6f);

		const ImVec2 textSize = ImGui::CalcTextSize(label);
		ImGui::GetWindowDrawList()->AddText(ImVec2(rectMin.x + rectSize.x / 2.0f - textSize.x / 2.0f, rectMin.y + rectSize.y / 2.0f - textSize.y / 2.0f), ImGui::ColorConvertFloat4ToU32(textCol), label);

		if (isIcon)
			PopScaledFont();


		return result;
	}

	bool Lina::Editor::WidgetsUtility::DragInt(const char* id, const char* label, int* var, int count)
	{
		if (label != nullptr)
		{
			float labelSize = ImGui::CalcTextSize(label).x + ImGui::GetStyle().ItemSpacing.x;
			IncrementCursorPosX(-labelSize);
			ImGui::Text(label);
			ImGui::SameLine();
		}
		else
		{

			IncrementCursorPosX(-21.2f);
			IncrementCursorPosY(6.2f);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.3f));
			PushScaledFont(0.7f);
			ImGui::Text(ICON_FA_ARROWS_ALT_H);
			PopScaledFont();
			ImGui::PopStyleColor();
			ImGui::SameLine();
			IncrementCursorPosX(-0.6f);
			IncrementCursorPosY(-5.8f);

		}

		DragBehaviour(id, var);

		float windowWidth = ImGui::GetWindowWidth();
		float currentCursor = ImGui::GetCursorPosX();
		float remaining = (windowWidth - currentCursor) / (float)count - 10;
		float comboWidth = remaining - 10;
		ImGui::SetNextItemWidth(comboWidth);
		return ImGui::InputInt(id, var);
	}

	bool Lina::Editor::WidgetsUtility::DragVector2(const char* id, float* var)
	{
		std::string xid = std::string(id) + "_x";
		std::string yid = std::string(id) + "_y";
		bool x = DragFloat(xid.c_str(), "X", &var[0]);
		ImGui::SameLine();
		bool y = DragFloat(yid.c_str(), "Y", &var[1]);
		return x || y;
	}

	bool Lina::Editor::WidgetsUtility::DragVector3(const char* id, float* var)
	{
		float windowWidth = ImGui::GetWindowWidth();
		float currentCursor = ImGui::GetCursorPosX();
		const float labelIncrement = 12;
		float widthPerItem = (windowWidth - currentCursor - VALUE_OFFSET_FROM_WINDOW - ImGui::GetStyle().ItemSpacing.x * 2.0f) / 3.0f;
		std::string xid = std::string(id) + "_x";
		std::string yid = std::string(id) + "_y";
		std::string zid = std::string(id) + "_z";

		bool x = DragFloat(xid.c_str(), "X", &var[0], widthPerItem);
		ImGui::SameLine();
		bool y = DragFloat(yid.c_str(), "Y", &var[1], widthPerItem);
		ImGui::SameLine();
		bool z = DragFloat(zid.c_str(), "Z", &var[2], widthPerItem);

		return false;
	}

	bool Lina::Editor::WidgetsUtility::DragVector4(const char* id, float* var)
	{
		std::string xid = std::string(id) + "_x";
		std::string yid = std::string(id) + "_y";
		std::string zid = std::string(id) + "_z";
		std::string wid = std::string(id) + "_w";
		bool x = DragFloat(xid.c_str(), "X", &var[0]);
		ImGui::SameLine();
		bool y = DragFloat(yid.c_str(), "Y", &var[1]);
		ImGui::SameLine();
		bool z = DragFloat(zid.c_str(), "Z", &var[2]);
		ImGui::SameLine();
		bool w = DragFloat(zid.c_str(), "W", &var[3]);
		return x || y || z || w;
	}

	void WidgetsUtility::IncrementCursorPosX(float f)
	{
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + f);
	}

	void WidgetsUtility::IncrementCursorPosY(float f)
	{
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + f);
	}

	void WidgetsUtility::IncrementCursorPos(const  ImVec2& v)
	{
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + v.x, ImGui::GetCursorPosY() + v.y));
	}

	void WidgetsUtility::CenteredText(const char* label)
	{
		float textW = ImGui::CalcTextSize(label).x;
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - textW / 2.0f);
		ImGui::Text(label);
	}

	void WidgetsUtility::CenterCursorX()
	{
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f);
	}

	void WidgetsUtility::CenterCursorY()
	{
		ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 2.0f);

	}

	void WidgetsUtility::CenterCursor()
	{
		CenterCursorX();
		CenterCursorY();
	}

	void WidgetsUtility::PushScaledFont(float defaultScale)
	{
		ImGui::GetFont()->Scale = defaultScale;
		ImGui::PushFont(ImGui::GetFont());
	}

	void WidgetsUtility::PopScaledFont()
	{
		ImGui::GetFont()->Scale = 1.0f;
		ImGui::PopFont();
	}

	void WidgetsUtility::FramePaddingX(float amt)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(amt, ImGui::GetStyle().FramePadding.y));
	}

	void WidgetsUtility::FramePaddingY(float amt)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, amt));
	}

	void WidgetsUtility::FramePadding(const ImVec2& amt)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, amt);
	}

	void WidgetsUtility::FrameRounding(float rounding)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, rounding);
	}

	void WidgetsUtility::WindowPaddingX(float amt)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(amt, ImGui::GetStyle().WindowPadding.y));
	}

	void WidgetsUtility::WindowPaddingY(float amt)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(ImGui::GetStyle().WindowPadding.x, amt));
	}

	void WidgetsUtility::WindowPadding(const ImVec2& amt)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, amt);
	}

	void WidgetsUtility::ItemSpacingX(float amt)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(amt, ImGui::GetStyle().ItemSpacing.y));
	}

	void WidgetsUtility::ItemSpacingY(float amt)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, amt));
	}

	void WidgetsUtility::ItemSpacing(const ImVec2& amt)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, amt);
	}

	void WidgetsUtility::WindowRounding(float rounding)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, rounding);
	}

	void WidgetsUtility::PopStyleVar()
	{
		ImGui::PopStyleVar();
	}

	void WidgetsUtility::Icon(const char* label, bool align, float scale)
	{
		PushScaledFont(scale);

		if (align)
		{
			ImVec2 cursorPos = ImGui::GetCursorPos();
			ImVec2 textSize = ImGui::CalcTextSize(label);
			ImGui::SetCursorPos(ImVec2(cursorPos.x - textSize.x / 2.0f, cursorPos.y - textSize.x / 2.0f));
		}
		ImGui::Text(label);
		PopScaledFont();
	}

	bool WidgetsUtility::IconButton(const char* id, const char* label, bool align, float scale, bool disabled)
	{

		if (!disabled)
		{
			Icon(label, align, scale);
			bool pressed = ImGui::IsItemClicked();
			bool hovered = ImGui::IsItemHovered();
			bool beingPressed = hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left);
			bool released = hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left);
			return released;
		}
		else
		{
			ImVec4 disabledColor = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
			ImGui::PushStyleColor(ImGuiCol_Text, disabledColor);
			Icon(label, align, scale);
			ImGui::PopStyleColor();
			return false;
		}


	}

	ImVec2 WidgetsUtility::GetWindowPosWithContentRegion()
	{
		return ImVec2(ImGui::GetWindowContentRegionMin().x + ImGui::GetWindowPos().x, ImGui::GetWindowContentRegionMin().y + ImGui::GetWindowPos().y);
	}

	ImVec2 WidgetsUtility::GetWindowSizeWithContentRegion()
	{
		float yDiff = ImGui::GetWindowPos().y - WidgetsUtility::GetWindowPosWithContentRegion().y;
		return ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetWindowSize().y + yDiff);
	}


}