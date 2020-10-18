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

#include "Drawers/ComponentDrawer.hpp"
#include "ECS/ECSComponent.hpp"
#include "ECS/Components/TransformComponent.hpp"
#include "ECS/Components/CameraComponent.hpp"
#include "ECS/Components/LightComponent.hpp"
#include "ECS/Components/FreeLookComponent.hpp"
#include "ECS/Components/MeshRendererComponent.hpp"
#include "ECS/Components/SpriteRendererComponent.hpp"
#include "ECS/Components/RigidbodyComponent.hpp"
#include "Widgets/WidgetsUtility.hpp"
#include "IconsFontAwesome5.h"
#include "IconsMaterialDesign.h"

using namespace LinaEngine::ECS;
using namespace LinaEditor;

namespace LinaEditor
{
	ComponentDrawer* ComponentDrawer::s_activeInstance = nullptr;

	void ComponentDrawer::RegisterComponentFunctions()
	{
		// Display names.
		std::get<0>(m_componentDrawFuncMap[GetTypeID<TransformComponent>()]) = "Transformation";
		std::get<0>(m_componentDrawFuncMap[GetTypeID<RigidbodyComponent>()]) = "Rigidbody";
		std::get<0>(m_componentDrawFuncMap[GetTypeID<CameraComponent>()]) = "Camera";
		std::get<0>(m_componentDrawFuncMap[GetTypeID<DirectionalLightComponent>()]) = "Directional Light";
		std::get<0>(m_componentDrawFuncMap[GetTypeID<SpotLightComponent>()]) = "Spot Light";
		std::get<0>(m_componentDrawFuncMap[GetTypeID<PointLightComponent>()]) = "Point Light";
		std::get<0>(m_componentDrawFuncMap[GetTypeID<FreeLookComponent>()]) = "Free Look";
		std::get<0>(m_componentDrawFuncMap[GetTypeID<MeshRendererComponent>()]) = "Mesh Renderer";
		std::get<0>(m_componentDrawFuncMap[GetTypeID<SpriteRendererComponent>()]) = "Sprite Renderer";

		// Add functions.
		std::get<1>(m_componentDrawFuncMap[GetTypeID<TransformComponent>()]) = std::bind(&TransformComponent::COMPONENT_ADDFUNC, std::placeholders::_1, std::placeholders::_2);
		std::get<1>(m_componentDrawFuncMap[GetTypeID<RigidbodyComponent>()]) = std::bind(&RigidbodyComponent::COMPONENT_ADDFUNC, std::placeholders::_1, std::placeholders::_2);
		std::get<1>(m_componentDrawFuncMap[GetTypeID<CameraComponent>()]) = std::bind(&CameraComponent::COMPONENT_ADDFUNC, std::placeholders::_1, std::placeholders::_2);
		std::get<1>(m_componentDrawFuncMap[GetTypeID<DirectionalLightComponent>()]) = std::bind(&DirectionalLightComponent::COMPONENT_ADDFUNC, std::placeholders::_1, std::placeholders::_2);
		std::get<1>(m_componentDrawFuncMap[GetTypeID<SpotLightComponent>()]) = std::bind(&SpotLightComponent::COMPONENT_ADDFUNC, std::placeholders::_1, std::placeholders::_2);
		std::get<1>(m_componentDrawFuncMap[GetTypeID<PointLightComponent>()]) = std::bind(&PointLightComponent::COMPONENT_ADDFUNC, std::placeholders::_1, std::placeholders::_2);
		std::get<1>(m_componentDrawFuncMap[GetTypeID<FreeLookComponent>()]) = std::bind(&FreeLookComponent::COMPONENT_ADDFUNC, std::placeholders::_1, std::placeholders::_2);
		std::get<1>(m_componentDrawFuncMap[GetTypeID<MeshRendererComponent>()]) = std::bind(&MeshRendererComponent::COMPONENT_ADDFUNC, std::placeholders::_1, std::placeholders::_2);
		std::get<1>(m_componentDrawFuncMap[GetTypeID<SpriteRendererComponent>()]) = std::bind(&SpriteRendererComponent::COMPONENT_ADDFUNC, std::placeholders::_1, std::placeholders::_2);

		// Draw functions.
		std::get<2>(m_componentDrawFuncMap[GetTypeID<TransformComponent>()]) = std::bind(&TransformComponent::COMPONENT_DRAWFUNC, std::placeholders::_1, std::placeholders::_2);
		std::get<2>(m_componentDrawFuncMap[GetTypeID<RigidbodyComponent>()]) = std::bind(&RigidbodyComponent::COMPONENT_DRAWFUNC, std::placeholders::_1, std::placeholders::_2);
		std::get<2>(m_componentDrawFuncMap[GetTypeID<CameraComponent>()]) = std::bind(&CameraComponent::COMPONENT_DRAWFUNC, std::placeholders::_1, std::placeholders::_2);
		std::get<2>(m_componentDrawFuncMap[GetTypeID<DirectionalLightComponent>()]) = std::bind(&DirectionalLightComponent::COMPONENT_DRAWFUNC, std::placeholders::_1, std::placeholders::_2);
		std::get<2>(m_componentDrawFuncMap[GetTypeID<SpotLightComponent>()]) = std::bind(&SpotLightComponent::COMPONENT_DRAWFUNC, std::placeholders::_1, std::placeholders::_2);
		std::get<2>(m_componentDrawFuncMap[GetTypeID<PointLightComponent>()]) = std::bind(&PointLightComponent::COMPONENT_DRAWFUNC, std::placeholders::_1, std::placeholders::_2);
		std::get<2>(m_componentDrawFuncMap[GetTypeID<FreeLookComponent>()]) = std::bind(&FreeLookComponent::COMPONENT_DRAWFUNC, std::placeholders::_1, std::placeholders::_2);
		std::get<2>(m_componentDrawFuncMap[GetTypeID<MeshRendererComponent>()]) = std::bind(&MeshRendererComponent::COMPONENT_DRAWFUNC, std::placeholders::_1, std::placeholders::_2);
		std::get<2>(m_componentDrawFuncMap[GetTypeID<SpriteRendererComponent>()]) = std::bind(&SpriteRendererComponent::COMPONENT_DRAWFUNC, std::placeholders::_1, std::placeholders::_2);
	}

