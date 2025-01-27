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

#include "Editor/Graphics/MousePickRenderer.hpp"
#include "Editor/Editor.hpp"
#include "Editor/Graphics/EditorGfxHelpers.hpp"
#include "Editor/Graphics/EditorWorldRenderer.hpp"
#include "Common/Math/Math.hpp"
#include "Core/Resources/ResourceManager.hpp"
#include "Core/Application.hpp"
#include "Core/Graphics/Renderers/DrawCollector.hpp"
#include "Core/Graphics/Resource/Texture.hpp"
#include "Core/Graphics/Renderers/WorldRenderer.hpp"
#include "Core/Graphics/Utility/GfxHelpers.hpp"
#include "Core/World/EntityWorld.hpp"
#include "Common/System/SystemInfo.hpp"

namespace Lina::Editor
{

	MousePickRenderer::MousePickRenderer(Editor* editor, EditorWorldRenderer* ewr)
	{
		m_editor = editor;
		m_ewr	 = ewr;
		m_wr	 = m_ewr->GetWorldRenderer();
		m_rm	 = &m_editor->GetApp()->GetResourceManager();
		m_world	 = m_wr->GetWorld();
		m_lgx	 = editor->GetApp()->GetLGX();

		m_entityBufferPass.Create(EditorGfxHelpers::GetEditorWorldPassDescription());

		for (uint32 i = 0; i < FRAMES_IN_FLIGHT; i++)
		{

			const uint16 set = m_entityBufferPass.GetDescriptorSet(i);

			m_lgx->DescriptorUpdateBuffer({
				.setHandle = set,
				.binding   = 1,
				.buffers   = {m_wr->GetInstanceDataBuffer(i).GetGPUResource()},
			});

			m_lgx->DescriptorUpdateBuffer({
				.setHandle = set,
				.binding   = 2,
				.buffers   = {m_wr->GetEntityDataBuffer(i).GetGPUResource()},
			});

			m_lgx->DescriptorUpdateBuffer({
				.setHandle = set,
				.binding   = 3,
				.buffers   = {m_wr->GetBoneBuffer(i).GetGPUResource()},
			});
		}

	} // namespace Lina::Editor

	MousePickRenderer::~MousePickRenderer()
	{
		DestroySizeRelativeResources();
		m_entityBufferPass.Destroy();
	}

	void MousePickRenderer::DestroySizeRelativeResources()
	{
		for (uint32 i = 0; i < FRAMES_IN_FLIGHT; i++)
		{
			PerFrameData& data = m_pfd[i];

			if (data.depthTarget == nullptr)
				continue;

			data.depthTarget->DestroyHW();
			data.renderTarget->DestroyHW();
			data.snapshotBuffer.Destroy();

			m_rm->DestroyResource(data.renderTarget);
			m_rm->DestroyResource(data.depthTarget);

			data.renderTarget = data.depthTarget = nullptr;
		}
	}

	void MousePickRenderer::CreateSizeRelativeResources()
	{
		m_size = m_wr->GetSize();
		m_entityBufferPass.SetSize(m_size);

		const LinaGX::TextureDesc rtDesc = {
			.format = LinaGX::Format::R32_UINT,
			.flags	= LinaGX::TF_ColorAttachment | LinaGX::TF_Sampled | LinaGX::TF_CopySource,
			.width	= m_size.x,
			.height = m_size.y,
		};

		const LinaGX::TextureDesc depthDesc = {
			.format					  = SystemInfo::GetDepthFormat(),
			.depthStencilSampleFormat = LinaGX::Format::R32_SFLOAT,
			.flags					  = LinaGX::TF_DepthTexture | LinaGX::TF_Sampled,
			.width					  = m_size.x,
			.height					  = m_size.y,
		};

		for (uint32 i = 0; i < FRAMES_IN_FLIGHT; i++)
		{
			PerFrameData& data = m_pfd[i];

			data.depthTarget  = m_rm->CreateResource<Texture>(m_rm->ConsumeResourceID(), "MousePickDepthTarget");
			data.renderTarget = m_rm->CreateResource<Texture>(m_rm->ConsumeResourceID(), "MousePickRenderTarget");
			data.renderTarget->GenerateHWFromDesc(rtDesc);
			data.depthTarget->GenerateHWFromDesc(depthDesc);
			m_entityBufferPass.SetColorAttachment(i, 0, {.clearColor = {0.0f, 0.0f, 0.0f, 1.0f}, .texture = data.renderTarget->GetGPUHandle(), .isSwapchain = false});
			m_entityBufferPass.DepthStencilAttachment(i, {.useDepth = true, .texture = data.depthTarget->GetGPUHandle(), .depthLoadOp = LinaGX::LoadOp::Clear, .depthStoreOp = LinaGX::StoreOp::Store, .clearDepth = 1.0f});
			data.snapshotBuffer.Create(LinaGX::ResourceTypeHint::TH_ReadbackDest, m_size.x * m_size.y * 4, "MousePickerSnapshot", true);
		}
	}

