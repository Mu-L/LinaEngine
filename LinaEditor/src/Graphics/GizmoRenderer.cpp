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

#include "Editor/Graphics/GizmoRenderer.hpp"
#include "Editor/CommonEditor.hpp"
#include "Editor/Editor.hpp"
#include "Editor/Graphics/MousePickRenderer.hpp"
#include "Editor/Graphics/EditorGfxHelpers.hpp"
#include "Common/Math/Math.hpp"
#include "Common/System/SystemInfo.hpp"
#include "Core/Resources/ResourceManager.hpp"
#include "Core/Graphics/Utility/GfxHelpers.hpp"
#include "Core/Graphics/Resource/Shader.hpp"
#include "Core/Graphics/Resource/Texture.hpp"
#include "Core/Graphics/Resource/TextureSampler.hpp"
#include "Core/Graphics/Resource/Font.hpp"
#include "Core/Graphics/Resource/Material.hpp"
#include "Core/Application.hpp"
#include "Core/Graphics/Pipeline/RenderPass.hpp"
#include "Core/Graphics/Renderers/DrawCollector.hpp"
#include "Core/Graphics/Renderers/WorldRenderer.hpp"
#include "Core/World/Components/CompLight.hpp"
#include "Core/World/Entity.hpp"
#include "Core/World/EntityWorld.hpp"

namespace Lina::Editor
{

#define UID_LIGHT_ICON UINT32_MAX - 1000

#ifdef LINA_DEBUG
#define DEBUG_LABEL_BEGIN(Stream, LABEL)                                                                                                                                                                                                                           \
	{                                                                                                                                                                                                                                                              \
		LinaGX::CMDDebugBeginLabel* debug = Stream->AddCommand<LinaGX::CMDDebugBeginLabel>();                                                                                                                                                                      \
		debug->label					  = LABEL;                                                                                                                                                                                                                 \
	}
#define DEBUG_LABEL_END(Stream)                                                                                                                                                                                                                                    \
	{                                                                                                                                                                                                                                                              \
		Stream->AddCommand<LinaGX::CMDDebugEndLabel>();                                                                                                                                                                                                            \
	}
#else
#define DEBUG_LABEL_BEGIN(Stream, LABEL)
#define DEBUG_LABEL_END(Stream)
#endif

#define ORIENTATION_PASS_SIZE 250