	// Use reflection for gods sake later on.
	std::vector<std::string> ComponentDrawer::GetEligibleComponents(LinaEngine::ECS::ECSRegistry* ecs, LinaEngine::ECS::ECSEntity entity)
	{
		std::vector<std::string> eligibleTypes;
		std::vector<ECSTypeID> typeIDs;

		// Store all components of the entity.
		ecs->visit(entity, [&typeIDs](const auto component)
			{
				ECSTypeID id = component;
				typeIDs.push_back(id);
			});

		// Iterate registered types & add as eligible if entity does not contain the type.
		for (std::map<ECSTypeID, ComponentValueTuple>::iterator it = m_componentDrawFuncMap.begin(); it != m_componentDrawFuncMap.end(); ++it)
		{
			if (std::find(typeIDs.begin(), typeIDs.end(), it->first) == typeIDs.end())
				eligibleTypes.push_back(std::get<0>(it->second));
		}

		return eligibleTypes;
	}

	void ComponentDrawer::AddComponentToEntity(ECSRegistry* ecs, ECSEntity entity, const std::string& comp)
	{
		// Call the add function of the type when the requested strings match.
		for (std::map<ECSTypeID, ComponentValueTuple>::iterator it = m_componentDrawFuncMap.begin(); it != m_componentDrawFuncMap.end(); ++it)
		{
			if (std::get<0>(it->second).compare(comp) == 0)
				std::get<1>(it->second)(ecs, entity);
		}
	}

