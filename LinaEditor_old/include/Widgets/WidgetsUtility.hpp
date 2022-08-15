/*
Class: WidgetsUtility

Wraps ImGui functions and provides extended functionality.

Timestamp: 10/11/2020 1:39:01 PM
*/

#pragma once

#ifndef WidgetsUtility_HPP
#define WidgetsUtility_HPP

#include "ECS/Registry.hpp"
#include "Utility/StringId.hpp"
#include "Utility/UtilityFunctions.hpp"
#include "Core/CommonResources.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_internal.h"

#include "Data/Map.hpp"
#include <Data/String.hpp>

namespace Lina
{
    class Quaternion;

    namespace Graphics
    {
        class Material;
        class Model;
        class ModelNode;
        class Shader;
    } // namespace Graphics

    namespace Audio
    {
        class Audio;
    }

    namespace Physics
    {
        class PhysicsMaterial;
    }
} // namespace Lina

namespace Lina::Editor
{
    class MenuBarElement;
    class Menu;

    class WidgetsUtility
    {

    public:
        /// <summary>
        /// Draws a simple tooltip pop-up.
        /// </summary>
        static void Tooltip(const char* tooltip);

        /// <summary>
        /// Returns whether two ImGui colors are equal or not.
        /// </summary>
        static bool ColorsEqual(ImVec4 col1, ImVec4 col2);

        /// <summary>
        /// Returns whether this item's path is either Root, Engine or Editor folders.
        /// </summary>
        /// <param name="item"></param>
        /// <returns></returns>
        static bool IsProtectedDirectory(Utility::DirectoryItem* item);

        /// <summary>
        /// Draws a folder in the style of Resources Panel.
        /// </summary>
        static bool DrawTreeFolder(Utility::Folder* folder, Utility::Folder*& selectedFolder, bool canRename);

        /// <summary>
        /// Saves the editor camera's aspect ratio as well as orientation prior to taking a model/shader/material preview.
        /// </summary>
        /// <param name="model"></param>
        static void SaveEditorCameraBeforeSnapshot(float aspectRatio);

        /// <summary>
        /// Sets the orientation of the editor camera for taking a model/shader/material preview.
        /// </summary>
        static void SetEditorCameraForSnapshot();

        /// <summary>
        /// Resets the editor camera's orientation to the saved orientation before snapshot.
        /// </summary>
        static void ResetEditorCamera();

        /// <summary>
        /// Draws the rectangular nodes in the Resources right pane.
        /// </summary>
        /// <param name="isFolder"></param>
        /// <param name="fullPath"></param>
        static void DrawResourceNode(Utility::DirectoryItem* item, bool selected, bool* renamedItem, float sizeMultiplier = 1.0f, bool canRename = false);

        /// <summary>
        /// Button with color-picker pop-up
        /// </summary>
        /// <param name="id"></param>
        /// <param name="colorX"></param>
        static void ColorButton(const char* id, float* colorX);

        /// <summary>
        /// Draws a button that the user can toggle from left-to right with toggling animation.
        /// </summary>
        static bool ToggleButton(const char* label, bool* toggled, ImVec2 size = ImVec2(0.0f, 0.0f)); // toggle button

        /// <summary>
        /// Begins a child window with a custom handle to move it around.
        /// </summary>
        static void BeginMovableChild(const char* childID, ImVec2 size, const ImVec2& defaultPosition, const ImRect& confineRect, bool isHorizontal, ImVec2 iconCursorOffset);

        /// <summary>
        /// Displays a small child for manipulating editor camera settings
        /// </summary>
        /// <param name="position"></param>
        static void DisplayEditorCameraSettings(const ImVec2 position);

        /// <summary>
        /// Draws a movable child window for controlling gizmos of translation, rotation & scale operations.
        /// </summary>
        /// <param name="childID"></param>
        static void TransformOperationTools(const char* childID, ImRect confineRect);

        /// <summary>
        /// Draws a movable child window for controller play buttons.
        /// </summary>
        static void PlayOperationTools(const char* childID, ImRect confineRect);