	GizmoRenderer::GizmoRenderer(Editor* editor, WorldRenderer* wr, RenderPass* pass, MousePickRenderer* mpr)
	{
		m_editor			= editor;
		m_worldRenderer		= wr;
		m_rm				= &m_editor->GetApp()->GetResourceManager();
		m_gizmoShader		= m_rm->GetResource<Shader>(EDITOR_SHADER_WORLD_GIZMO_ID);
		m_gizmoRotateShader = m_rm->GetResource<Shader>(EDITOR_SHADER_WORLD_GIZMO_ROTATE_ID);
		m_line3DShader		= m_rm->GetResource<Shader>(EDITOR_SHADER_WORLD_LINE3D_ID);
		m_lvgShader			= m_rm->GetResource<Shader>(EDITOR_SHADER_WORLD_LVG3D_ID);
        m_billboardSDFShader = m_rm->GetResource<Shader>(EDITOR_SHADER_WORLD_BILLBOARD_SDF_ID);
		m_orientGizmoShader = m_rm->GetResource<Shader>(EDITOR_SHADER_WORLD_ORIENT_GIZMO_ID);
        m_iconFont = m_rm->GetResource<Font>(EDITOR_FONT_ICON_ID);
		m_world				= m_worldRenderer->GetWorld();
		m_targetPass		= pass;
		m_mousePickRenderer = mpr;

		m_gizmoMaterialCenter = m_rm->CreateResource<Material>(m_rm->ConsumeResourceID(), "Gizmo Material Center");
		m_gizmoMaterialX	  = m_rm->CreateResource<Material>(m_rm->ConsumeResourceID(), "Gizmo Material X");
		m_gizmoMaterialY	  = m_rm->CreateResource<Material>(m_rm->ConsumeResourceID(), "Gizmo Material Y");
		m_gizmoMaterialZ	  = m_rm->CreateResource<Material>(m_rm->ConsumeResourceID(), "Gizmo Material Z");
		m_gizmoRotateMaterial = m_rm->CreateResource<Material>(m_rm->ConsumeResourceID(), "Gizmo Rotate Material");
        m_matLightIconSpot = m_rm->CreateResource<Material>(m_rm->ConsumeResourceID(), "SDF Icon Mat SpotLight");
        m_matLightIconSun = m_rm->CreateResource<Material>(m_rm->ConsumeResourceID(), "SDF Icon Mat SunLight");
        m_matLightIconPoint = m_rm->CreateResource<Material>(m_rm->ConsumeResourceID(), "SDF Icon Mat PointLight");
		m_editor->GetApp()->GetGfxContext().MarkBindlessDirty();

		m_gizmoMaterialCenter->SetShader(m_gizmoShader);
		m_gizmoMaterialX->SetShader(m_gizmoShader);
		m_gizmoMaterialY->SetShader(m_gizmoShader);
		m_gizmoMaterialZ->SetShader(m_gizmoShader);
		m_gizmoRotateMaterial->SetShader(m_gizmoRotateShader);
        m_matLightIconSpot->SetShader(m_billboardSDFShader);
        m_matLightIconPoint->SetShader(m_billboardSDFShader);
        m_matLightIconSun->SetShader(m_billboardSDFShader);

		m_gizmoMaterialCenter->SetProperty("color"_hs, Vector4(Theme::GetDef().foreground0));
		m_gizmoMaterialX->SetProperty("color"_hs, Vector4(Theme::GetDef().accentPrimary2));
		m_gizmoMaterialY->SetProperty("color"_hs, Vector4(Theme::GetDef().accentSuccess));
		m_gizmoMaterialZ->SetProperty("color"_hs, Vector4(Theme::GetDef().accentSecondary));
        
        LinaVG::Font* lvgFont = m_iconFont->GetFont(m_gizmoSettings.defaultShaderScale);
        LinaVG::TextCharacter& glyphPoint = lvgFont->glyphs.at(0x0031);
        LinaVG::TextCharacter& glyphSpot = lvgFont->glyphs.at(0x0032);
        LinaVG::TextCharacter& glyphSun = lvgFont->glyphs.at(0x0033);
        
        m_iconSzSpot =  Vector2(glyphSpot.m_uv34.x, glyphSpot.m_uv34.y) - Vector2(glyphSpot.m_uv12.x, glyphSpot.m_uv12.y);
        m_iconSzSun = Vector2(glyphSun.m_uv34.x, glyphSun.m_uv34.y) - Vector2(glyphSun.m_uv12.x, glyphSun.m_uv12.y);
        m_iconSzPoint = Vector2(glyphPoint.m_uv34.x, glyphPoint.m_uv34.y) - Vector2(glyphPoint.m_uv12.x, glyphPoint.m_uv12.y);
        
        Texture* ft = m_editor->GetApp()->GetGUIBackend().GetFontTexture(lvgFont->atlas).texture;
        const LinaTexture2D txt = {.texture = ft->GetID(), .sampler = m_editor->GetEditorRenderer().GetGUITextSampler()->GetID(),};
        
        m_matLightIconSpot->SetProperty("diffuse"_hs, txt);
        m_matLightIconSpot->SetProperty("color"_hs, Vector4(Theme::GetDef().accentYellowGold));
        m_matLightIconSpot->SetProperty("thickness"_hs, 0.5f);
        m_matLightIconSpot->SetProperty("outlineThickness"_hs, 0.0f);
        m_matLightIconSpot->SetProperty("softness"_hs, 0.02f);
        m_matLightIconSpot->SetProperty("outlineSoftness"_hs, 0.02f);
        m_matLightIconSpot->SetProperty("uvStart"_hs, Vector2(glyphSpot.m_uv12.x, glyphSpot.m_uv12.y));
        m_matLightIconSpot->SetProperty("uvSize"_hs, m_iconSzSpot);
        
        m_matLightIconPoint->SetProperty("diffuse"_hs, txt);
        m_matLightIconPoint->SetProperty("color"_hs, Vector4(Theme::GetDef().accentYellowGold));
        m_matLightIconPoint->SetProperty("thickness"_hs, 0.5f);
        m_matLightIconPoint->SetProperty("outlineThickness"_hs, 0.0f);
        m_matLightIconPoint->SetProperty("softness"_hs, 0.02f);
        m_matLightIconPoint->SetProperty("outlineSoftness"_hs, 0.02f);
        m_matLightIconPoint->SetProperty("uvStart"_hs, Vector2(glyphPoint.m_uv12.x, glyphPoint.m_uv12.y));
        m_matLightIconPoint->SetProperty("uvSize"_hs, m_iconSzPoint);
        
        m_matLightIconSun->SetProperty("diffuse"_hs, txt);
        m_matLightIconSun->SetProperty("color"_hs, Vector4(Theme::GetDef().accentYellowGold));
        m_matLightIconSun->SetProperty("thickness"_hs, 0.5f);
        m_matLightIconSun->SetProperty("outlineThickness"_hs, 0.0f);
        m_matLightIconSun->SetProperty("softness"_hs, 0.02f);
        m_matLightIconSun->SetProperty("outlineSoftness"_hs, 0.02f);
        m_matLightIconSun->SetProperty("uvStart"_hs, Vector2(glyphSun.m_uv12.x, glyphSun.m_uv12.y));
        m_matLightIconSun->SetProperty("uvSize"_hs, m_iconSzSun);

		m_translateModel  = m_rm->GetResource<Model>(EDITOR_MODEL_GIZMO_TRANSLATE_ID);
		m_rotateModel	  = m_rm->GetResource<Model>(EDITOR_MODEL_GIZMO_ROTATE_ID);
		m_rotateFullModel = m_rm->GetResource<Model>(EDITOR_MODEL_GIZMO_ROTATE_FULL_ID);
		m_scaleModel	  = m_rm->GetResource<Model>(EDITOR_MODEL_GIZMO_SCALE_ID);
		m_centerTranslate = m_rm->GetResource<Model>(EDITOR_MODEL_GIZMO_TRANSLATE_CENTER_ID);
		m_centerScale	  = m_rm->GetResource<Model>(EDITOR_MODEL_GIZMO_SCALE_CENTER_ID);
		m_orientGizmo	  = m_rm->GetResource<Model>(EDITOR_MODEL_GIZMO_ORIENTATION_ID);
        m_quadModel = m_rm->GetResource<Model>(EDITOR_MODEL_QUAD_ID);

	} // namespace Lina::Editor