	void ComponentDrawer::SwapComponentOrder(LinaEngine::ECS::ECSTypeID id1, LinaEngine::ECS::ECSTypeID id2)
	{
		// Swap iterators.
		std::vector<ECSTypeID>::iterator it1 = std::find(m_componentDrawList.begin(), m_componentDrawList.end(), id1);
		std::vector<ECSTypeID>::iterator it2 = std::find(m_componentDrawList.begin(), m_componentDrawList.end(), id2);
		std::iter_swap(it1, it2);
	}

	void ComponentDrawer::AddIDToDrawList(LinaEngine::ECS::ECSTypeID id)
	{
		// Add only if it doesn't exists.
		if (std::find(m_componentDrawList.begin(), m_componentDrawList.end(), id) == m_componentDrawList.end())
			m_componentDrawList.push_back(id);
	}

	void ComponentDrawer::ClearDrawList()
	{
		m_componentDrawList.clear();
	}

	void ComponentDrawer::DrawComponents(LinaEngine::ECS::ECSRegistry* ecs, LinaEngine::ECS::ECSEntity entity)
	{
		s_activeInstance = this;

		// Draw components.
		for (int i = 0; i < m_componentDrawList.size(); i++)
			std::get<2>(m_componentDrawFuncMap[m_componentDrawList[i]])(ecs, entity);
	}

	bool ComponentDrawer::DrawComponentTitle(LinaEngine::ECS::ECSTypeID typeID, const char* title, const char* icon, bool* refreshPressed, bool* enabled, bool* foldoutOpen, const ImVec4& iconColor, const ImVec2& iconOffset)
	{
		// Caret button.
		const char* caret = *foldoutOpen ? ICON_FA_CARET_DOWN : ICON_FA_CARET_RIGHT;
		if (WidgetsUtility::IconButtonNoDecoration(caret, 30, 0.8f))
			*foldoutOpen = !*foldoutOpen;

		// Title.
		ImGui::SameLine();
		ImGui::AlignTextToFramePadding();
		WidgetsUtility::IncrementCursorPosY(-5);
		ImGui::Text(title);
		ImGui::AlignTextToFramePadding();
		ImGui::SameLine();


		// Title is the drag and drop target.
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			// Set payload to carry the type id.
			ImGui::SetDragDropPayload("COMP_MOVE_PAYLOAD", &typeID, sizeof(int));

			// Display preview 
			ImGui::Text("Move ");
			ImGui::EndDragDropSource();
		}

		// Dropped on another title, swap component orders.
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("COMP_MOVE_PAYLOAD"))
			{
				IM_ASSERT(payload->DataSize == sizeof(LinaEngine::ECS::ECSTypeID));
				LinaEngine::ECS::ECSTypeID payloadID = *(const LinaEngine::ECS::ECSTypeID*)payload->Data;
				SwapComponentOrder(payloadID, typeID);
			}
			ImGui::EndDragDropTarget();
		}

		// Icon
		WidgetsUtility::IncrementCursorPosY(6);
		WidgetsUtility::IncrementCursorPos(ImVec2(iconOffset.x, iconOffset.y));
		WidgetsUtility::Icon(icon, 0.6f, iconColor);
		WidgetsUtility::IncrementCursorPos(ImVec2(-iconOffset.x, -iconOffset.y));

		// Enabled toggle
		std::string buf(title);
		buf.append("t");
		ImVec4 toggleColor = ImGui::GetStyleColorVec4(ImGuiCol_Header);
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 88);
		WidgetsUtility::IncrementCursorPosY(-4);
		WidgetsUtility::ToggleButton(buf.c_str(), enabled, 0.8f, 1.4f, toggleColor, ImVec4(toggleColor.x, toggleColor.y, toggleColor.z, 0.7f));

		// Refresh button
		buf.append("r");
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 43);
		WidgetsUtility::IncrementCursorPosY(4);
		*refreshPressed = WidgetsUtility::IconButton(buf.c_str(), ICON_FA_SYNC_ALT, 0.0f, 0.6f, ImVec4(1, 1, 1, 0.8f), ImVec4(1, 1, 1, 1), ImGui::GetStyleColorVec4(ImGuiCol_Header));

		// Close button
		buf.append("c");
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 20);
		return WidgetsUtility::IconButton(buf.c_str(), ICON_FA_TIMES, 0.0f, 0.6f, ImVec4(1, 1, 1, 0.8f), ImVec4(1, 1, 1, 1), ImGui::GetStyleColorVec4(ImGuiCol_Header));
	}
}

