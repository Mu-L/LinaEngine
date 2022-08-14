#include "ECS/Systems/EditorCameraSystem.hpp"

#include "Core/Application.hpp"
#include "Core/EditorCommon.hpp"
#include "Core/InputEngine.hpp"
#include "ECS/Components/EntityDataComponent.hpp"
#include "ECS/Components/FreeLookComponent.hpp"
#include "ECS/Registry.hpp"
#include "Math/Math.hpp"
#include "Panels/LevelPanel.hpp"
#include "imgui/imgui.h"

namespace Lina::ECS
{

    void EditorCameraSystem::Initialize(const String& name, Editor::LevelPanel& scenePanel)
    {
        System::Initialize(name);
        m_levelPanel  = &scenePanel;
        m_inputEngine = Input::InputEngine::Get();
        m_poolSize    = 1;
    }

    void ECS::EditorCameraSystem::UpdateComponents(float delta)
    {

        if (m_editorCamera != entt::null)
        {
            FreeLookComponent& freeLook = ECS::Registry::Get()->get<FreeLookComponent>(m_editorCamera);
            m_movementSpeeds            = freeLook.m_movementSpeeds;
            m_rotationSpeeds            = freeLook.m_rotationSpeeds;

            if (!freeLook.GetIsEnabled() || !m_isActive || !m_levelPanel->IsFocused() || !m_levelPanel->IsHovered())
                return;

            EntityDataComponent& data = ECS::Registry::Get()->get<EntityDataComponent>(m_editorCamera);

            Vector3    location = data.GetPosition();
            Quaternion rotation = data.GetRotation();

            RotateBehaviour(delta, rotation, freeLook.m_angles);
            data.SetLocalRotation(rotation);

            MoveBehaviour(delta, freeLook.m_mouseDrag, location, rotation);
            data.SetPosition(location);
        }
    }

    void EditorCameraSystem::MoveBehaviour(float delta, Vector2& mouseDragStart, Vector3& location, Quaternion& rotation)
    {
        // Handle movement.
        float   horizontalKey    = m_inputEngine->GetHorizontalAxisValue();
        float   verticalKey      = m_inputEngine->GetVerticalAxisValue();
        float   sprintMultiplier = Input::InputEngine::Get()->GetKey(Input::InputCode::LSHIFT) ? 3.0f : 1.0f;
        Vector3 fw               = rotation.GetForward();
        Vector3 up               = rotation.GetUp();
        Vector3 rg               = rotation.GetRight();

        location += verticalKey * delta * m_movementSpeeds.x * fw.Normalized() * sprintMultiplier * m_cameraSpeedMultiplier;
        location += horizontalKey * delta * m_movementSpeeds.y * rg.Normalized() * sprintMultiplier * m_cameraSpeedMultiplier;
        const float scrollY = -Input::InputEngine::Get()->GetMouseScroll().y;
        location += scrollY * 1.8f * delta * -fw.Normalized() * m_movementSpeeds.x * sprintMultiplier;

        if (m_inputEngine->GetMouseButtonDown(LINA_MOUSE_3))
            mouseDragStart = m_inputEngine->GetMousePosition();

        if (m_inputEngine->GetMouseButton(LINA_MOUSE_3))
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

            const Vector2 mousePosition = m_inputEngine->GetMousePosition();
            const Vector2 deltaMouse    = mousePosition - mouseDragStart;
            location += delta * rg * -deltaMouse.x;
            location += delta * up * deltaMouse.y;
            mouseDragStart = mousePosition;
        }
    }

    void EditorCameraSystem::RotateBehaviour(float delta, Quaternion& q, Vector2& angles)
    {
        static Quaternion qStart;
        if (m_inputEngine->GetMouseButtonDown(LINA_MOUSE_2))
        {
            m_inputEngine->SetAxisMousePos(m_inputEngine->GetMousePosition());
            m_targetXAngle = angles.x;
            m_targetYAngle = angles.y;
        }

        // Holding right click enables rotating.
        if (m_inputEngine->GetMouseButton(LINA_MOUSE_2))
        {

            Vector2 mouseAxis = m_inputEngine->GetMouseAxis();

            m_targetYAngle += mouseAxis.y * m_rotationSpeeds.x;
            m_targetXAngle += mouseAxis.x * m_rotationSpeeds.y;

            angles.y = Math::Lerp(angles.y, m_targetYAngle, 15 * delta);
            angles.x = Math::Lerp(angles.x, m_targetXAngle, 15 * delta);

            Quaternion qX = Quaternion::AxisAngle(Vector3::Up, angles.x);
            Quaternion qY = Quaternion::AxisAngle(Vector3::Right, angles.y);
            q             = qX * qY ;
        }
    }
} // namespace Lina::ECS