	GizmoRenderer::~GizmoRenderer()
	{
		m_rm->DestroyResource(m_gizmoMaterialX);
		m_rm->DestroyResource(m_gizmoMaterialY);
        m_rm->DestroyResource(m_gizmoMaterialZ);
        m_rm->DestroyResource(m_gizmoMaterialCenter);
        m_rm->DestroyResource(m_gizmoRotateMaterial);
        m_rm->DestroyResource(m_matLightIconSpot);
        m_rm->DestroyResource(m_matLightIconPoint);
        m_rm->DestroyResource(m_matLightIconSun);
		m_editor->GetApp()->GetGfxContext().MarkBindlessDirty();
	}

	void GizmoRenderer::Tick(float delta)
	{
        DrawLightWireframes();
        DrawLightIcons(m_targetPass, 0);
        DrawLightIcons(&m_mousePickRenderer->GetRenderPass(), "StaticEntityID"_hs);
        
		if (m_gizmoSettings.drawOrientation)
		{
            DrawOrientationGizmos(m_targetPass, 0, m_gizmoSettings.defaultShaderScale);
			DrawOrientationGizmos(&m_mousePickRenderer->GetRenderPass(), "StaticEntityID"_hs, m_gizmoSettings.defaultShaderScale * 2);
		}

		if (!m_gizmoSettings.draw)
			return;

		if (m_gizmoSettings.type != GizmoMode::Rotate)
		{
			DrawGizmoMoveScale(m_targetPass, 0, m_gizmoSettings.defaultShaderScale);
			DrawGizmoMoveScale(&m_mousePickRenderer->GetRenderPass(), "StaticEntityID"_hs, m_gizmoSettings.defaultShaderScale * 2);
		}
		else
		{
			if (m_gizmoSettings.focusedAxis == GizmoAxis::None)
			{
				DrawGizmoRotate(m_targetPass, 0, m_gizmoSettings.defaultShaderScale);
				DrawGizmoRotate(&m_mousePickRenderer->GetRenderPass(), "StaticEntityID"_hs, m_gizmoSettings.defaultShaderScale * 1);
			}
			else
			{
				DrawGizmoRotateFocus(m_targetPass);
			}
		}

		if (m_gizmoSettings.visualizeAxis)
		{
			Camera& cam = m_world->GetWorldCamera();
			m_worldRenderer->StartLine3DBatch();
            m_worldRenderer->DrawLine3D(m_gizmoSettings.position + m_gizmoSettings.worldAxis * -cam.GetZFar() * 0.5f, m_gizmoSettings.position + m_gizmoSettings.worldAxis * cam.GetZFar() * 0.5f, 0.08f * m_gizmoSettings.defaultShaderScale, GetColorFromAxis(m_gizmoSettings.focusedAxis));
			m_worldRenderer->EndLine3DBatch(*m_targetPass, 0, m_line3DShader->GetGPUHandle());
		}
	}