#define CURSORPOS_X_LABELS 12
#define CURSORPOS_Y_INCREMENT_BEFORE 15
#define CURSORPOS_Y_INCREMENT_BEFOREVAL 2.5f
#define CURSORPOS_Y_INCREMENT_AFTER 6.5f
#define CURSORPOS_XPERC_VALUES 0.32f
#define CURSORPOS_XPERC_VALUES2 0.545f

const char* rigidbodyShapes[]
{
	"SPHERE",
	"BOX",
	"CYLINDER",
	"CAPSULE"
};


void LinaEngine::ECS::TransformComponent::COMPONENT_DRAWFUNC(LinaEngine::ECS::ECSRegistry* ecs, LinaEngine::ECS::ECSEntity entity)
{
	// Get component
	TransformComponent& transform = ecs->get<TransformComponent>(entity);

	// Align.
	WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_BEFORE);
	WidgetsUtility::IncrementCursorPosX(CURSORPOS_X_LABELS);

	// Draw title.
	static bool open = false;
	bool refreshPressed = false;
	bool removeComponent = ComponentDrawer::s_activeInstance->DrawComponentTitle(GetTypeID<TransformComponent>(), "Transformation", ICON_FA_ARROWS_ALT, &refreshPressed, &transform.m_isEnabled, &open, ImGui::GetStyleColorVec4(ImGuiCol_Header));

	// Remove if requested.
	if (removeComponent)
	{
		ecs->remove<TransformComponent>(entity);
		return;
	}

	// Refresh
	if (refreshPressed)
		ecs->replace<TransformComponent>(entity, TransformComponent());

	// Draw component
	if (open)
	{
		WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_BEFOREVAL);
		float cursorPosValues = ImGui::GetWindowSize().x * CURSORPOS_XPERC_VALUES;
		float cursorPosLabels = CURSORPOS_X_LABELS;
		ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Location"); ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues); ImGui::DragFloat3("##loc", &transform.transform.m_location.x);
		ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Rotation"); ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues); WidgetsUtility::DragQuaternion("##rot", transform.transform.m_rotation);
		ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Scale");	 ImGui::SameLine();	ImGui::SetCursorPosX(cursorPosValues); ImGui::DragFloat3("##scale", &transform.transform.m_scale.x);
		WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_AFTER);
	}

	// Draw bevel line
	WidgetsUtility::DrawBeveledLine();
}

