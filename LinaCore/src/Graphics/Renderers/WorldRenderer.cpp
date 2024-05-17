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

#include "Core/Graphics/Renderers/WorldRenderer.hpp"
#include "Core/Graphics/GfxManager.hpp"
#include "Core/Graphics/Utility/GfxHelpers.hpp"
#include "Core/Graphics/Resource/Model.hpp"
#include "Core/Graphics/Resource/Shader.hpp"
#include "Core/Graphics/Resource/Material.hpp"
#include "Core/Graphics/Resource/Texture.hpp"
#include "Core/Graphics/Data/ModelNode.hpp"
#include "Core/Graphics/Data/Mesh.hpp"
#include "Core/World/EntityWorld.hpp"
#include "Core/Resources/ResourceManager.hpp"
#include "Core/Components/MeshComponent.hpp"
#include "Core/Components/CameraComponent.hpp"
#include "Common/Platform/LinaGXIncl.hpp"
#include "Common/System/System.hpp"
#include "Common/Profiling/Profiler.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <Common/Math/Math.hpp>
#include <LinaGX/Core/InputMappings.hpp>

namespace Lina
{
#define MAX_GFX_COMMANDS  250
#define MAX_COPY_COMMANDS 50
#define MAX_GUI_VERTICES  5000
#define MAX_GUI_INDICES	  5000
#define MAX_GUI_MATERIALS 100
#define MAX_OBJECTS		  256

	WorldRenderer::WorldRenderer(GfxManager* man, EntityWorld* world, const Vector2ui& viewSize)
	{
		m_world		 = world;
		m_gfxManager = man;
		m_lgx		 = m_gfxManager->GetLGX();
		m_world->AddListener(this);
		m_size					 = viewSize;
		m_rm					 = m_gfxManager->GetSystem()->CastSubsystem<ResourceManager>(SubsystemType::ResourceManager);
		m_gBufSampler			 = m_gfxManager->GetDefaultSampler(0);
		m_deferredLightingShader = m_rm->GetResource<Shader>(DEFAULT_SHADER_DEFERRED_LIGHTING_SID);
		m_checkerTexture		 = m_rm->GetResource<Texture>("Resources/Core/Textures/Checkered.png"_hs);
		m_skyCube				 = m_rm->GetResource<Model>("Resources/Core/Models/SkyCube.glb"_hs)->GetMesh(0);

		auto* rm = m_gfxManager->GetSystem()->CastSubsystem<ResourceManager>(SubsystemType::ResourceManager);

		for (uint32 i = 0; i < FRAMES_IN_FLIGHT; i++)
		{
			auto& data		= m_pfd[i];
			data.gfxStream	= m_lgx->CreateCommandStream({LinaGX::CommandType::Graphics, MAX_GFX_COMMANDS, 24000, 4096, 32, "WorldRenderer: Gfx Stream"});
			data.copyStream = m_lgx->CreateCommandStream({LinaGX::CommandType::Transfer, MAX_COPY_COMMANDS, 4000, 1024, 32, "WorldRenderer: Copy Stream"});
			data.objectBuffer.Create(m_lgx, LinaGX::ResourceTypeHint::TH_StorageBuffer, sizeof(GPUDataObject) * MAX_OBJECTS, "WorldRenderer: ObjectData", false);
			data.signalSemaphore.semaphore = m_lgx->CreateUserSemaphore();
			data.copySemaphore.semaphore   = m_lgx->CreateUserSemaphore();
		}

		CreateSizeRelativeResources();
		FetchRenderables();
	}

	WorldRenderer::~WorldRenderer()
	{
		for (uint32 i = 0; i < FRAMES_IN_FLIGHT; i++)
		{
			auto& data = m_pfd[i];
			m_lgx->DestroyCommandStream(data.gfxStream);
			m_lgx->DestroyCommandStream(data.copyStream);
			data.objectBuffer.Destroy();
			m_lgx->DestroyUserSemaphore(data.signalSemaphore.semaphore);
			m_lgx->DestroyUserSemaphore(data.copySemaphore.semaphore);
		}
		DestroySizeRelativeResources();
		m_world->RemoveListener(this);
	}