        /// <summary>
        /// Draws a combo-box for primitive selection.
        /// </summary>
        static int SelectPrimitiveCombobox(const char* comboID, const Vector<String>& primitives, int currentSelected, float widthDecrease = 0.0f);

        /// <summary>
        /// Draws a full-window-width line, the Y position determines the local offset from current cursor pos.
        /// Use ImGuiCol_Text to style.
        /// </summary>
        /// <param name="thickness"></param>
        /// <param name="color"></param>
        static void HorizontalDivider(float yOffset = 0.0f, float thickness = 1.0f, float maxOverride = 0.0f);

        /// <summary>
        /// Draws minimize, maximize and close buttons on the window. Pass in the window ID used for BeginWindow().
        /// </summary>
        static void WindowButtons(const char* windowID, float yOffset = 0.0f, bool isAppWindow = false, float sizeMultiplier = 1.0f);

        /// <summary>
        /// Draws a custom title bar for the window.
        /// </summary>
        /// <param name="label"></param>
        static void WindowTitlebar(const char* id);

        /// <summary>
        /// Draws multiple horizontal dividers to create a drop shadow effect.
        /// </summary>
        static void DropShadow();

        /// <summary>
        /// Draws a horizontal reactangle, with foldout caret, component title, icon, and component buttons.
        /// </summary>
        static bool ComponentHeader(TypeID tid, const char* componentLabel, const char* componentIcon, bool* toggled, bool* removed, bool* copied, bool* pasted, bool* resetted, bool moveButton = true, bool disableHeader = false);

        /// <summary>
        /// Draws a header same style as component headers, no icons or component buttons. Returns true if pressed.
        /// Send in a cursor pos value to get the cursor position inside the header.
        /// </summary>
        static bool Header(TypeID tid, const char* title, ImVec2* cursorPos = nullptr);

        /// <summary>
        /// Draws a simple caret and a title, return true upon press.
        /// </summary>
        static bool CaretTitle(const char* title, const String& id);

        /// <summary>
        /// Draws a property label, automatically sets the cursor position, asks for the same line.
        /// </summary>
        /// <param name="label"></param>
        static void PropertyLabel(const char* label, bool sameLine = true, const String& tooltip = "");

        /// <summary>
        /// Base implementation of a combobox begin, uses ImGui Combo with custom remove buttons.
        /// </summary>
        static bool BeginComboBox(const char* comboID, const char* label, bool hasRemoveButton = false, float widthDecrease = 0.0f);

        /// <summary>
        /// Base implementation of a combobox end, uses ImGui Combo with custom remove buttons.
        /// </summary>
        static bool PostComboBox(const char* id);

        /// <summary>
        /// Draws a vertical divider with functionality to move on horizontal axis.
        /// </summary>
        static void VerticalResizeDivider(bool canResize, float* pressedPos, float* leftPaneWidth, float leftPaneMinWidth, float leftPaneMaxWidth, bool* lockWindowPos, bool* dragging);

        /// <summary>
        /// Drop-down combo-box for selecting physics simulation type.
        /// </summary>
        static int SimulationTypeComboBox(const char* comboID, int currentShapeID);

        /// <summary>
        /// Drop-down combo-box for selecting collision shapes.
        /// </summary>
        static int CollisionShapeComboBox(const char* comboID, int currentShapeID);

        /// <summary>
        /// Drop-down combo-box for selecting material surface type.
        /// </summary>
        static int SurfaceTypeComboBox(const char* comboID, int currentType);

        /// <summary>
        /// Drop-down combo-box for selecting material workflow type.
        /// </summary>
        static int WorkflowComboBox(const char* comboID, int currentType);

        /// <summary>
        /// Draws a combo-box selection type of frame which launches the resource selector panel upon clicking.
        /// </summary>
        static StringID ResourceSelection(const String& id, void* currentResource, void* currentHandle, const char* resourceStr, bool* removed, TypeID resourceType);