void LinaEngine::ECS::RigidbodyComponent::COMPONENT_DRAWFUNC(LinaEngine::ECS::ECSRegistry* ecs, LinaEngine::ECS::ECSEntity entity)
{
	// Get component
	RigidbodyComponent& rb = ecs->get<RigidbodyComponent>(entity);

	// Align.
	WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_BEFORE);
	WidgetsUtility::IncrementCursorPosX(CURSORPOS_X_LABELS);

	// Draw title.
	static bool open = false;
	bool refreshPressed = false;
	bool removeComponent = ComponentDrawer::s_activeInstance->DrawComponentTitle(GetTypeID<RigidbodyComponent>(), "Rigidbody", ICON_MD_ACCESSIBILITY, &refreshPressed, &rb.m_isEnabled, &open, ImGui::GetStyleColorVec4(ImGuiCol_Header), ImVec2(0, 3));

	// Remove if requested.
	if (removeComponent)
	{
		ecs->remove<RigidbodyComponent>(entity);
		return;
	}

	// Refresh
	if (refreshPressed)
		ecs->replace<RigidbodyComponent>(entity, RigidbodyComponent());

	// Draw component.
	if (open)
	{
		float cursorPosValues = ImGui::GetWindowSize().x * CURSORPOS_XPERC_VALUES;
		float cursorPosLabels = CURSORPOS_X_LABELS;
		WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_BEFOREVAL);

		ComponentDrawer::s_activeInstance->m_currentCollisionShape = (int)rb.m_collisionShape;

		// Draw collision shape.
		static ImGuiComboFlags flags = 0;
		static ECS::CollisionShape selectedCollisionShape = rb.m_collisionShape;
		const char* collisionShapeLabel = rigidbodyShapes[ComponentDrawer::s_activeInstance->m_currentCollisionShape];

		ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Shape"); ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues);
		if (ImGui::BeginCombo("##collshape", collisionShapeLabel, flags))
		{
			for (int i = 0; i < IM_ARRAYSIZE(rigidbodyShapes); i++)
			{
				const bool is_selected = (ComponentDrawer::s_activeInstance->m_currentCollisionShape == i);
				if (ImGui::Selectable(rigidbodyShapes[i], is_selected))
				{
					selectedCollisionShape = (ECS::CollisionShape)i;
					ComponentDrawer::s_activeInstance->m_currentCollisionShape = i;
					rb.m_collisionShape = selectedCollisionShape;
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();

		}

		ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Mass"); ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues);  ImGui::DragFloat("##mass", &rb.m_mass);

		if (rb.m_collisionShape == ECS::CollisionShape::BOX || rb.m_collisionShape == ECS::CollisionShape::CYLINDER)
		{
			ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Half Extents"); ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues); ImGui::DragFloat3("##halfextents", &rb.m_halfExtents.x);
		}
		else if (rb.m_collisionShape == ECS::CollisionShape::SPHERE)
		{
			ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Radius"); ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues); ImGui::DragFloat("##radius", &rb.m_radius);
		}
		else if (rb.m_collisionShape == ECS::CollisionShape::CAPSULE)
		{
			ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Radius"); ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues); ImGui::DragFloat("##radius", &rb.m_radius);
			ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Height"); ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues); ImGui::DragFloat("##height", &rb.m_capsuleHeight);
		}

		ImGui::SetCursorPosX(cursorPosLabels);
		if (ImGui::Button("Apply"))
			ecs->replace<LinaEngine::ECS::RigidbodyComponent>(entity, rb);

		WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_AFTER);
	}

	// Draw bevel line
	WidgetsUtility::DrawBeveledLine();
}

void LinaEngine::ECS::CameraComponent::COMPONENT_DRAWFUNC(LinaEngine::ECS::ECSRegistry* ecs, LinaEngine::ECS::ECSEntity entity)
{
	// Get component
	CameraComponent& camera = ecs->get<CameraComponent>(entity);

	// Align.
	WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_BEFORE);
	WidgetsUtility::IncrementCursorPosX(CURSORPOS_X_LABELS);

	// Draw title.
	static bool open = false;
	bool refreshPressed = false;
	bool removeComponent = ComponentDrawer::s_activeInstance->DrawComponentTitle(GetTypeID<CameraComponent>(), "Camera", ICON_FA_VIDEO, &refreshPressed, &camera.m_isEnabled, &open, ImGui::GetStyleColorVec4(ImGuiCol_Header));

	// Remove if requested.
	if (removeComponent)
	{
		ecs->remove<CameraComponent>(entity);
		return;
	}

	// Refresh
	if (refreshPressed)
		ecs->replace<CameraComponent>(entity, CameraComponent());

	// Draw component.
	if (open)
	{
		WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_BEFOREVAL);
		float cursorPosValues = ImGui::GetWindowSize().x * CURSORPOS_XPERC_VALUES;
		float cursorPosLabels = CURSORPOS_X_LABELS;
		ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Clear Color");	 ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues); WidgetsUtility::ColorButton("##clrclr", &camera.m_clearColor.r);
		ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Field of View"); ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues); ImGui::DragFloat("##fov", &camera.m_fieldOfView);
		ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Near Plane");	 ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues); ImGui::DragFloat("##zNear", &camera.m_zNear);
		ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Far Plane");	 ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues); ImGui::DragFloat("##zFar", &camera.m_zFar);
		WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_AFTER);
	}

	// Draw bevel line.
	WidgetsUtility::DrawBeveledLine();
}