	void WorldRenderer::CreateSizeRelativeResources()
	{
		LinaGX::TextureDesc rtDesc = {
			.format	   = DEFAULT_RT_FORMAT,
			.flags	   = LinaGX::TF_ColorAttachment | LinaGX::TF_Sampled,
			.width	   = m_size.x,
			.height	   = m_size.y,
			.debugName = "WorldRendererTexture",
		};

		LinaGX::TextureDesc rtDescLighting = {
			.format	   = DEFAULT_RT_FORMAT_HDR,
			.flags	   = LinaGX::TF_ColorAttachment | LinaGX::TF_Sampled,
			.width	   = m_size.x,
			.height	   = m_size.y,
			.debugName = "WorldRendererTextureHDR",
		};

		LinaGX::TextureDesc depthDesc = {
			.format					  = LinaGX::Format::D32_SFLOAT,
			.depthStencilSampleFormat = LinaGX::Format::R32_SFLOAT,
			.flags					  = LinaGX::TF_DepthTexture | LinaGX::TF_Sampled,
			.width					  = m_size.x,
			.height					  = m_size.y,
			.debugName				  = "WorldRendererDepthTexture",
		};

		for (uint32 i = 0; i < FRAMES_IN_FLIGHT; i++)
		{
			auto& data = m_pfd[i];

			data.gBufColorMaterialID = m_rm->CreateUserResource<Texture>("WorldRenderer GBuffer: ColorMaterialID", TO_SIDC("WorldRenderer GBuffer: ColorMaterialID"));
			data.gBufPosition		 = m_rm->CreateUserResource<Texture>("WorldRenderer GBuffer: Position", TO_SIDC("WorldRenderer GBuffer: Position"));
			data.gBufNormal			 = m_rm->CreateUserResource<Texture>("WorldRenderer GBuffer: Normal", TO_SIDC("WorldRenderer GBuffer: Normal"));
			data.gBufDepth			 = m_rm->CreateUserResource<Texture>("WorldRenderer GBuffer: Depth", TO_SIDC("WorldRenderer GBuffer: Depth"));
			data.lightingPassOutput	 = m_rm->CreateUserResource<Texture>("WorldRenderer Lighting Pass Output", TO_SIDC("WorldRenderer Lighting Pass Output"));

			data.gBufColorMaterialID->CreateGPUOnly(rtDesc);
			data.gBufPosition->CreateGPUOnly(rtDesc);
			data.gBufNormal->CreateGPUOnly(rtDesc);
			data.gBufDepth->CreateGPUOnly(depthDesc);
			data.lightingPassOutput->CreateGPUOnly(rtDescLighting);

			m_mainPass.SetColorAttachment(i, 0, {.clearColor = {0.0f, 0.0f, 0.0f, 1.0f}, .texture = data.gBufColorMaterialID->GetGPUHandle(), .isSwapchain = false});
			m_mainPass.SetColorAttachment(i, 1, {.clearColor = {0.0f, 0.0f, 0.0f, 1.0f}, .texture = data.gBufPosition->GetGPUHandle(), .isSwapchain = false});
			m_mainPass.SetColorAttachment(i, 2, {.clearColor = {0.0f, 0.0f, 0.0f, 1.0f}, .texture = data.gBufNormal->GetGPUHandle(), .isSwapchain = false});
			m_mainPass.DepthStencilAttachment(i, {.useDepth = true, .texture = data.gBufDepth->GetGPUHandle(), .depthLoadOp = LinaGX::LoadOp::Clear, .depthStoreOp = LinaGX::StoreOp::Store, .clearDepth = 1.0f});

			m_lightingPass.SetColorAttachment(i, 0, {.clearColor = {0.0f, 0.0f, 0.0f, 1.0f}, .texture = data.lightingPassOutput->GetGPUHandle(), .isSwapchain = false});
			m_lightingPass.DepthStencilAttachment(i,
												  {
													  .useDepth		= true,
													  .texture		= data.gBufDepth->GetGPUHandle(),
													  .depthLoadOp	= LinaGX::LoadOp::Load,
													  .depthStoreOp = LinaGX::StoreOp::DontCare,
													  .clearDepth	= 1.0f,
												  });
		}
		m_mainPass.Create(m_gfxManager, RenderPassDescriptorType::Main);
		m_lightingPass.Create(m_gfxManager, RenderPassDescriptorType::Lighting);

		for (uint32 i = 0; i < FRAMES_IN_FLIGHT; i++)
		{
			auto& data = m_pfd[i];

			m_lgx->DescriptorUpdateBuffer({
				.setHandle			= m_mainPass.GetDescriptorSet(i),
				.setAllocationIndex = 0,
				.binding			= 1,
				.buffers			= {data.objectBuffer.GetGPUResource()},
			});
		}
	}