        /// <summary>
        /// Material selection helper, calls ResourceSelection().
        /// </summary>
        static StringID ResourceSelectionMaterial(const String& id, void* handleAddr);

        /// <summary>
        /// Material selection helper, calls ResourceSelection().
        /// </summary>
        static StringID ResourceSelectionTexture(const String& id, void* handleAddr);

        /// <summary>
        /// Material selection helper, calls ResourceSelection().
        /// </summary>
        static StringID ResourceSelectionAudio(const String& id, void* handleAddr);

        /// <summary>
        /// Material selection helper, calls ResourceSelection().
        /// </summary>
        static StringID ResourceSelectionPhysicsMaterial(const String& id, void* handleAddr);

        /// <summary>
        /// Material selection helper, calls ResourceSelection().
        /// </summary>
        static StringID ResourceSelectionShader(const String& id, void* handleAddr);

        /// <summary>
        /// Default IMGUI button with fixed styling options.
        /// </summary>
        static bool Button(const char* label, const ImVec2& size = ImVec2(0, 0), float textSize = 1.0f, float rounding = 0.0f, ImVec2 contentOffset = ImVec2(0.0f, 0.0f), bool locked = false);

        /// <summary>
        /// Draws a simple icon button with no background, hovering sets icon color.
        /// </summary>
        static bool IconButton(const char* icon, bool useSmallIcon = true);

        /// <summary>
        /// Tree node with a custom arrow.
        /// </summary>
        static bool TreeNode(const void* id, ImGuiTreeNodeFlags flags, const char* name, bool drawArrow);

        /// <summary>
        /// Dragging functionality for custom drag widgets.
        /// </summary>
        static void DragBehaviour(const char* id, float* var, ImRect rect);

        /// <summary>
        /// Dragging functionality for custom drag widgets.
        /// </summary>
        static void DragBehaviour(const char* id, int* var);

        /// <summary>
        /// Draws ImGui::DragFloat with custom dragger.
        /// </summary>
        static bool DragFloat(const char* id, const char* label, float* var, float width = -1.0f);

        /// <summary>
        /// Draws ImGui::DragInt with custom dragger.
        /// </summary>
        static bool DragInt(const char* id, const char* label, int* var, float width = -1.0f);

        /// <summary>
        /// Draws 2 drag floats side by side.
        /// </summary>
        static bool DragVector2(const char* id, float* var);

        /// <summary>
        /// Draws 2 drag floats side by side.
        /// </summary>
        static bool DragVector2i(const char* id, int* var);

        /// <summary>
        /// Draws 3 drag floats side by side.
        /// </summary>
        static bool DragVector3(const char* id, float* var);

        /// <summary>
        /// Draws 4 drag floats side by side.
        /// </summary>
        static bool DragVector4(const char* id, float* var);

        /// <summary>
        /// Returns current window position, excluding the window's title bar.
        /// </summary>
        /// <returns></returns>
        static ImVec2 GetWindowPosWithContentRegion();

        /// <summary>
        /// Returns current window size, excluding the window's title bar.
        /// </summary>
        /// <returns></returns>
        static ImVec2 GetWindowSizeWithContentRegion();

        /// <summary>
        /// Sets the cursor pos X such that the next item will be positioned in the middle of the column,
        /// given item width.
        /// </summary>
        /// <param name="itemWidth"></param>
        static void TableAlignCenter(float itemWidth);

        /// <summary>
        /// Draws a text centered within the current window.
        /// </summary>
        static void CenteredText(const char* label);

        /// <summary>
        /// Sets ImGui cursor position X & Y to the current window center.
        /// </summary>
        static void CenterCursor();

        /// <summary>
        /// Sets ImGui cursor position X to current window center.
        /// </summary>
        static void CenterCursorX();

        /// <summary>
        /// Sets ImGui cursor position Y to current window center.
        /// </summary>
        static void CenterCursorY();