	void MousePickRenderer::AddBuffersToUploadQueue(uint32 frameIndex, ResourceUploadQueue& queue)
	{
		// View data.
		{
			Camera&					worldCam = m_world->GetWorldCamera();
			EditorWorldPassViewData view	 = {
					.view = worldCam.GetView(),
					.proj = worldCam.GetProjection(),
			};

			const Vector2 displayPos = m_world->GetScreen().GetDisplayPos();

			const Vector3& camPos	   = worldCam.GetPosition();
			const Vector3& camDir	   = worldCam.GetRotation().GetForward();
			view.viewProj			   = view.proj * view.view;
			view.cameraPositionAndNear = Vector4(camPos.x, camPos.y, camPos.z, worldCam.GetZNear());
			view.cameraDirectionAndFar = Vector4(camDir.x, camDir.y, camDir.z, worldCam.GetZFar());
			view.size				   = Vector2(static_cast<float>(m_size.x), static_cast<float>(m_size.y));
			view.mouse				   = m_world->GetInput().GetMousePositionRatio();
			m_entityBufferPass.GetBuffer(frameIndex, "ViewData"_hs).BufferData(0, (uint8*)&view, sizeof(EditorWorldPassViewData));
		}

		const Vector2 mp		  = m_world->GetInput().GetMousePosition();
		const bool	  mpOK		  = mp.x < m_size.x - 1 && mp.y < m_size.y - 1 && mp.x > 0 && mp.y > 0;
		size_t		  entityIndex = 0;
		uint32*		  mapping	  = reinterpret_cast<uint32*>(m_pfd[frameIndex].snapshotBuffer.GetMapped());

		if (mpOK)
		{
			const uint32 pixel = m_size.x * static_cast<uint32>(mp.y) + static_cast<uint32>(mp.x);
			entityIndex		   = static_cast<size_t>(mapping[pixel]);
		}

		if (m_rectSelectData.select)
		{
			m_rectSelectData.select = false;

			if (mpOK)
			{
				for (uint32 y = m_rectSelectData.start.y; y < m_rectSelectData.end.y; y++)
				{
					for (uint32 x = m_rectSelectData.start.x; x < m_rectSelectData.end.x; x++)
					{
						const uint32 pixelIndex = y * m_size.x + x;

						const size_t index = static_cast<size_t>(mapping[pixelIndex]);
						if (index != 0 && index - 1 < m_lastEntityIDs.size())
						{
							const EntityID res = m_lastEntityIDs[index - 1];
							auto		   it  = linatl::find_if(m_gpuData.rectSelectionIDs.begin(), m_gpuData.rectSelectionIDs.end(), [res](EntityID id) -> bool { return id == res; });

							if (it == m_gpuData.rectSelectionIDs.end())
								m_gpuData.rectSelectionIDs.push_back(res);
						}
					}
				}
			}
		}

		if (entityIndex == 0)
			m_gpuData.lastHoveredEntityID = 0;
		else if (entityIndex - 1 < m_lastEntityIDs.size())
			m_gpuData.lastHoveredEntityID = m_lastEntityIDs[entityIndex - 1];

		m_entityBufferPass.AddBuffersToUploadQueue(frameIndex, queue);
	}

	void MousePickRenderer::Tick(float delta)
	{
		const Vector<CompModel*>& compModels = m_wr->GetCompModels();
		DrawCollector::CollectCompModels(compModels,
										 m_entityBufferPass,
										 m_rm,
										 m_wr,
										 &m_editor->GetApp()->GetGfxContext(),
										 {
											 .useVariantOverride	 = true,
											 .staticVariantOverride	 = "StaticEntityID"_hs,
											 .skinnedVariantOverride = "SkinnedEntityID"_hs,
											 .allowedShaderTypes	 = {ShaderType::DeferredSurface, ShaderType::ForwardSurface},
										 });
	}