	void WorldRenderer::DestroySizeRelativeResources()
	{
		for (uint32 i = 0; i < FRAMES_IN_FLIGHT; i++)
		{
			auto& data = m_pfd[i];
			m_rm->DestroyUserResource(data.gBufColorMaterialID);
			m_rm->DestroyUserResource(data.gBufPosition);
			m_rm->DestroyUserResource(data.gBufNormal);
			m_rm->DestroyUserResource(data.gBufDepth);
			m_rm->DestroyUserResource(data.lightingPassOutput);
			// m_rm->DestroyUserResource(data.lightingPassDepth);
		}

		m_mainPass.Destroy();
		m_lightingPass.Destroy();
	}

	void WorldRenderer::FetchRenderables()
	{
		m_meshComponents.clear();
		m_world->GetAllComponents<MeshComponent>(m_meshComponents);
	}

	void WorldRenderer::OnComponentAdded(Component* c)
	{
		if (c->GetComponentType() == GetTypeID<MeshComponent>())
			m_meshComponents.push_back(static_cast<MeshComponent*>(c));
	}

	void WorldRenderer::OnComponentRemoved(Component* c)
	{
		if (c->GetComponentType() == GetTypeID<MeshComponent>())
			m_meshComponents.erase(linatl::find_if(m_meshComponents.begin(), m_meshComponents.end(), [c](MeshComponent* model) -> bool { return c == model; }));
	}

	void WorldRenderer::Tick(float delta)
	{
	}

	void WorldRenderer::Resize(const Vector2ui& newSize)
	{
		m_size = newSize;
		DestroySizeRelativeResources();
		CreateSizeRelativeResources();
	}

