/*
Class: RenderEngine

Responsible for managing the whole rendering pipeline, creating frame buffers, textures,
materials, meshes etc. Also handles most of the memory management for the rendering pipeline.

Timestamp: 4/15/2019 12:26:31 PM
*/

#pragma once

#ifndef RenderEngine_HPP
#define RenderEngine_HPP

#include "ECS/SystemList.hpp"
#include "ECS/Systems/AnimationSystem.hpp"
#include "ECS/Systems/CameraSystem.hpp"
#include "ECS/Systems/FrustumSystem.hpp"
#include "ECS/Systems/LightingSystem.hpp"
#include "ECS/Systems/ModelNodeSystem.hpp"
#include "ECS/Systems/ReflectionSystem.hpp"
#include "ECS/Systems/SpriteRendererSystem.hpp"
#include "RenderDevice.hpp"
#include "Window.hpp"
#include "Rendering/Mesh.hpp"
#include "Rendering/Model.hpp"
#include "Rendering/PostProcessEffect.hpp"
#include "Rendering/RenderBuffer.hpp"
#include "Rendering/RenderSettings.hpp"
#include "Rendering/RenderingCommon.hpp"
#include "Rendering/UniformBuffer.hpp"
#include "Rendering/VertexArray.hpp"

#include <functional>
#include <queue>
#include "Data/Set.hpp"

namespace Lina
{
    class Engine;

    namespace Resources
    {
        class ResourceStorage;
    }

    namespace Event
    {
        class EventSystem;
        struct EAllResourcesOfTypeLoaded;
        struct EDrawLine;
        struct EDrawBox;
        struct EDrawCircle;
        struct EDrawSphere;
        struct EDrawHemiSphere;
        struct EDrawCapsule;
        struct EWindowResized;
    } // namespace Event
} // namespace Lina

namespace Lina::Graphics
{
    class Shader;

    struct BufferValueRecord
    {
        float zNear;
        float zFar;
    };

    class RenderEngine
    {
    public:
        static RenderEngine* Get()
        {
            return s_renderEngine;
        }

        /// <summary>
        /// Any system added to the rendering pipeline will be updated within the render loop.
        /// </summary>
        void AddToRenderingPipeline(ECS::System& system);

        /// <summary>
        /// Any system added to the animation pipeline will be updated within the animation loop.
        /// </summary>
        void AddToAnimationPipeline(ECS::System& system);

        /// <summary>
        /// Sets the screen position and size, resizes the framebuffers accordingly.
        /// </summary>
        void SetScreenDisplay(Vector2i offset, Vector2i size);

        /// <summary>
        /// Use this to change current draw parameters. Will be replaced with in-engine parameters every frame.
        /// </summary>
        /// <param name="params"></param>
        void SetDrawParameters(const DrawParams& params);

        /// <summary>
        /// Sets uniform render settings from the current Render Settings struct.
        /// </summary>
        void UpdateRenderSettings();

        /// <summary>
        /// Sets the current skybox material used to draw the scene.
        /// </summary>
        inline void SetSkyboxMaterial(Material* skyboxMaterial)
        {
            m_skyboxMaterial = skyboxMaterial;
        }

        /// <summary>
        /// Returns a texture pointer to the final drawed image, which is usually drawn to the screen as a full-screen quad.
        /// </summary>
        uint32 GetFinalImage();

        /// <summary>
        /// Returns a texture pointer to the current shadow map.
        /// </summary>
        uint32 GetShadowMapImage();

        /// <summary>
        /// Given a texture & a location, captures 6-faced cubemap & writes into the given texture.
        /// </summary>
        void CaptureReflections(Texture& writeTexture, const Vector3& areaLocation, const Vector2i& resolution);

        /// <summary>
        /// Draws the current skybox into an irradiance cubemap.
        /// </summary>
        void CaptureSkybox();

        /// <summary>
        /// Given an HDRI loaded texture, captures & calculates HDRI cube probes & writes it into global HRDI buffer.
        /// This buffer will be sent to the materials whose HDRI support is enabled.
        /// </summary>
        void CaptureCalculateHDRI(Texture& hdriTexture);

