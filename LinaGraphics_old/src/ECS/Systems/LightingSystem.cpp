#include "ECS/Systems/LightingSystem.hpp"
#include "Resources/ResourceStorage.hpp"
#include "Core/RenderDevice.hpp"
#include "Core/RenderEngine.hpp"
#include "ECS/Components/EntityDataComponent.hpp"
#include "ECS/Components/LightComponent.hpp"
#include "ECS/Registry.hpp"
#include "Rendering/RenderConstants.hpp"

namespace Lina::ECS
{

    const float DIRLIGHT_DISTANCE_OFFSET = 10;

    StringID pLightIconID = -1;
    StringID sLightIconID = -1;
    StringID dLightIconID = -1;

    void LightingSystem::Initialize(const String& name, ApplicationMode& appMode)
    {
        System::Initialize(name);
        m_renderEngine = Graphics::RenderEngine::Get();
        m_renderDevice = m_renderEngine->GetRenderDevice();
        m_appMode      = appMode;
    }

    void LightingSystem::UpdateComponents(float delta)
    {
        auto* ecs = ECS::Registry::Get();

        if (m_appMode == ApplicationMode::Editor || pLightIconID == -1)
        {
            // Create debug icon textures for lights
            auto* storage    = Resources::ResourceStorage::Get();
            pLightIconID = storage->GetResource<Graphics::Texture>("Resources/Editor/Textures/Icons/PLightIcon.png")->GetSID();
            sLightIconID = storage->GetResource<Graphics::Texture>("Resources/Editor/Textures/Icons/SLightIcon.png")->GetSID();
            dLightIconID = storage->GetResource<Graphics::Texture>("Resources/Editor/Textures/Icons/DLightIcon.png")->GetSID();
        }

        // Flush lights every update.
        std::get<0>(m_directionalLight) = nullptr;
        std::get<1>(m_directionalLight) = nullptr;
        m_pointLights.clear();
        m_spotLights.clear();

        // We find the lights here, for the directional light we set it as the current dirLight as there
        // only can be, actually should be one.

        // Set directional light.
        auto& dirLightView = ecs->view<EntityDataComponent, DirectionalLightComponent>();
        for (auto& entity : dirLightView)
        {
            DirectionalLightComponent* dirLight = &dirLightView.get<DirectionalLightComponent>(entity);
            if (!dirLight->GetIsEnabled())
                continue;

            EntityDataComponent& data       = dirLightView.get<EntityDataComponent>(entity);
            std::get<0>(m_directionalLight) = &data;
            std::get<1>(m_directionalLight) = dirLight;

            if (m_appMode == ApplicationMode::Editor)
                m_renderEngine->DrawIcon(data.GetLocation(), dLightIconID, 0.12f);
        }

        // For the point & spot lights, we simply find them and add them to their respective lists,
        // which is to be iterated when there is an active shader before drawing, so that we can
        // update lighting data in the shader.

        // Set point lights.
        auto& pointLightView = ecs->view<EntityDataComponent, PointLightComponent>();
        for (auto it = pointLightView.begin(); it != pointLightView.end(); ++it)
        {
            PointLightComponent* pLight = &pointLightView.get<PointLightComponent>(*it);
            if (!pLight->GetIsEnabled())
                continue;

            EntityDataComponent& data = pointLightView.get<EntityDataComponent>(*it);
            m_pointLights.push_back(std::make_pair(&data, pLight));

            if (m_appMode == ApplicationMode::Editor)
                m_renderEngine->DrawIcon(data.GetLocation(), pLightIconID, 0.12f);
        }

        // Set Spot lights.
        auto& spotLightView = ecs->view<EntityDataComponent, SpotLightComponent>();
        for (auto it = spotLightView.begin(); it != spotLightView.end(); ++it)
        {
            SpotLightComponent* sLight = &spotLightView.get<SpotLightComponent>(*it);
            if (!sLight->GetIsEnabled())
                continue;

            EntityDataComponent& data = spotLightView.get<EntityDataComponent>(*it);
            m_spotLights.push_back(std::make_pair(&data, sLight));

            if (m_appMode == ApplicationMode::Editor)
                m_renderEngine->DrawIcon(data.GetLocation(), sLightIconID, 0.12f);
        }

        m_poolSize = (int)dirLightView.size_hint() + (int)spotLightView.size_hint() + (int)pointLightView.size_hint();
    }