	Color GizmoRenderer::GetColorFromAxis(GizmoAxis axis)
	{
		if (axis == GizmoAxis::X)
			return Theme::GetDef().accentPrimary2.AsLVG4();
		else if (axis == GizmoAxis::Y)
			return Theme::GetDef().accentSuccess.AsLVG4();
		if (axis == GizmoAxis::Z)
			return Theme::GetDef().accentSecondary.AsLVG4();
		return Color::White;
	}

    void GizmoRenderer::DrawLightWireframes()
    {
        const Vector<CompLight*>& compLights = m_worldRenderer->GetCompLights();
        m_worldRenderer->StartLine3DBatch();
        
        const float thickness = 0.1f * m_gizmoSettings.defaultShaderScale;
        
        for(CompLight* l : compLights)
        {
            const LightType type = l->GetType();
            Entity* e = l->GetEntity();
            
            auto it = linatl::find_if(m_selectedEntities.begin(), m_selectedEntities.end(), [e](Entity* selected) -> bool {return selected == e;});
            if(it == m_selectedEntities.end())
                continue;
            
            const Vector3 p = e->GetPosition();

            if(type == LightType::Directional)
            {
                m_worldRenderer->DrawLine3D(p, p + e->GetRotation().GetForward() * 2.0f, thickness, Theme::GetDef().accentYellowGold);
            }
            else if(type == LightType::Point)
            {
                m_worldRenderer->DrawWireSphere3D(e->GetPosition(), l->GetRadius(), thickness, Theme::GetDef().accentYellowGold);
                m_worldRenderer->DrawWireSphere3D(e->GetPosition(), l->GetFalloff(), thickness, Theme::GetDef().accentPrimary3);
            }
            else if(type == LightType::Spot)
            {
                const float radius = l->GetRadius() * Math::Tan(DEG_2_RAD * l->GetCutoff());
                const float radiusOuter = l->GetRadius() * Math::Tan(DEG_2_RAD * l->GetOuterCutoff());
                m_worldRenderer->DrawWireCone3D(p, e->GetRotation().GetForward(), l->GetRadius(), radius, thickness, Theme::GetDef().accentYellowGold, true);
                m_worldRenderer->DrawWireCone3D(p, e->GetRotation().GetForward(), l->GetRadius(), radiusOuter, thickness, Theme::GetDef().accentPrimary3, false);
            }
        }
        
        m_worldRenderer->EndLine3DBatch(*m_targetPass, 0, m_line3DShader->GetGPUHandle());
    }

    void GizmoRenderer::DrawLightIcons(RenderPass* pass, StringID variant)
    {
        const Vector<CompLight*>& lights = m_worldRenderer->GetCompLights();
        if(lights.empty())
            return;
        
        const PrimitiveStatic& prim          = m_quadModel->GetAllMeshes().at(0).primitivesStatic.at(0);
        const uint32           baseVertex = prim._vertexOffset;
        const uint32           baseIndex  = prim._indexOffset;
        const uint32           indexCount = static_cast<uint32>(prim.indices.size());
    
        uint32 pc = 0;
        uint32 instanceCount = 0;
    
        for(CompLight* l : lights)
        {
            const LightType type = l->GetType();
            Entity* e = l->GetEntity();
            
            uint32 matIndex = 0;
            Vector3 scale = Vector3::One;
            
            if(type == LightType::Directional)
            {
                matIndex = m_matLightIconSun->GetBindlessIndex();
                scale = m_iconSzSun.Normalized() * 0.5f;
            }
            else if(type == LightType::Spot)
            {
                matIndex = m_matLightIconSpot->GetBindlessIndex();
                scale = m_iconSzSpot.Normalized() * 0.5f;
            }
            else{
                matIndex = m_matLightIconPoint->GetBindlessIndex();
                scale = m_iconSzPoint.Normalized() * 0.5f;
            }
      
            const GPUEntity entity = {
                .model = Matrix4::TransformMatrix(e->GetPosition(), Quaternion::LookAt(e->GetPosition(), m_world->GetWorldCamera().GetPosition(), -Vector3::Up), scale),
            };
            
            const uint32 entityIndex =  m_worldRenderer->PushEntity(entity, {
                .entityGUID = e->GetGUID(),
                .uniqueID2 = UID_LIGHT_ICON,
            });
            
            const GPUDrawArguments args = {
                .constant0 = entityIndex,
                .constant1 = matIndex,
            };
    
            const uint32 argIndex = m_worldRenderer->PushArgument(args);
    
            if(instanceCount == 0)
            {
                pc = argIndex;
            }
            
            instanceCount++;
        }
        
        Buffer* vtx = &m_editor->GetApp()->GetGfxContext().GetMeshManagerDefault().GetVtxBufferStatic();
        Buffer* idx = &m_editor->GetApp()->GetGfxContext().GetMeshManagerDefault().GetIdxBufferStatic();
    
        const RenderPass::InstancedDraw draw = {
            .vertexBuffers = {vtx, vtx},
            .indexBuffers  = {idx, idx},
            .vertexSize       = sizeof(VertexStatic),
            .shaderHandle  = variant == 0 ? m_billboardSDFShader->GetGPUHandle() : m_billboardSDFShader->GetGPUHandle(variant),
            .baseVertex       = baseVertex,
            .baseIndex       = baseIndex,
            .indexCount       = indexCount,
            .instanceCount = instanceCount,
            .pushConstant  = pc,
        };
        
        pass->AddDrawCall(draw);
    }