        /// <summary>
        /// Add the texture with the given StringID to the debug icon buffer.
        /// </summary>
        void DrawIcon(Vector3 p, StringID textureID, float size = 1.0f);

        /// <summary>
        /// Adds a line to the debug buffer with given parameters.
        /// </summary>
        void DrawLine(Vector3 p1, Vector3 p2, Color col, float width = 1.0f);

        /// <summary>
        /// Pass in any run-time constructed shader. The shader will be drawn to a full-screen quad & added as a
        /// post-process effect.
        /// </summary>
        PostProcessEffect& AddPostProcessEffect(Shader* shader);

        /// <summary>
        /// Renders the given model in a preview scene and returns the resulting image.
        /// </summary>
        uint32 RenderModelPreview(Model* model, Matrix& modelMatrix, RenderTarget* overrideTarget = nullptr, Material* materialOverride = nullptr);

        void UpdateShaderData(Material* mat, bool lightPass = false);

        inline UniformBuffer& GetViewBuffer()
        {
            return m_globalViewBuffer;
        }
        inline DrawParams GetMainDrawParams()
        {
            return m_defaultDrawParams;
        }
        inline RenderDevice* GetRenderDevice()
        {
            return &m_renderDevice;
        }
        inline Texture& GetHDRICubemap()
        {
            return m_hdriCubemap;
        }
        inline uint32 GetScreenQuadVAO()
        {
            return m_screenQuadVAO;
        }
        inline Vector2i GetScreenSize()
        {
            return m_screenSize;
        }
        inline Vector2i GetScreenPos()
        {
            return m_screenPos;
        }
        inline ECS::CameraSystem* GetCameraSystem()
        {
            return &m_cameraSystem;
        }
        inline ECS::LightingSystem* GetLightingSystem()
        {
            return &m_lightingSystem;
        }
        inline ECS::ModelNodeSystem* GetModelNodeSystem()
        {
            return &m_modelNodeSystem;
        }
        inline ECS::ReflectionSystem* GetReflectionSystem()
        {
            return &m_reflectionSystem;
        }
        inline ECS::FrustumSystem* GetFrustumSystem()
        {
            return &m_frustumSystem;
        }
        inline Texture* GetDefaultTexture()
        {
            return &m_defaultTexture;
        }
        inline Material* GetDefaultUnlitMaterial()
        {
            return m_defaultUnlit;
        }
        inline Material* GetDefaultLitMaterial()
        {
            return m_defaultLit;
        }
        inline Material* GetDefaultSkyboxMaterial()
        {
            return m_defaultSkybox;
        }
        inline Material* GetDefaultSkyboxHDRIMaterial()
        {
            return m_defaultSkyboxHDRI;
        }
        inline Material* GetDefaultSpriteMaterial()
        {
            return m_defaultSprite;
        }
        inline Shader* GetDefaultUnlitShader()
        {
            return m_standardUnlitShader;
        }
        inline Shader* GetDefaultLitShader()
        {
            return m_standardLitShader;
        }
        inline void SetCurrentPLightCount(int count)
        {
            m_currentPointLightCount = count;
        }
        inline void SetCurrentSLightCount(int count)
        {
            m_currentSpotLightCount = count;
        }
        inline const ECS::SystemList& GetRenderingPipeline()
        {
            return m_renderingPipeline;
        }
        inline const ECS::SystemList& GetAnimationPipeline()
        {
            return m_animationPipeline;
        }
        inline SamplerParameters GetPrimaryRTParams()
        {
            return m_primaryRTParams;
        }
        inline void SetAppData(float delta, float elapsed, const Vector2& mousePos)
        {
            m_deltaTime     = delta;
            m_elapsedTime   = elapsed;
            m_mousePosition = mousePos;
        }
        inline Material* GetSkyboxMaterial()
        {
            return m_skyboxMaterial;
        }