        /// <summary>
        /// Increments ImGui Cursor, both X & Y.
        /// </summary>
        static void IncrementCursorPos(const ImVec2& v);

        /// <summary>
        /// Increments ImGui Cursor Pos X
        /// </summary>
        static void IncrementCursorPosX(float f);

        /// <summary>
        /// Increments ImGui Cursor Pos Y
        /// </summary>
        static void IncrementCursorPosY(float f);

        /// <summary>
        /// Use Push & Pop Scaled Font functions to scale only a single text/icon item.
        /// </summary>
        static void PushScaledFont(float defaultScale = 0.6f);

        /// <summary>
        /// Use Push & Pop Scaled Font functions to scale only a single text/icon item.
        /// </summary>
        static void PopScaledFont();

        /// <summary>
        /// ImGui Styling option, pushes ImGuiStyleVar_FramePadding X & Y
        /// </summary>
        static void FramePadding(const ImVec2& amt);

        /// <summary>
        /// ImGui Styling option, pushes ImGuiStyleVar_FramePadding X
        /// </summary>
        static void FramePaddingX(float amt);

        /// <summary>
        /// ImGui Styling option, pushes ImGuiStyleVar_FramePadding Y
        /// </summary>
        static void FramePaddingY(float amt);

        /// <summary>
        /// ImGui Styling option, pushes ImGuiStyleVar_FrameRounding
        /// </summary>
        static void FrameRounding(float rounding);

        /// <summary>
        /// ImGui Styling option, pushes ImGuiStyleVar_WindowPadding, both X & Y
        /// </summary>
        static void WindowPadding(const ImVec2& amt);

        /// <summary>
        /// ImGui Styling option, pushes ImGuiStyleVar_WindowPadding X
        /// </summary>
        static void WindowPaddingX(float amt);

        /// <summary>
        /// ImGui Styling option, pushes ImGuiStyleVar_WindowPadding Y
        /// </summary>
        static void WindowPaddingY(float amt);

        /// <summary>
        /// ImGui Styling option, pushes ImGuiStyleVar_ItemSpacing, both X & Y
        /// </summary>
        static void ItemSpacing(const ImVec2& amt);

        /// <summary>
        /// ImGui Styling option, pushes ImGuiStyleVar_ItemSpacing X
        /// </summary>
        static void ItemSpacingX(float amt);

        /// <summary>
        /// ImGui Styling option, pushes ImGuiStyleVar_ItemSpacing Y
        /// </summary>
        static void ItemSpacingY(float amt);

        /// <summary>
        /// ImGui Styling option, pushes ImGuiStyleVar_WindowRounding, both X & Y
        /// </summary>
        static void WindowRounding(float rounding);

        /// <summary>
        /// Pushes style vars for creating a uniform popup accross the engine.
        /// </summary>
        static void PushPopupStyle();

        /// <summary>
        /// Pops the last popup style.
        /// </summary>
        static void PopPopupStyle();

        /// <summary>
        /// Pops ImGui StyleVar Stack (ImGui::PopStyleVar)
        /// </summary>
        static void PopStyleVar();

        /// <summary>
        /// Draws a simple icon in the current cursor position, offers to modify the cursor to align.
        /// </summary>
        static void Icon(const char* label, bool align, float scale = 0.6f);

        /// <summary>
        /// Pushes the small icon font to the font stack & draws icon, then pops the stack.
        /// </summary>
        static void IconSmall(const char* icon);

        /// <summary>
        /// Pushes the default icon font to the font stack & draws icon, then pops the stack.
        /// </summary>
        /// <param name="icon"></param>
        static void IconDefault(const char* icon);

        /// <summary>
        /// Pushes the default icon font.
        /// </summary>
        static void PushIconFontDefault();

        /// <summary>
        /// Pushes the small icon font.
        /// </summary>
        static void PushIconFontSmall();

        /// <summary>
        /// Draws a clickable button to select OS path.
        /// </summary>
        static String PathSelectPopup(const String& original);
    };
} // namespace Lina::Editor

#endif