void LinaEngine::ECS::DirectionalLightComponent::COMPONENT_DRAWFUNC(LinaEngine::ECS::ECSRegistry* ecs, LinaEngine::ECS::ECSEntity entity)
{
	// Get component
	DirectionalLightComponent& dLight = ecs->get<DirectionalLightComponent>(entity);

	// Align.
	WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_BEFORE);
	WidgetsUtility::IncrementCursorPosX(CURSORPOS_X_LABELS);

	// Draw title.
	static bool open = false;
	bool refreshPressed = false;
	bool removeComponent = ComponentDrawer::s_activeInstance->DrawComponentTitle(GetTypeID<DirectionalLightComponent>(), "DirectionalLight", ICON_FA_SUN, &refreshPressed, &dLight.m_isEnabled, &open, ImGui::GetStyleColorVec4(ImGuiCol_Header));

	// Remove if requested.
	if (removeComponent)
	{
		ecs->remove<DirectionalLightComponent>(entity);
		return;
	}

	// Refresh
	if (refreshPressed)
		ecs->replace<DirectionalLightComponent>(entity, DirectionalLightComponent());

	// Draw component.
	if (open)
	{
		WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_BEFOREVAL);
		float cursorPosValues = ImGui::GetWindowSize().x * CURSORPOS_XPERC_VALUES;
		float cursorPosLabels = CURSORPOS_X_LABELS;
		ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Color");				ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues); WidgetsUtility::ColorButton("##dclr", &dLight.m_color.r);
		ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Shadow Near Plane");	ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues); ImGui::DragFloat("##szNear", &dLight.m_shadowZNear);
		ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Shadow Far Plane");		ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues); ImGui::DragFloat("##szFar", &dLight.m_shadowZFar);
		ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Shadow Projection");	ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues); ImGui::DragFloat4("##sproj", &dLight.m_shadowOrthoProjection.x);
		WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_AFTER);
	}

	// Draw bevel line.
	WidgetsUtility::DrawBeveledLine();
}

void LinaEngine::ECS::PointLightComponent::COMPONENT_DRAWFUNC(LinaEngine::ECS::ECSRegistry* ecs, LinaEngine::ECS::ECSEntity entity)
{
	// Get component
	PointLightComponent& pLight = ecs->get<PointLightComponent>(entity);

	// Align.
	WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_BEFORE);
	WidgetsUtility::IncrementCursorPosX(CURSORPOS_X_LABELS);

	// Draw title.
	static bool open = false;
	bool refreshPressed = false;
	bool removeComponent = ComponentDrawer::s_activeInstance->DrawComponentTitle(GetTypeID<PointLightComponent>(), "PointLight", ICON_FA_LIGHTBULB, &refreshPressed, &pLight.m_isEnabled, &open, ImGui::GetStyleColorVec4(ImGuiCol_Header));

	// Remove if requested.
	if (removeComponent)
	{
		ecs->remove<PointLightComponent>(entity);
		return;
	}

	// Refresh
	if (refreshPressed)
		ecs->replace<PointLightComponent>(entity, PointLightComponent());

	// Draw component.
	if (open)
	{
		WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_BEFOREVAL);
		float cursorPosValues = ImGui::GetWindowSize().x * CURSORPOS_XPERC_VALUES;
		float cursorPosLabels = CURSORPOS_X_LABELS;
		ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Color");		ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues); WidgetsUtility::ColorButton("##pclr", &pLight.m_color.r);
		ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Distance");	ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues); ImGui::DragFloat("##pldist", &pLight.m_distance);
		WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_AFTER);
	}

	// Draw bevel line.
	WidgetsUtility::DrawBeveledLine();
}