    private:
        friend class Engine;
        RenderEngine()  = default;
        ~RenderEngine() = default;
        void ConnectEvents();
        void Initialize(ApplicationMode appMode, RenderSettings* renderSettings, const WindowProperties& windowProps);
        void Shutdown();
        void Render(float interpolation);
        void Tick(float delta);
        void UpdateSystems(float interpolation);
        void DrawSceneObjects(DrawParams& drawpParams, Material* overrideMaterial = nullptr, bool completeFlush = true);
        void DrawSkybox();
        void SetHDRIData(Material* mat);
        void RemoveHDRIData(Material* mat);
        void ProcessDebugQueue();

    private:
        void OnDrawLine(const Event::EDrawLine& event);
        void OnDrawBox(const Event::EDrawBox& event);
        void OnDrawCircle(const Event::EDrawCircle& event);
        void OnDrawSphere(const Event::EDrawSphere& event);
        void OnDrawHemiSphere(const Event::EDrawHemiSphere& event);
        void OnDrawCapsule(const Event::EDrawCapsule& event);
        void OnWindowResized(const Event::EWindowResized& event);
        void OnAllResourcesOfTypeLoaded(const Event::EAllResourcesOfTypeLoaded& ev);
        void OnResourceReloaded(const Event::EResourceReloaded& ev);
        bool ValidateEngineShaders();
        void SetupEngineShaders();
        void ConstructEngineMaterials();
        void ConstructRenderTargets();
        void DumpMemory();
        void Draw();
        void DrawFinalize(RenderTarget* overrideTarget = nullptr);
        void UpdateUniformBuffers();
        void CalculateHDRICubemap(Texture& hdriTexture, glm::mat4& captureProjection, glm::mat4 views[6]);
        void CalculateHDRIIrradiance(Matrix& captureProjection, Matrix views[6]);
        void CalculateHDRIPrefilter(Matrix& captureProjection, Matrix views[6]);
        void CalculateHDRIBRDF(Matrix& captureProjection, Matrix views[6]);

    private:
        static RenderEngine* s_renderEngine;
        ApplicationMode            m_appMode   = ApplicationMode::Editor;
        Window*              m_appWindow = nullptr;
        RenderDevice         m_renderDevice;
        Event::EventSystem*        m_eventSystem = nullptr;

        ModelNode* m_cubeNode   = nullptr;
        ModelNode* m_sphereNode = nullptr;

        RenderTarget m_primaryRenderTarget;
        RenderTarget m_secondaryRenderTarget;
        RenderTarget m_previewRenderTarget;
        RenderTarget m_primaryMSAATarget;
        RenderTarget m_pingPongRenderTarget1;
        RenderTarget m_pingPongRenderTarget2;
        RenderTarget m_hdriCaptureRenderTarget;
        RenderTarget m_reflectionCaptureRenderTarget;
        RenderTarget m_skyboxIrradianceCaptureRenderTarget;
        RenderTarget m_shadowMapTarget;
        RenderTarget m_pLightShadowTargets[MAX_POINT_LIGHTS];
        RenderTarget m_gBuffer;

        RenderBuffer m_primaryBuffer;
        RenderBuffer m_primaryMSAABuffer;
        RenderBuffer m_secondaryRenderBuffer;
        RenderBuffer m_previewRenderBuffer;
        RenderBuffer m_hdriCaptureRenderBuffer;
        RenderBuffer m_reflectionCaptureRenderBuffer;
        RenderBuffer m_skyboxIrradianceCaptureRenderBuffer;
        RenderBuffer m_gBufferRenderBuffer;

        Texture m_gBufferPosition;
        Texture m_gBufferNormal;
        Texture m_gBufferAlbedo;
        Texture m_gBufferEmission;
        Texture m_gBufferMetallicRoughnessAOWorkflow;

        Texture  m_primaryMSAARTTexture0;
        Texture  m_primaryMSAARTTexture1;
        Texture  m_primaryRTTexture0;
        Texture  m_primaryRTTexture1;
        Texture  m_secondaryRTTexture;
        Texture  m_previewRTTexture;
        Texture  m_pingPongRTTexture1;
        Texture  m_pingPongRTTexture2;
        Texture  m_hdriCubemap;
        Texture  m_hdriIrradianceMap;
        Texture  m_hdriPrefilterMap;
        Texture  m_hdriLutMap;
        Texture  m_shadowMapRTTexture;
        Texture  m_defaultCubemapTexture;
        Texture  m_pLightShadowTextures[MAX_POINT_LIGHTS];
        Texture  m_defaultTexture;
        Texture  m_skyboxIrradianceCubemap;
        Texture  m_reflectionCubemap;
        Texture* m_lastCapturedHDR = nullptr;