	void WorldRenderer::UpdateBuffers(uint32 frameIndex)
	{
		auto& currentFrame = m_pfd[frameIndex];

		CameraComponent* camera = m_world->GetActiveCamera();
		GPUDataView		 view	= {};

		if (camera == nullptr)
		{
			static float camX  = 25.0f;
			static float camY  = 25.0f;
			static float camZ  = 50.0f;
			const float	 speed = 30.0f;

			if (m_gfxManager->GetLGX()->GetInput().GetKey(LINAGX_KEY_A))
				camX -= 0.016f * speed;

			if (m_gfxManager->GetLGX()->GetInput().GetKey(LINAGX_KEY_D))
				camX += 0.016f * speed;

			if (m_gfxManager->GetLGX()->GetInput().GetKey(LINAGX_KEY_W))
				camY += 0.016f * speed;

			if (m_gfxManager->GetLGX()->GetInput().GetKey(LINAGX_KEY_S))
				camY -= 0.016f * speed;

			view.view	  = Matrix4::InitLookAt(Vector3(camX, camY, camZ), Vector3(0.0f, 0.0f, 0.0f), Vector3::Up);
			view.proj	  = Matrix4::Perspective(90, static_cast<float>(m_size.x) / static_cast<float>(m_size.y), 0.1f, 100.0f);
			view.viewProj = view.proj * view.view;
		}
		else
		{
			const Vector3& camPos	   = camera->GetEntity()->GetPosition();
			const Vector3& camDir	   = camera->GetEntity()->GetRotation().GetForward();
			view.view				   = camera->GetView();
			view.proj				   = camera->GetProjection();
			view.viewProj			   = view.proj * view.view;
			view.cameraPositionAndNear = Vector4(camPos.x, camPos.y, camPos.z, camera->GetNear());
			view.cameraDirectionAndFar = Vector4(camDir.x, camDir.y, camDir.z, camera->GetFar());
			view.size				   = Vector2(static_cast<float>(m_size.x), static_cast<float>(m_size.y));
		}

		m_lightingPass.GetBuffer(frameIndex, 0).BufferData(0, (uint8*)&view, sizeof(GPUDataView));
		m_mainPass.GetBuffer(frameIndex, 0).BufferData(0, (uint8*)&view, sizeof(GPUDataView));

		GPUDataAtmosphere atmosphere = {
			.skyTopAndDiffusion	   = Color(0.1f, 0.5f, 0.7f, 0.05f),
			.skyHorizonAndBase	   = Color(0.5f, 0.1f, 0.6f, 0.5f),
			.skyGroundAndCurvature = Color(0.01f, 0.01f, 0.01f, 0.05f),
			.sunLightAndCoef	   = Vector4(1.0f, 1.0f, 1.0f, 20.0f),
			.sunPosition		   = Vector4(25, 15, 100, 0),
		};

		m_lightingPass.GetBuffer(frameIndex, 1).BufferData(0, (uint8*)&atmosphere, sizeof(GPUDataAtmosphere));

		GPUDataDeferredLightingPass renderPassData = {
			.gBufColorMaterialID = currentFrame.gBufColorMaterialID->GetBindlessIndex(),
			.gBufPosition		 = currentFrame.gBufPosition->GetBindlessIndex(),
			.gBufNormal			 = currentFrame.gBufNormal->GetBindlessIndex(),
			.gBufDepth			 = currentFrame.gBufDepth->GetBindlessIndex(),
			.gBufSampler		 = m_gBufSampler->GetBindlessIndex(),
			.checkerTexture		 = m_checkerTexture->GetBindlessIndex(),
		};

		m_lightingPass.GetBuffer(frameIndex, 2).BufferData(0, (uint8*)&renderPassData, sizeof(GPUDataDeferredLightingPass));

		Vector<Entity*> entities;
		m_world->GetAllEntities(entities);
		m_objects.resize(entities.size());

		for (size_t i = 0; i < entities.size(); i++)
		{
			Entity* e	   = entities[i];
			e->m_ssboIndex = static_cast<uint32>(i);
			auto& data	   = m_objects[i];
			data.model	   = e->GetTransform().GetMatrix();

			// static Quaternion rot = Quaternion();
			// static float angle = 0.0f;
			// angle += 0.016f * 10;
			// rot.AxisAngle(Vector3(0, 1,0), angle);
			// data.model = Matrix4::TransformMatrix(Vector3(Math::RandF(-10, 10), Math::RandF(-10, 10), Math::RandF(-10, 10)), rot, Vector3::One);
		}
		currentFrame.objectBuffer.BufferData(0, (uint8*)m_objects.data(), sizeof(GPUDataObject) * m_objects.size());

		// Draw data map.
		m_drawData.clear();

		for (auto* mc : m_meshComponents)
		{
			MeshDefault* mesh	  = mc->GetMeshRaw();
			Material*	 material = mc->GetMaterialRaw();
			Shader*		 shader	  = material->GetShader();
			auto&		 vec	  = m_drawData[shader];

			auto it = linatl::find_if(vec.begin(), vec.end(), [mesh](const DrawDataMeshDefault& dd) -> bool { return dd.mesh == mesh; });

			if (it != vec.end())
				it->instances.push_back({material, mc->GetEntity()->m_ssboIndex});
			else
			{
				DrawDataMeshDefault drawData = {
					.mesh	   = mesh,
					.instances = {{material, mc->GetEntity()->m_ssboIndex}},
				};
				vec.push_back(drawData);
			}
		}

		// Indirect commands.
		uint32 constantsCount = 0;
		uint32 drawID		  = 0;
		uint32 indirectAmount = 0;

		for (const auto& [shader, meshVector] : m_drawData)
		{
			for (const auto& md : meshVector)
			{
				m_lgx->BufferIndexedIndirectCommandData(m_mainPass.GetBuffer(frameIndex, 1).GetMapped(),
														indirectAmount * m_lgx->GetIndexedIndirectCommandSize(),
														drawID,
														md.mesh->GetIndexCount(),
														static_cast<uint32>(md.instances.size()),
														md.mesh->GetIndexOffset(),
														md.mesh->GetVertexOffset(),
														drawID);

				indirectAmount++;
				for (const InstanceData& inst : md.instances)
				{
					GPUIndirectConstants0 constants = {
						.entityID		   = inst.entityIndex,
						.materialByteIndex = inst.material->GetBindlessBytePadding(),
					};
					m_mainPass.GetBuffer(frameIndex, 2).BufferData(constantsCount * sizeof(GPUIndirectConstants0), (uint8*)&constants, sizeof(GPUIndirectConstants0));
					constantsCount++;
					drawID++;
				}
			}
		}
	}