	void GizmoRenderer::DrawGizmoMoveScale(RenderPass* pass, StringID variant, float shaderScale)
	{
		Model*				   model	  = m_gizmoSettings.type == GizmoMode::Move ? m_translateModel : m_scaleModel;
		const PrimitiveStatic& prim		  = model->GetAllMeshes().at(0).primitivesStatic.at(0);
		const uint32		   baseVertex = prim._vertexOffset;
		const uint32		   baseIndex  = prim._indexOffset;
		const uint32		   indexCount = static_cast<uint32>(prim.indices.size());

		const GPUEntity axisCenter = {
			.model = Matrix4::TransformMatrix(m_gizmoSettings.position, Quaternion::LookAt(Vector3::Zero, m_gizmoSettings.rotation.GetForward(), m_gizmoSettings.rotation.GetUp()), Vector3(0.06f)),
		};

		const GPUEntity axisX = {
			.model = Matrix4::TransformMatrix(m_gizmoSettings.position, Quaternion::LookAt(Vector3::Zero, m_gizmoSettings.rotation.GetRight(), m_gizmoSettings.rotation.GetUp()), Vector3::One),
		};

		const GPUEntity axisY = {
			.model = Matrix4::TransformMatrix(m_gizmoSettings.position, Quaternion::LookAt(Vector3::Zero, m_gizmoSettings.rotation.GetUp(), m_gizmoSettings.rotation.GetForward()), Vector3::One),
		};

		const GPUEntity axisZ = {
			.model = Matrix4::TransformMatrix(m_gizmoSettings.position, Quaternion::LookAt(Vector3::Zero, m_gizmoSettings.rotation.GetForward(), m_gizmoSettings.rotation.GetUp()), Vector3::One),
		};

		const GPUDrawArguments argsCenter = {
			.constant0 = m_worldRenderer->PushEntity(axisCenter, {.entityGUID = GIZMO_GUID_CENTER_AXIS}),
			.constant1 = m_gizmoMaterialCenter->GetBindlessIndex(),
			.constant2 = m_gizmoSettings.hoveredEntityID == GIZMO_GUID_CENTER_AXIS,
			.constant3 = static_cast<uint32>(shaderScale), // scale
		};

		const GPUDrawArguments argsX = {
			.constant0 = m_worldRenderer->PushEntity(axisX, {.entityGUID = GIZMO_GUID_X_AXIS}),
			.constant1 = m_gizmoMaterialX->GetBindlessIndex(),
			.constant2 = m_gizmoSettings.hoveredEntityID == GIZMO_GUID_X_AXIS,
			.constant3 = static_cast<uint32>(shaderScale), // scale
		};

		const GPUDrawArguments argsY = {
			.constant0 = m_worldRenderer->PushEntity(axisY, {.entityGUID = GIZMO_GUID_Y_AXIS}),
			.constant1 = m_gizmoMaterialY->GetBindlessIndex(),
			.constant2 = m_gizmoSettings.hoveredEntityID == GIZMO_GUID_Y_AXIS,
			.constant3 = static_cast<uint32>(shaderScale), // scale
		};

		const GPUDrawArguments argsZ = {
			.constant0 = m_worldRenderer->PushEntity(axisZ, {.entityGUID = GIZMO_GUID_Z_AXIS}),
			.constant1 = m_gizmoMaterialZ->GetBindlessIndex(),
			.constant2 = m_gizmoSettings.hoveredEntityID == GIZMO_GUID_Z_AXIS,
			.constant3 = static_cast<uint32>(shaderScale), // scale
		};

		Buffer* vtx = &m_editor->GetApp()->GetGfxContext().GetMeshManagerDefault().GetVtxBufferStatic();
		Buffer* idx = &m_editor->GetApp()->GetGfxContext().GetMeshManagerDefault().GetIdxBufferStatic();

		const uint32 pc = m_worldRenderer->PushArgument(argsX);
		m_worldRenderer->PushArgument(argsY);
		m_worldRenderer->PushArgument(argsZ);
        
		const RenderPass::InstancedDraw draw = {
			.vertexBuffers = {vtx, vtx},
			.indexBuffers  = {idx, idx},
			.vertexSize	   = sizeof(VertexStatic),
			.shaderHandle  = variant == 0 ? m_gizmoShader->GetGPUHandle() : m_gizmoShader->GetGPUHandle(variant),
			.baseVertex	   = baseVertex,
			.baseIndex	   = baseIndex,
			.indexCount	   = indexCount,
			.instanceCount = 3,
			.pushConstant  = pc,
		};
		pass->AddDrawCall(draw);

		// central
		{
			Model*							centerModel = m_gizmoSettings.type == GizmoMode::Move ? m_centerTranslate : m_centerScale;
			const PrimitiveStatic&			prim		= centerModel->GetAllMeshes().at(0).primitivesStatic.at(0);
			const uint32					pc			= m_worldRenderer->PushArgument(argsCenter);
			const RenderPass::InstancedDraw draw		= {
					   .vertexBuffers = {vtx, vtx},
					   .indexBuffers  = {idx, idx},
					   .vertexSize	  = sizeof(VertexStatic),
					   .shaderHandle  = variant == 0 ? m_gizmoShader->GetGPUHandle() : m_gizmoShader->GetGPUHandle(variant),
					   .baseVertex	  = prim._vertexOffset,
					   .baseIndex	  = prim._indexOffset,
					   .indexCount	  = static_cast<uint32>(prim.indices.size()),
					   .instanceCount = 1,
					   .pushConstant  = pc,
			   };
			pass->AddDrawCall(draw);
		}
	}