    void LightingSystem::SetLightingShaderData(uint32 shaderID)
    {
        // When this function is called it means a shader is activated in the
        // gpu pipeline, so we go through our available lights and update the shader
        // data according to their states.

        // Update directional light data.
        EntityDataComponent*       dirLightData = std::get<0>(m_directionalLight);
        DirectionalLightComponent* dirLight     = std::get<1>(m_directionalLight);
        if (dirLightData != nullptr && dirLight != nullptr)
        {
            Vector3 direction = (Vector3::Zero - dirLightData->GetLocation()).Normalized();
            m_renderDevice->UpdateShaderUniformInt(shaderID, SC_DIRECTIONALLIGHT_EXISTS, 1);
            m_renderDevice->UpdateShaderUniformColor(shaderID, SC_DIRECTIONALLIGHT + SC_LIGHTCOLOR, dirLight->m_color * dirLight->m_intensity);
            m_renderDevice->UpdateShaderUniformVector3(shaderID, SC_DIRECTIONALLIGHT + SC_LIGHTDIRECTION, direction);
        }
        else
            m_renderDevice->UpdateShaderUniformInt(shaderID, SC_DIRECTIONALLIGHT_EXISTS, 0);

        // Iterate point lights.
        int currentPointLightCount = 0;

        for (Vector<std::tuple<EntityDataComponent*, PointLightComponent*>>::iterator it = m_pointLights.begin(); it != m_pointLights.end(); ++it)
        {
            EntityDataComponent* data                 = std::get<0>(*it);
            PointLightComponent* pointLight           = std::get<1>(*it);
            String          currentPointLightStr = TO_STRING(currentPointLightCount);
            m_renderDevice->UpdateShaderUniformVector3(shaderID, SC_POINTLIGHTS + "[" + currentPointLightStr + "]" + SC_LIGHTPOSITION, data->GetLocation());
            m_renderDevice->UpdateShaderUniformColor(shaderID, SC_POINTLIGHTS + "[" + currentPointLightStr + "]" + SC_LIGHTCOLOR, pointLight->m_color * pointLight->m_intensity);
            m_renderDevice->UpdateShaderUniformFloat(shaderID, SC_POINTLIGHTS + "[" + currentPointLightStr + "]" + SC_LIGHTDISTANCE, pointLight->m_distance);
            m_renderDevice->UpdateShaderUniformFloat(shaderID, SC_POINTLIGHTS + "[" + currentPointLightStr + "]" + SC_LIGHTBIAS, pointLight->m_bias);
            m_renderDevice->UpdateShaderUniformFloat(shaderID, SC_POINTLIGHTS + "[" + currentPointLightStr + "]" + SC_LIGHTSHADOWFAR, pointLight->m_shadowFar);

            currentPointLightCount++;
        }

        // Iterate Spot lights.
        int currentSpotLightCount = 0;

        for (Vector<std::tuple<EntityDataComponent*, SpotLightComponent*>>::iterator it = m_spotLights.begin(); it != m_spotLights.end(); ++it)
        {
            EntityDataComponent* data                = std::get<0>(*it);
            SpotLightComponent*  spotLight           = std::get<1>(*it);
            String          currentSpotLightStr = TO_STRING(currentSpotLightCount);
            m_renderDevice->UpdateShaderUniformVector3(shaderID, SC_SPOTLIGHTS + "[" + currentSpotLightStr + "]" + SC_LIGHTPOSITION, data->GetLocation());
            m_renderDevice->UpdateShaderUniformColor(shaderID, SC_SPOTLIGHTS + "[" + currentSpotLightStr + "]" + SC_LIGHTCOLOR, spotLight->m_color * spotLight->m_intensity);
            m_renderDevice->UpdateShaderUniformVector3(shaderID, SC_SPOTLIGHTS + "[" + currentSpotLightStr + "]" + SC_LIGHTDIRECTION, data->GetRotation().GetForward());
            m_renderDevice->UpdateShaderUniformFloat(shaderID, SC_SPOTLIGHTS + "[" + currentSpotLightStr + "]" + SC_LIGHTCUTOFF, spotLight->m_cutoff);
            m_renderDevice->UpdateShaderUniformFloat(shaderID, SC_SPOTLIGHTS + "[" + currentSpotLightStr + "]" + SC_LIGHTOUTERCUTOFF, spotLight->m_outerCutoff);
            m_renderDevice->UpdateShaderUniformFloat(shaderID, SC_SPOTLIGHTS + "[" + currentSpotLightStr + "]" + SC_LIGHTDISTANCE, spotLight->m_distance);
            currentSpotLightCount++;
        }

        m_renderEngine->SetCurrentPLightCount(currentPointLightCount);
        m_renderEngine->SetCurrentSLightCount(currentSpotLightCount);
    }

    void LightingSystem::ResetLightData()
    {
        m_renderEngine->SetCurrentPLightCount(0);
        m_renderEngine->SetCurrentSLightCount(0);
    }