	SemaphoreData WorldRenderer::Render(uint32 frameIndex, const SemaphoreData& waitSemaphore)
	{
		UpdateBuffers(frameIndex);

		auto&		 currentFrame		= m_pfd[frameIndex];
		const uint64 copySemaphoreValue = currentFrame.copySemaphore.value;

		const LinaGX::Viewport viewport = {
			.x		  = 0,
			.y		  = 0,
			.width	  = m_size.x,
			.height	  = m_size.y,
			.minDepth = 0.0f,
			.maxDepth = 1.0f,
		};

		const LinaGX::ScissorsRect scissors = {
			.x		= 0,
			.y		= 0,
			.width	= m_size.x,
			.height = m_size.y,
		};

		// Global vertex/index buffers.
		m_gfxManager->GetMeshManager().BindBuffers(currentFrame.gfxStream, 0);

		// Global set.
		LinaGX::CMDBindDescriptorSets* bindGlobal = currentFrame.gfxStream->AddCommand<LinaGX::CMDBindDescriptorSets>();
		bindGlobal->descriptorSetHandles		  = currentFrame.gfxStream->EmplaceAuxMemory<uint16>(m_gfxManager->GetDescriptorSetPersistentGlobal(frameIndex));
		bindGlobal->firstSet					  = 0;
		bindGlobal->setCount					  = 1;
		bindGlobal->layoutSource				  = LinaGX::DescriptorSetsLayoutSource::CustomLayout;
		bindGlobal->customLayout				  = m_gfxManager->GetPipelineLayoutPersistentGlobal(frameIndex);

		uint8 transferExists = 0;

		transferExists |= currentFrame.objectBuffer.Copy(currentFrame.copyStream);
		transferExists |= m_lightingPass.GetBuffer(frameIndex, 0).Copy(currentFrame.copyStream);
		transferExists |= m_lightingPass.GetBuffer(frameIndex, 1).Copy(currentFrame.copyStream);
		transferExists |= m_lightingPass.GetBuffer(frameIndex, 2).Copy(currentFrame.copyStream);
		transferExists |= m_mainPass.GetBuffer(frameIndex, 0).Copy(currentFrame.copyStream); // view
		transferExists |= m_mainPass.GetBuffer(frameIndex, 1).Copy(currentFrame.copyStream); // indirect
		transferExists |= m_mainPass.GetBuffer(frameIndex, 2).Copy(currentFrame.copyStream); // indirect args

		if (transferExists != 0)
			BumpAndSendTransfers(frameIndex);

		{
			// Barrier to Attachment
			LinaGX::CMDBarrier* barrierToAttachment	 = currentFrame.gfxStream->AddCommand<LinaGX::CMDBarrier>();
			barrierToAttachment->srcStageFlags		 = LinaGX::PSF_TopOfPipe;
			barrierToAttachment->dstStageFlags		 = LinaGX::PSF_ColorAttachment | LinaGX::PSF_EarlyFragment;
			barrierToAttachment->textureBarrierCount = 5;
			barrierToAttachment->textureBarriers	 = currentFrame.gfxStream->EmplaceAuxMemorySizeOnly<LinaGX::TextureBarrier>(sizeof(LinaGX::TextureBarrier) * 5);
			barrierToAttachment->textureBarriers[0]	 = GfxHelpers::GetTextureBarrierColorRead2Att(currentFrame.gBufColorMaterialID->GetGPUHandle());
			barrierToAttachment->textureBarriers[1]	 = GfxHelpers::GetTextureBarrierColorRead2Att(currentFrame.gBufPosition->GetGPUHandle());
			barrierToAttachment->textureBarriers[2]	 = GfxHelpers::GetTextureBarrierColorRead2Att(currentFrame.gBufNormal->GetGPUHandle());
			barrierToAttachment->textureBarriers[3]	 = GfxHelpers::GetTextureBarrierDepthRead2Att(currentFrame.gBufDepth->GetGPUHandle());
			barrierToAttachment->textureBarriers[4]	 = GfxHelpers::GetTextureBarrierColorRead2Att(currentFrame.lightingPassOutput->GetGPUHandle());
		}

		m_mainPass.Begin(currentFrame.gfxStream, viewport, scissors, frameIndex);
		m_mainPass.BindDescriptors(currentFrame.gfxStream, frameIndex, false);

		Shader*	  lastBoundShader	= nullptr;
		Material* lastBoundMaterial = nullptr;

		uint32 indirectOffset = 0;

		for (auto& [shader, meshVec] : m_drawData)
		{
			if (shader != lastBoundShader)
			{
				shader->Bind(currentFrame.gfxStream, shader->GetGPUHandle());
				lastBoundShader = shader;
			}

			LinaGX::CMDDrawIndexedIndirect* draw = currentFrame.gfxStream->AddCommand<LinaGX::CMDDrawIndexedIndirect>();
			draw->count							 = static_cast<uint32>(meshVec.size());
			draw->indirectBuffer				 = m_mainPass.GetBuffer(frameIndex, 1).GetGPUResource();
			draw->indirectBufferOffset			 = indirectOffset;
			indirectOffset += draw->count * static_cast<uint32>(m_lgx->GetIndexedIndirectCommandSize());

#ifdef LINA_DEBUG
			uint32 totalTris = 0;
			for (const auto& md : meshVec)
				totalTris += md.mesh->GetIndexCount() / 3;
			PROFILER_ADD_DRAWCALL(totalTris, "WorldRenderer", 0);
#endif
		}

		m_mainPass.End(currentFrame.gfxStream);

		// Barrier to shader read.
		{
			LinaGX::CMDBarrier* barrier	 = currentFrame.gfxStream->AddCommand<LinaGX::CMDBarrier>();
			barrier->srcStageFlags		 = LinaGX::PSF_ColorAttachment | LinaGX::PSF_EarlyFragment;
			barrier->dstStageFlags		 = LinaGX::PSF_FragmentShader;
			barrier->textureBarrierCount = 3;
			barrier->textureBarriers	 = currentFrame.gfxStream->EmplaceAuxMemorySizeOnly<LinaGX::TextureBarrier>(sizeof(LinaGX::TextureBarrier) * 3);
			barrier->textureBarriers[0]	 = GfxHelpers::GetTextureBarrierColorAtt2Read(currentFrame.gBufColorMaterialID->GetGPUHandle());
			barrier->textureBarriers[1]	 = GfxHelpers::GetTextureBarrierColorAtt2Read(currentFrame.gBufPosition->GetGPUHandle());
			barrier->textureBarriers[2]	 = GfxHelpers::GetTextureBarrierColorAtt2Read(currentFrame.gBufNormal->GetGPUHandle());
		}

		m_lightingPass.Begin(currentFrame.gfxStream, viewport, scissors, frameIndex);
		m_lightingPass.BindDescriptors(currentFrame.gfxStream, frameIndex, false);

		m_deferredLightingShader->Bind(currentFrame.gfxStream, m_deferredLightingShader->GetGPUHandle());

		LinaGX::CMDDrawInstanced* lightingDraw = currentFrame.gfxStream->AddCommand<LinaGX::CMDDrawInstanced>();
		lightingDraw->instanceCount			   = 1;
		lightingDraw->startInstanceLocation	   = 0;
		lightingDraw->startVertexLocation	   = 0;
		lightingDraw->vertexCountPerInstance   = 3;

		Shader* skyShader = m_world->GetGfxSettings().skyMaterial.raw->GetShader();
		skyShader->Bind(currentFrame.gfxStream, skyShader->GetGPUHandle());

		LinaGX::CMDDrawIndexedInstanced* skyDraw = currentFrame.gfxStream->AddCommand<LinaGX::CMDDrawIndexedInstanced>();
		skyDraw->baseVertexLocation				 = m_skyCube->GetVertexOffset();
		skyDraw->indexCountPerInstance			 = m_skyCube->GetIndexCount();
		skyDraw->instanceCount					 = 1;
		skyDraw->startIndexLocation				 = m_skyCube->GetIndexOffset();
		skyDraw->startInstanceLocation			 = 0;

		m_lightingPass.End(currentFrame.gfxStream);

		// Barrier to shader read
		{
			LinaGX::CMDBarrier* barrier	 = currentFrame.gfxStream->AddCommand<LinaGX::CMDBarrier>();
			barrier->srcStageFlags		 = LinaGX::PSF_ColorAttachment | LinaGX::PSF_EarlyFragment;
			barrier->dstStageFlags		 = LinaGX::PSF_FragmentShader;
			barrier->textureBarrierCount = 2;
			barrier->textureBarriers	 = currentFrame.gfxStream->EmplaceAuxMemorySizeOnly<LinaGX::TextureBarrier>(sizeof(LinaGX::TextureBarrier) * 2);
			barrier->textureBarriers[0]	 = GfxHelpers::GetTextureBarrierColorAtt2Read(currentFrame.lightingPassOutput->GetGPUHandle());
			barrier->textureBarriers[1]	 = GfxHelpers::GetTextureBarrierColorAtt2Read(currentFrame.gBufDepth->GetGPUHandle());
		}

		m_lgx->CloseCommandStreams(&currentFrame.gfxStream, 1);

		Vector<uint16> waitSemaphores;
		Vector<uint64> waitValues;

		if (waitSemaphore.value != 0)
		{
			waitSemaphores.push_back(waitSemaphore.semaphore);
			waitValues.push_back(waitSemaphore.value);
		}

		if (currentFrame.copySemaphore.value != copySemaphoreValue)
		{
			waitSemaphores.push_back(currentFrame.copySemaphore.semaphore);
			waitValues.push_back(currentFrame.copySemaphore.value);
		}

		currentFrame.signalSemaphore.value++;

		m_lgx->SubmitCommandStreams({
			.targetQueue	  = m_lgx->GetPrimaryQueue(LinaGX::CommandType::Graphics),
			.streams		  = &currentFrame.gfxStream,
			.streamCount	  = 1,
			.useWait		  = !waitSemaphores.empty(),
			.waitCount		  = static_cast<uint32>(waitSemaphores.size()),
			.waitSemaphores	  = waitSemaphores.data(),
			.waitValues		  = waitValues.data(),
			.useSignal		  = true,
			.signalCount	  = 1,
			.signalSemaphores = &currentFrame.signalSemaphore.semaphore,
			.signalValues	  = &currentFrame.signalSemaphore.value,
			.isMultithreaded  = true,
		});

		return currentFrame.signalSemaphore;
	}

	uint64 WorldRenderer::BumpAndSendTransfers(uint32 frameIndex)
	{
		auto& currentFrame = m_pfd[frameIndex];
		currentFrame.copySemaphore.value++;
		m_lgx->CloseCommandStreams(&currentFrame.copyStream, 1);
		m_lgx->SubmitCommandStreams({
			.targetQueue	  = m_lgx->GetPrimaryQueue(LinaGX::CommandType::Transfer),
			.streams		  = &currentFrame.copyStream,
			.streamCount	  = 1,
			.useSignal		  = true,
			.signalCount	  = 1,
			.signalSemaphores = &(currentFrame.copySemaphore.semaphore),
			.signalValues	  = &currentFrame.copySemaphore.value,
			.isMultithreaded  = true,
		});
		return currentFrame.copySemaphore.value;
	}

} // namespace Lina