	void GizmoRenderer::DrawGizmoRotate(RenderPass* pass, StringID variant, float shaderScale)
	{
		Model*				   model	  = m_rotateModel;
		const PrimitiveStatic& prim		  = model->GetAllMeshes().at(0).primitivesStatic.at(0);
		const uint32		   baseVertex = prim._vertexOffset;
		const uint32		   baseIndex  = prim._indexOffset;
		const uint32		   indexCount = static_cast<uint32>(prim.indices.size());

		Camera&		  cam  = m_world->GetWorldCamera();
		const Vector3 dir  = (cam.GetPosition() - m_gizmoSettings.position);
		const float	  dot  = dir.Normalized().Dot(m_gizmoSettings.rotation.GetForward());
		const float	  dotR = dir.Normalized().Dot(m_gizmoSettings.rotation.GetRight());
		const float	  dotU = dir.Normalized().Dot(m_gizmoSettings.rotation.GetUp());

		const GPUEntity axisX = {
			.model = Matrix4::TransformMatrix(
				m_gizmoSettings.position, Quaternion::LookAt(Vector3::Zero, -m_gizmoSettings.rotation.GetRight() * (dot > 0.0f ? 1.0f : -1.0f) * (dotU > 0.0f ? 1.0f : -1.0f), m_gizmoSettings.rotation.GetUp() * (dotU > 0.0f ? 1.0f : -1.0f)), Vector3::One * 2),
		};

		const GPUEntity axisY = {
			.model = Matrix4::TransformMatrix(
				m_gizmoSettings.position, Quaternion::LookAt(Vector3::Zero, m_gizmoSettings.rotation.GetUp() * (dotR > 0.0f ? -1.0f : 1.0f) * (dot > 0.0f ? 1.0f : -1.0f), m_gizmoSettings.rotation.GetForward() * (dot > 0.0f ? 1.0f : -1.0f)), Vector3::One * 2),
		};

		const GPUEntity axisZ = {
			.model = Matrix4::TransformMatrix(m_gizmoSettings.position,
											  Quaternion::LookAt(Vector3::Zero, -m_gizmoSettings.rotation.GetForward() * (dotR > 0.0f ? -1.0f : 1.0f) * (dotU > 0.0f ? 1.0f : -1.0f), m_gizmoSettings.rotation.GetUp() * (dotU > 0.0f ? 1.0f : -1.0f)),
											  Vector3::One * 2),
		};
		const GPUDrawArguments argsX = {
			.constant0 = m_worldRenderer->PushEntity(axisX, {.entityGUID = GIZMO_GUID_X_AXIS}),
			.constant1 = m_gizmoMaterialX->GetBindlessIndex(),
			.constant2 = m_gizmoSettings.hoveredEntityID == GIZMO_GUID_X_AXIS,
			.constant3 = static_cast<uint32>(shaderScale), // scale
		};

		const GPUDrawArguments argsY = {
			.constant0 = m_worldRenderer->PushEntity(axisY, {.entityGUID = GIZMO_GUID_Y_AXIS}),
			.constant1 = m_gizmoMaterialY->GetBindlessIndex(),
			.constant2 = m_gizmoSettings.hoveredEntityID == GIZMO_GUID_Y_AXIS,
			.constant3 = static_cast<uint32>(shaderScale), // scale
		};

		const GPUDrawArguments argsZ = {
			.constant0 = m_worldRenderer->PushEntity(axisZ, {.entityGUID = GIZMO_GUID_Z_AXIS}),
			.constant1 = m_gizmoMaterialZ->GetBindlessIndex(),
			.constant2 = m_gizmoSettings.hoveredEntityID == GIZMO_GUID_Z_AXIS,
			.constant3 = static_cast<uint32>(shaderScale), // scale
		};

		Buffer* vtx = &m_editor->GetApp()->GetGfxContext().GetMeshManagerDefault().GetVtxBufferStatic();
		Buffer* idx = &m_editor->GetApp()->GetGfxContext().GetMeshManagerDefault().GetIdxBufferStatic();

		const uint32 pc = m_worldRenderer->PushArgument(argsX);
		m_worldRenderer->PushArgument(argsY);
		m_worldRenderer->PushArgument(argsZ);
		const RenderPass::InstancedDraw draw = {
			.vertexBuffers = {vtx, vtx},
			.indexBuffers  = {idx, idx},
			.vertexSize	   = sizeof(VertexStatic),
			.shaderHandle  = variant == 0 ? m_gizmoShader->GetGPUHandle() : m_gizmoShader->GetGPUHandle(variant),
			.baseVertex	   = baseVertex,
			.baseIndex	   = baseIndex,
			.indexCount	   = indexCount,
			.instanceCount = 3,
			.pushConstant  = pc,
		};
		pass->AddDrawCall(draw);
	}