        // Frame buffer texture parameters
        SamplerParameters m_primaryRTParams;
        SamplerParameters m_pingPongRTParams;
        SamplerParameters m_shadowsRTParams;

        Mesh m_quadMesh;

        Material  m_gBufferLightPassMaterial;
        Material  m_screenQuadFinalMaterial;
        Material  m_screenQuadBlurMaterial;
        Material  m_screenQuadOutlineMaterial;
        Material* m_skyboxMaterial = nullptr;
        Material  m_debugLineMaterial;
        Material  m_debugIconMaterial;
        Material  m_hdriMaterial;
        Material  m_shadowMapMaterial;
        Material  m_defaultSkyboxMaterial;
        Material  m_pLightShadowDepthMaterial;
        Material* m_defaultSkyboxHDRI = nullptr;
        Material* m_defaultUnlit      = nullptr;
        Material* m_defaultLit        = nullptr;
        Material* m_defaultSkybox     = nullptr;
        Material* m_defaultSprite     = nullptr;

        Shader* m_hdriBRDFShader            = nullptr;
        Shader* m_hdriPrefilterShader       = nullptr;
        Shader* m_hdriEquirectangularShader = nullptr;
        Shader* m_hdriIrradianceShader      = nullptr;
        Shader* m_standardUnlitShader       = nullptr;
        Shader* m_standardLitShader         = nullptr;

        DrawParams m_defaultDrawParams;
        DrawParams m_skyboxDrawParams;
        DrawParams m_fullscreenQuadDP;
        DrawParams m_shadowMapDrawParams;

        UniformBuffer m_globalViewBuffer;
        UniformBuffer m_globalLightBuffer;
        UniformBuffer m_globalDebugBuffer;
        UniformBuffer m_globalAppDataBuffer;

        RenderingDebugData m_debugData;
        RenderSettings*    m_renderSettings;

        // Structure that keeps track of current buffer values
        BufferValueRecord m_bufferValueRecord;

        ECS::AnimationSystem        m_animationSystem;
        ECS::CameraSystem           m_cameraSystem;
        ECS::ModelNodeSystem        m_modelNodeSystem;
        ECS::ReflectionSystem       m_reflectionSystem;
        ECS::SpriteRendererSystem   m_spriteRendererSystem;
        ECS::LightingSystem         m_lightingSystem;
        ECS::FrustumSystem          m_frustumSystem;
        ECS::SystemList             m_renderingPipeline;
        ECS::SystemList             m_animationPipeline;
        Resources::ResourceStorage* m_storage = nullptr;

    private:
        uint32 m_skyboxVAO     = 0;
        uint32 m_screenQuadVAO = 0;
        uint32 m_hdriCubeVAO   = 0;
        uint32 m_lineVAO       = 0;

        int m_currentSpotLightCount  = 0;
        int m_currentPointLightCount = 0;

        Vector2i m_hdriResolution             = Vector2i(512, 512);
        Vector2i m_shadowMapResolution        = Vector2i(2048, 2048);
        Vector2i m_screenPos                  = Vector2i(0, 0);
        Vector2i m_screenSize                 = Vector2i(0, 0);
        Vector2i m_pLightShadowResolution     = Vector2i(1024, 1024);
        Vector2i m_skyboxIrradianceResolution = Vector2i(1024, 1024);
        bool     m_firstFrameDrawn            = false;
        float    m_deltaTime                  = 0.0f;
        float    m_elapsedTime                = 0.0f;
        Vector2  m_mousePosition              = Vector2::Zero;

        std::queue<DebugLine>                m_debugLineQueue;
        std::queue<DebugIcon>                m_debugIconQueue;
        Map<Shader*, PostProcessEffect> m_postProcessMap;
    };

} // namespace Lina::Graphics

#endif