void LinaEngine::ECS::SpotLightComponent::COMPONENT_DRAWFUNC(LinaEngine::ECS::ECSRegistry* ecs, LinaEngine::ECS::ECSEntity entity)
{
	// Get component
	SpotLightComponent& sLight = ecs->get<SpotLightComponent>(entity);

	// Align.
	WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_BEFORE);
	WidgetsUtility::IncrementCursorPosX(CURSORPOS_X_LABELS);

	// Draw title.
	static bool open = false;
	bool refreshPressed = false;
	bool removeComponent = ComponentDrawer::s_activeInstance->DrawComponentTitle(GetTypeID<SpotLightComponent>(), "SpotLight", ICON_MD_HIGHLIGHT, &refreshPressed, &sLight.m_isEnabled, &open, ImGui::GetStyleColorVec4(ImGuiCol_Header), ImVec2(0, 3.0f));

	// Remove if requested.
	if (removeComponent)
	{
		ecs->remove<SpotLightComponent>(entity);
		return;
	}

	// Refresh
	if (refreshPressed)
		ecs->replace<SpotLightComponent>(entity, SpotLightComponent());

	// Draw component.
	if (open)
	{
		WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_BEFOREVAL);
		float cursorPosValues = ImGui::GetWindowSize().x * CURSORPOS_XPERC_VALUES;
		float cursorPosLabels = CURSORPOS_X_LABELS;
		ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Color");		ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues); WidgetsUtility::ColorButton("##sclr", &sLight.m_color.r);
		ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Distance");		ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues); ImGui::DragFloat("##sldist", &sLight.m_distance);
		ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Cutoff");		ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues); ImGui::DragFloat("##cutOff", &sLight.m_cutoff);
		ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Outer Cutoff");	ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues); ImGui::DragFloat("##outerCutOff", &sLight.m_outerCutoff);
		WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_AFTER);
	}

	// Draw bevel line.
	WidgetsUtility::DrawBeveledLine();
}

void LinaEngine::ECS::FreeLookComponent::COMPONENT_DRAWFUNC(LinaEngine::ECS::ECSRegistry* ecs, LinaEngine::ECS::ECSEntity entity)
{
	// Get component
	FreeLookComponent& freeLook = ecs->get<FreeLookComponent>(entity);

	// Align.
	WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_BEFORE);
	WidgetsUtility::IncrementCursorPosX(CURSORPOS_X_LABELS);

	// Draw title.
	static bool open = false;
	bool refreshPressed = false;
	bool removeComponent = ComponentDrawer::s_activeInstance->DrawComponentTitle(GetTypeID<FreeLookComponent>(), "FreeLook", ICON_MD_3D_ROTATION, &refreshPressed, &freeLook.m_isEnabled, &open, ImGui::GetStyleColorVec4(ImGuiCol_Header), ImVec2(0, 3));

	// Remove if requested.
	if (removeComponent)
	{
		ecs->remove<FreeLookComponent>(entity);
		return;
	}

	// Refresh
	if (refreshPressed)
		ecs->replace<FreeLookComponent>(entity, FreeLookComponent());

	// Draw component.
	if (open)
	{
		WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_BEFOREVAL);
		float cursorPosValues = ImGui::GetWindowSize().x * CURSORPOS_XPERC_VALUES;
		float cursorPosLabels = CURSORPOS_X_LABELS;
		ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Movement Speeds");	ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues); ImGui::DragFloat2("##ms", &freeLook.m_movementSpeeds.x);
		ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Rotation Speeds");	ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues); ImGui::DragFloat2("##rs", &freeLook.m_rotationSpeeds.x);
		WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_AFTER);
	}

	// Draw bevel line.
	WidgetsUtility::DrawBeveledLine();
}