	void MousePickRenderer::Render(uint32 frameIndex, LinaGX::CommandStream* stream)
	{
		if (m_entityBufferPass.GetDrawCallsGPU().empty())
			return;

		PerFrameData& pfd = m_pfd[frameIndex];

		// PASS
		{
			DEBUG_LABEL_BEGIN(stream, "Editor: Entity Buffer Pass");

			// Barrier to Attachment
			{
				LinaGX::CMDBarrier* barrierToAttachment	 = stream->AddCommand<LinaGX::CMDBarrier>();
				barrierToAttachment->srcStageFlags		 = LinaGX::PSF_TopOfPipe;
				barrierToAttachment->dstStageFlags		 = LinaGX::PSF_ColorAttachment | LinaGX::PSF_EarlyFragment;
				barrierToAttachment->textureBarrierCount = 2;
				barrierToAttachment->textureBarriers	 = stream->EmplaceAuxMemorySizeOnly<LinaGX::TextureBarrier>(sizeof(LinaGX::TextureBarrier) * 2);
				barrierToAttachment->textureBarriers[0]	 = GfxHelpers::GetTextureBarrierColorRead2Att(pfd.renderTarget->GetGPUHandle());
				barrierToAttachment->textureBarriers[1]	 = GfxHelpers::GetTextureBarrierDepthRead2Att(pfd.depthTarget->GetGPUHandle());
			}

			m_entityBufferPass.Begin(stream, frameIndex);
			m_entityBufferPass.BindDescriptors(stream, frameIndex, m_ewr->GetPipelineLayout(), 1);
			m_entityBufferPass.Render(frameIndex, stream);
			m_entityBufferPass.End(stream);

			// Render target to transfer source, depth to shader read.
			{
				LinaGX::CMDBarrier* barrier	 = stream->AddCommand<LinaGX::CMDBarrier>();
				barrier->srcStageFlags		 = LinaGX::PSF_ColorAttachment | LinaGX::PSF_EarlyFragment;
				barrier->dstStageFlags		 = LinaGX::PSF_FragmentShader;
				barrier->textureBarrierCount = 2;
				barrier->textureBarriers	 = stream->EmplaceAuxMemorySizeOnly<LinaGX::TextureBarrier>(sizeof(LinaGX::TextureBarrier) * 2);
				barrier->textureBarriers[0]	 = {
					 .texture		 = pfd.renderTarget->GetGPUHandle(),
					 .isSwapchain	 = false,
					 .toState		 = LinaGX::TextureState::TransferSource,
					 .srcAccessFlags = LinaGX::AF_ColorAttachmentRead,
					 .dstAccessFlags = LinaGX::AF_ShaderRead,
				 };
				barrier->textureBarriers[1] = GfxHelpers::GetTextureBarrierDepthAtt2Read(pfd.depthTarget->GetGPUHandle());
			}

			LinaGX::CMDCopyTexture2DToBuffer* copy = stream->AddCommand<LinaGX::CMDCopyTexture2DToBuffer>();
			copy->destBuffer					   = m_pfd[frameIndex].snapshotBuffer.GetGPUResource();
			copy->srcLayer						   = 0;
			copy->srcMip						   = 0;
			copy->srcTexture					   = m_pfd[frameIndex].renderTarget->GetGPUHandle();

			const Vector<DrawEntity>& drawEntities = m_wr->GetEntitiesGPU();
			m_lastEntityIDs.resize(drawEntities.size());
			for (size_t i = 0; i < drawEntities.size(); i++)
				m_lastEntityIDs[i] = drawEntities.at(i).ident.entityGUID;

			// To shader read
			{
				// LinaGX::CMDBarrier* barrier	 = stream->AddCommand<LinaGX::CMDBarrier>();
				// barrier->srcStageFlags		 = LinaGX::PSF_ColorAttachment | LinaGX::PSF_EarlyFragment;
				// barrier->dstStageFlags		 = LinaGX::PSF_FragmentShader;
				// barrier->textureBarrierCount = 2;
				// barrier->textureBarriers	 = stream->EmplaceAuxMemorySizeOnly<LinaGX::TextureBarrier>(sizeof(LinaGX::TextureBarrier) * 2);
				// barrier->textureBarriers[0]	 = GfxHelpers::GetTextureBarrierColorAtt2Read(pfd.renderTarget->GetGPUHandle());
				// barrier->textureBarriers[1]	 = GfxHelpers::GetTextureBarrierDepthAtt2Read(pfd.depthTarget->GetGPUHandle());
			}

			DEBUG_LABEL_END(stream);
		}
	}

	void MousePickRenderer::SyncRender()
	{
		m_cpuData.lastHoveredEntityID = m_gpuData.lastHoveredEntityID;
		m_cpuData.rectSelectionIDs	  = m_gpuData.rectSelectionIDs;
		m_gpuData.lastHoveredEntityID = 0;
		m_gpuData.rectSelectionIDs.resize(0);
		m_entityBufferPass.SyncRender();
	}

	void MousePickRenderer::SelectRect(const Vector2ui& start, const Vector2ui& end)
	{
		m_rectSelectData.select = true;
		m_rectSelectData.start	= Vector2(Math::Min(start.x, end.x), Math::Min(start.y, end.y));
		m_rectSelectData.end	= Vector2(Math::Max(start.x, end.x), Math::Max(start.y, end.y));
	}
} // namespace Lina::Editor