	void GizmoRenderer::DrawGizmoRotateFocus(RenderPass* pass, float shaderScale)
	{
		Camera& cam = m_world->GetWorldCamera();

		m_worldRenderer->StartLinaVGBatch();

		Model*				   model	  = m_rotateFullModel;
		const PrimitiveStatic& prim		  = model->GetAllMeshes().at(0).primitivesStatic.at(0);
		const uint32		   baseVertex = prim._vertexOffset;
		const uint32		   baseIndex  = prim._indexOffset;
		const uint32		   indexCount = static_cast<uint32>(prim.indices.size());

		const GPUEntity e = {
			.model = Matrix4::TransformMatrix(m_gizmoSettings.position, Quaternion::Rotation(Vector3::Forward, m_gizmoSettings.worldAxis), Vector3(2.0f)),
		};

		Color col = Vector4(GetColorFromAxis(m_gizmoSettings.focusedAxis));
		col.w	  = 0.5f;

		m_gizmoRotateMaterial->SetProperty("color"_hs, col);
		m_gizmoRotateMaterial->SetProperty("angle0"_hs, m_gizmoSettings.angle0);
		m_gizmoRotateMaterial->SetProperty("angle1"_hs, m_gizmoSettings.angle1);
		m_editor->GetApp()->GetGfxContext().MarkBindlessDirty();

		const GPUDrawArguments args = {
			.constant0 = m_worldRenderer->PushEntity(e, {}),
			.constant1 = m_gizmoRotateMaterial->GetBindlessIndex(),
			.constant3 = static_cast<uint32>(shaderScale), // scale
		};
		const uint32 pc = m_worldRenderer->PushArgument(args);

		Buffer* vtx = &m_editor->GetApp()->GetGfxContext().GetMeshManagerDefault().GetVtxBufferStatic();
		Buffer* idx = &m_editor->GetApp()->GetGfxContext().GetMeshManagerDefault().GetIdxBufferStatic();

		const RenderPass::InstancedDraw draw = {
			.vertexBuffers = {vtx, vtx},
			.indexBuffers  = {idx, idx},
			.vertexSize	   = sizeof(VertexStatic),
			.shaderHandle  = m_gizmoRotateShader->GetGPUHandle(),
			.baseVertex	   = baseVertex,
			.baseIndex	   = baseIndex,
			.indexCount	   = indexCount,
			.instanceCount = 1,
			.pushConstant  = pc,
		};
		pass->AddDrawCall(draw);
	}