void LinaEngine::ECS::MeshRendererComponent::COMPONENT_DRAWFUNC(LinaEngine::ECS::ECSRegistry* ecs, LinaEngine::ECS::ECSEntity entity)
{
	// Get component
	MeshRendererComponent& renderer = ecs->get<MeshRendererComponent>(entity);

	// Align.
	WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_BEFORE);
	WidgetsUtility::IncrementCursorPosX(CURSORPOS_X_LABELS);

	// Draw title.
	static bool open = false;
	bool refreshPressed = false;
	bool removeComponent = ComponentDrawer::s_activeInstance->DrawComponentTitle(GetTypeID<MeshRendererComponent>(), "MeshRenderer", ICON_MD_GRID_ON, &refreshPressed, &renderer.m_isEnabled, &open, ImGui::GetStyleColorVec4(ImGuiCol_Header), ImVec2(0, 3));

	// Remove if requested.
	if (removeComponent)
	{
		ecs->remove<MeshRendererComponent>(entity);
		return;
	}

	// Refresh
	if (refreshPressed)
		ecs->replace<MeshRendererComponent>(entity, MeshRendererComponent());

	// Draw component.
	if (open)
	{
		WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_BEFOREVAL);
		float cursorPosValues = ImGui::GetWindowSize().x * CURSORPOS_XPERC_VALUES;
		float cursorPosLabels = CURSORPOS_X_LABELS;
		//ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Movement Speeds");	ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues); ImGui::DragFloat2("##ms", &renderer.m_movementSpeeds.x);
		//ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Rotation Speeds");	ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues); ImGui::DragFloat2("##rs", &renderer.m_rotationSpeeds.x);
		WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_AFTER);
	}

	// Draw bevel line.
	WidgetsUtility::DrawBeveledLine();
}

void LinaEngine::ECS::SpriteRendererComponent::COMPONENT_DRAWFUNC(LinaEngine::ECS::ECSRegistry* ecs, LinaEngine::ECS::ECSEntity entity)
{
	// Get component
	SpriteRendererComponent& renderer = ecs->get<SpriteRendererComponent>(entity);

	// Align.
	WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_BEFORE);
	WidgetsUtility::IncrementCursorPosX(CURSORPOS_X_LABELS);

	// Draw title.
	static bool open = false;
	bool refreshPressed = false;
	bool removeComponent = ComponentDrawer::s_activeInstance->DrawComponentTitle(GetTypeID<SpriteRendererComponent>(), "SpriteRenderer", ICON_FA_LIGHTBULB, &refreshPressed, &renderer.m_isEnabled, &open, ImGui::GetStyleColorVec4(ImGuiCol_Header));

	// Remove if requested.
	if (removeComponent)
	{
		ecs->remove<SpriteRendererComponent>(entity);
		return;
	}

	// Refresh
	if (refreshPressed)
		ecs->replace<SpriteRendererComponent>(entity, SpriteRendererComponent());

	// Draw component.
	if (open)
	{
		WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_BEFOREVAL);
		float cursorPosValues = ImGui::GetWindowSize().x * CURSORPOS_XPERC_VALUES;
		float cursorPosLabels = CURSORPOS_X_LABELS;
		//ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Movement Speeds");	ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues); ImGui::DragFloat2("##ms", &renderer.m_movementSpeeds.x);
		//ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Rotation Speeds");	ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosValues); ImGui::DragFloat2("##rs", &renderer.m_rotationSpeeds.x);
		WidgetsUtility::IncrementCursorPosY(CURSORPOS_Y_INCREMENT_AFTER);
	}

	// Draw bevel line.
	WidgetsUtility::DrawBeveledLine();
}