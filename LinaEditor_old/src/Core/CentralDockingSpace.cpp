#include "Core/CentralDockingSpace.hpp"
#include "Core/GUILayer.hpp"
#include "Core/Window.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

namespace Lina::Editor
{
    ImGuiID dockspaceID    = 0;
    bool    dockspaceBuilt = false;

    void CentralDockingSpace::BuildDockspace()
    {
        dockspaceBuilt     = true;
        Vector2 screenSize = Graphics::Window::Get()->GetSize();
        ImGui::DockBuilderRemoveNode(dockspaceID); // Clear out existing layout

        const ImGuiDockNodeFlags dockNodeFlags = ImGuiDockNodeFlags_DockSpace;
        ImGui::DockBuilderAddNode(dockspaceID, dockNodeFlags); // Add empty node
        ImGui::DockBuilderSetNodeSize(dockspaceID, ImVec2(screenSize.x, screenSize.y - FOOTER_HEIGHT));

        ImGuiID dock_main_id        = dockspaceID; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.
        ImGuiID dock_id_prop        = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.15f, NULL, &dock_main_id);
        ImGuiID dock_id_propBottom  = ImGui::DockBuilderSplitNode(dock_id_prop, ImGuiDir_Down, 0.6f, NULL, &dock_id_prop);
        ImGuiID dock_id_right       = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f, NULL, &dock_main_id);
        ImGuiID dock_id_rightBottom = ImGui::DockBuilderSplitNode(dock_id_right, ImGuiDir_Down, 0.20f, NULL, &dock_id_right);
        ImGuiID dock_id_bottom      = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.45f, NULL, &dock_main_id);

        ImGui::DockBuilderDockWindow(ID_RESOURCES, dock_id_bottom);
        ImGui::DockBuilderDockWindow(ID_ENTITIES, dock_id_prop);
        ImGui::DockBuilderDockWindow(ID_SYSTEMS, dock_id_propBottom);
        ImGui::DockBuilderDockWindow(ID_SCENE, dock_main_id);
        ImGui::DockBuilderDockWindow(ID_LOG, dock_id_rightBottom);
        ImGui::DockBuilderDockWindow(ID_GLOBAL, dock_id_rightBottom);
        ImGui::DockBuilderDockWindow(ID_PROPERTIES, dock_id_right);
        ImGui::DockBuilderFinish(dockspaceID);
    }

    void CentralDockingSpace::Draw()
    {

        dockspaceID                              = ImGui::GetID("Lina Dockspace");
        const ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
        ImGuiWindowFlags         windowFlags     = 0;
        windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
        windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        const ImGuiViewport* viewport      = ImGui::GetMainViewport();
        const ImVec2         dockspaceSize = ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - GUILayer::Get()->m_headerSize - GUILayer::Get()->m_footerSize);
        const ImVec2         dockspacePos  = ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + GUILayer::Get()->m_headerSize);
        ImGui::SetNextWindowPos(dockspacePos);
        ImGui::SetNextWindowSize(dockspaceSize);
        ImGui::Begin("Lina Engine Dock Space", NULL, windowFlags);
        ImGui::DockSpace(dockspaceID, ImVec2(0, 0), dockspace_flags);
        ImGui::End();

        if (!dockspaceBuilt)
        {
            BuildDockspace();
        }
    }
} // namespace Lina::Editor