	void GizmoRenderer::DrawOrientationGizmos(RenderPass* pass, StringID variant, float shaderScale)
	{
		Camera& cam = m_world->GetWorldCamera();

		Model*				   model	  = m_orientGizmo;
		const PrimitiveStatic& prim		  = model->GetAllMeshes().at(0).primitivesStatic.at(0);
		const uint32		   baseVertex = prim._vertexOffset;
		const uint32		   baseIndex  = prim._indexOffset;
		const uint32		   indexCount = static_cast<uint32>(prim.indices.size());

		Matrix4 modelMatX = Matrix4::TransformMatrix(Vector3::Zero, Quaternion::LookAt(Vector3::Zero, Vector3::Right, Vector3::Up), Vector3::One * 0.5f);
		Matrix4 modelMatY = Matrix4::TransformMatrix(Vector3::Zero, Quaternion::LookAt(Vector3::Zero, Vector3::Up, Vector3::Forward), Vector3::One * 0.5f);
		Matrix4 modelMatZ = Matrix4::TransformMatrix(Vector3::Zero, Quaternion::LookAt(Vector3::Zero, Vector3::Forward, Vector3::Up), Vector3::One * 0.5f);

		const Vector2 sz		  = m_world->GetScreen().GetRenderSize();
		const float	  aspect	  = sz.x / sz.y;
		const Matrix4 projection  = glm::ortho(-(1.0f * aspect), 1.0f * aspect, -1.0f, 1.0f, 0.1f, 10.0f);
		const Matrix4 rot		  = cam.GetRotation().Inverse();
		const Matrix4 translation = Matrix4::Translate(cam.GetRotation().GetForward());
		const Matrix4 view		  = rot * translation;

		const GPUEntity ex = {
			.model = projection * view * modelMatX,
		};
		const GPUEntity ey = {
			.model = projection * view * modelMatY,
		};

		const GPUEntity ez = {
			.model = projection * view * modelMatZ,
		};

		const GPUDrawArguments argsX = {
			.constant0 = m_worldRenderer->PushEntity(ex, {.entityGUID = GIZMO_ORIENTATION_X}),
			.constant1 = m_gizmoMaterialX->GetBindlessIndex(),
			.constant2 = m_gizmoSettings.hoveredEntityID == GIZMO_ORIENTATION_X,
			.constant3 = static_cast<uint32>(shaderScale), // scale
		};

		const GPUDrawArguments argsY = {
			.constant0 = m_worldRenderer->PushEntity(ey, {.entityGUID = GIZMO_ORIENTATION_Y}),
			.constant1 = m_gizmoMaterialY->GetBindlessIndex(),
			.constant2 = m_gizmoSettings.hoveredEntityID == GIZMO_ORIENTATION_Y,
			.constant3 = static_cast<uint32>(shaderScale), // scale
		};

		const GPUDrawArguments argsZ = {
			.constant0 = m_worldRenderer->PushEntity(ez, {.entityGUID = GIZMO_ORIENTATION_Z}),
			.constant1 = m_gizmoMaterialZ->GetBindlessIndex(),
			.constant2 = m_gizmoSettings.hoveredEntityID == GIZMO_ORIENTATION_Z,
			.constant3 = static_cast<uint32>(shaderScale), // scale
		};

		const uint32 pc = m_worldRenderer->PushArgument(argsX);
		m_worldRenderer->PushArgument(argsY);
		m_worldRenderer->PushArgument(argsZ);

		Buffer* vtx = &m_editor->GetApp()->GetGfxContext().GetMeshManagerDefault().GetVtxBufferStatic();
		Buffer* idx = &m_editor->GetApp()->GetGfxContext().GetMeshManagerDefault().GetIdxBufferStatic();

		const RenderPass::InstancedDraw draw = {
			.vertexBuffers = {vtx, vtx},
			.indexBuffers  = {idx, idx},
			.vertexSize	   = sizeof(VertexStatic),
			.shaderHandle  = variant == 0 ? m_orientGizmoShader->GetGPUHandle() : m_orientGizmoShader->GetGPUHandle(variant),
			.baseVertex	   = baseVertex,
			.baseIndex	   = baseIndex,
			.indexCount	   = indexCount,
			.instanceCount = 3,
			.pushConstant  = pc,
		};

		pass->AddDrawCall(draw);
	}

} // namespace Lina::Editor