    Matrix LightingSystem::GetDirectionalLightMatrix()
    {

        // Used for directional shadow mapping.

        EntityDataComponent*       directionalLightData = std::get<0>(m_directionalLight);
        DirectionalLightComponent* light                = std::get<1>(m_directionalLight);

        if (directionalLightData == nullptr || light == nullptr)
            return Matrix();

        Matrix lightProjection = Matrix::Orthographic(light->m_shadowOrthoProjection.x, light->m_shadowOrthoProjection.y, light->m_shadowOrthoProjection.z, light->m_shadowOrthoProjection.w, light->m_shadowZNear, light->m_shadowZFar);
        //	Matrix lightView = Matrix::TransformMatrix(directionalLightTransform->transform.GetLocation(), directionalLightTransform->transform.GetRotation(), Vector3::One);
        Matrix lightView = glm::lookAt(directionalLightData->GetLocation(), glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));

        // Matrix lightProjection = Matrix::Orthographic(light->m_shadowOrthoProjection.x, light->m_shadowOrthoProjection.y, light->m_shadowOrthoProjection.z, light->m_shadowOrthoProjection.w,
        // light->m_shadowZNear, light->m_shadowZFar);
        Vector3    location = directionalLightData->GetLocation();
        Quaternion rotation = directionalLightData->GetRotation();
        // Matrix lightView = Matrix::InitLookAt(location, location + rotation.GetForward().Normalized(), Vector3::Up);
        return lightProjection * lightView;

        // Matrix lightView = Matrix::InitRotationFromDirection(directionalLightTransform->transform.rotation.GetForward(), directionalLightTransform->transform.rotation.GetUp());
        // Matrix lightView = Matrix::InitLookAt(directionalLightTransform == nullptr ? Vector3::Zero : directionalLightTransform->transform.location, Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
        // Matrix lightView = glm::lookAt(directionalLightTransform->transform.location, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        // Matrix lightView = Matrix::InitRotationFromDirection(directionalLightTransform->transform.rotation.GetForward(), directionalLightTransform->transform.rotation.GetUp());
        // Matrix lightView = glm::lookAt(directionalLightTransform->transform.location, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        // glm::mat4 lightView = glm::lookAt(directionalLightTransform->transform.location,
        //	glm::vec3(0.0f, 0.0f, 0.0f),
        //	glm::vec3(0.0f, 1.0f, 0.0f));
        //	return lightView * lightProjection;
    }

    Matrix LightingSystem::GetDirLightBiasMatrix()
    {
        glm::mat4 biasMatrix(0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0);
        return GetDirectionalLightMatrix() * biasMatrix;
    }

    const Vector3& LightingSystem::GetDirectionalLightPos()
    {
        EntityDataComponent* directionalLightData = std::get<0>(m_directionalLight);
        if (directionalLightData == nullptr)
            return Vector3::Zero;
        return directionalLightData->GetLocation();
    }

    Vector<Matrix> LightingSystem::GetPointLightMatrices(Vector3 lp, Vector2i m_resolution, float near, float farPlane)
    {
        // Used for point light shadow mapping.

        float     aspect     = (float)m_resolution.x / (float)m_resolution.y;
        float     znear      = near;
        float     zfar       = farPlane;
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, znear, zfar);
        // glm::mat4 shadowProj = Matrix::Perspective(45, aspect, znear, zfar);

        Vector<Matrix> shadowTransforms;
        glm::vec3           lightPos = lp;

        // shadowTransforms.push_back(shadowProj * glm::lookAtLH(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        // shadowTransforms.push_back(shadowProj * glm::lookAtLH(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        // shadowTransforms.push_back(shadowProj * glm::lookAtLH(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
        // shadowTransforms.push_back(shadowProj * glm::lookAtLH(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
        // shadowTransforms.push_back(shadowProj * glm::lookAtLH(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        // shadowTransforms.push_back(shadowProj * glm::lookAtLH(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

        // + Z DO�RU
        // shadowTransforms.push_back(shadowProj * Matrix::InitLookAt(lightPos, Vector3::Right, Vector3::Up));
        // shadowTransforms.push_back(shadowProj * Matrix::InitLookAt(lightPos, Vector3::Left, Vector3::Up));
        // shadowTransforms.push_back(shadowProj * Matrix::InitLookAt(lightPos, Vector3::Down, Vector3::Back));
        // shadowTransforms.push_back(shadowProj * Matrix::InitLookAt(lightPos, Vector3::Up, Vector3::Forward));
        // shadowTransforms.push_back(shadowProj * Matrix::InitLookAt(lightPos, Vector3::Back, Vector3::Down));
        // shadowTransforms.push_back(shadowProj * Matrix::InitLookAt(lightPos, Vector3::Forward, Vector3::Down));
        //
        // shadowTransforms.push_back(shadowProj * glm::lookAtLH(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        // shadowTransforms.push_back(shadowProj * glm::lookAtLH(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        //
        // shadowTransforms.push_back(shadowProj * glm::lookAtLH(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
        // shadowTransforms.push_back(shadowProj * glm::lookAtLH(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
        //
        // shadowTransforms.push_back(shadowProj * glm::lookAtLH(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        // shadowTransforms.push_back(shadowProj * glm::lookAtLH(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));
        return shadowTransforms;
    }

} // namespace Lina::ECS
