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

#include "Editor/Graphics/SurfaceRenderer.hpp"
#include "Editor/Editor.hpp"
#include "Editor/Graphics/EditorGfxHelpers.hpp"
#include "Core/Graphics/Utility/GfxHelpers.hpp"
#include "Core/Graphics/Resource/Material.hpp"
#include "Core/Graphics/Resource/Texture.hpp"
#include "Core/Graphics/Resource/TextureSampler.hpp"
#include "Core/Graphics/Resource/Shader.hpp"
#include "Common/System/SystemInfo.hpp"
#include "Core/Graphics/Data/Vertex.hpp"
#include "Core/Application.hpp"
#include "Core/ApplicationDelegate.hpp"
#include "Core/Resources/ResourceManager.hpp"
#include "Core/Graphics/Data/RenderData.hpp"
#include "Common/Profiling/Profiler.hpp"

namespace Lina::Editor
{
#define MAX_GFX_COMMANDS  1000
#define MAX_COPY_COMMANDS 1000

#define MAX_GUI_VERTICES 120000
#define MAX_GUI_INDICES	 140000

#define VSYNC_DX LinaGX::DXVsync::None
#define VSYNC_VK LinaGX::VKVsync::None

	SurfaceRenderer::SurfaceRenderer(Editor* editor, LinaGX::Window* window, const Color& clearColor) : m_window(window)
	{
		m_editor			= editor;
		m_resourceManagerV2 = &m_editor->GetApp()->GetResourceManager();
		m_lgx				= Application::GetLGX();
		m_size				= m_window->GetSize();
		m_guiDefault		= m_editor->GetApp()->GetResourceManager().GetResource<Shader>(EDITOR_SHADER_GUI_DEFAULT_ID);
		m_guiColorWheel		= m_editor->GetApp()->GetResourceManager().GetResource<Shader>(EDITOR_SHADER_GUI_COLOR_WHEEL_ID);
		m_guiHue			= m_editor->GetApp()->GetResourceManager().GetResource<Shader>(EDITOR_SHADER_GUI_HUE_DISPLAY_ID);
		m_guiText			= m_editor->GetApp()->GetResourceManager().GetResource<Shader>(EDITOR_SHADER_GUI_TEXT_ID);
		m_guiSDF			= m_editor->GetApp()->GetResourceManager().GetResource<Shader>(EDITOR_SHADER_GUI_SDF_TEXT_ID);

		const LinaGX::VSyncStyle vsync = {
			.vulkanVsync = VSYNC_VK,
			.dx12Vsync	 = VSYNC_DX,
		};

		// Swapchain
		const auto monitorSize = window->GetMonitorSize();
		const auto windowSize  = window->GetSize();
		m_swapchain			   = m_lgx->CreateSwapchain({
					   .format		 = DEFAULT_SWAPCHAIN_FORMAT,
					   .x			 = 0,
					   .y			 = 0,
					   .width		 = windowSize.x,
					   .height		 = windowSize.y,
					   .window		 = window->GetWindowHandle(),
					   .osHandle	 = window->GetOSHandle(),
					   .isFullscreen = windowSize.x == monitorSize.x && windowSize.y == monitorSize.y,
					   .vsyncStyle	 = vsync,
		   });

		for (uint32 i = 0; i < FRAMES_IN_FLIGHT; i++)
		{
			auto& data	   = m_pfd[i];
			data.gfxStream = m_lgx->CreateCommandStream({LinaGX::CommandType::Graphics, MAX_GFX_COMMANDS, 24000, 4096, 32, "SurfaceRenderer: Gfx Stream"});
			m_guiPass.SetColorAttachment(i, 0, {.clearColor = {clearColor.x, clearColor.y, clearColor.z, clearColor.w}, .texture = static_cast<uint32>(m_swapchain), .isSwapchain = true});

			const String cmdStreamDbg = "SurfaceRenderer: CopyStream" + TO_STRING(i);
			data.copyStream			  = m_lgx->CreateCommandStream({LinaGX::CommandType::Transfer, MAX_COPY_COMMANDS, 4000, 1024, 32, cmdStreamDbg.c_str()});
			data.copySemaphore		  = SemaphoreData(m_lgx->CreateUserSemaphore());

			const String istr = TO_STRING(i);
			data.guiVertexBuffer.Create(LinaGX::ResourceTypeHint::TH_VertexBuffer, MAX_GUI_VERTICES * sizeof(LinaVG::Vertex), "SurfaceRenderer: VertexBuffer" + istr);
			data.guiIndexBuffer.Create(LinaGX::ResourceTypeHint::TH_IndexBuffer, MAX_GUI_INDICES * sizeof(LinaVG::Index), "SurfaceRenderer: IndexBuffer" + istr);
		}

		m_cpuDraw.guiIndexBuffer  = {new uint8[MAX_GUI_INDICES * sizeof(LinaVG::Index)], MAX_GUI_INDICES * sizeof(LinaVG::Index)};
		m_cpuDraw.guiVertexBuffer = {new uint8[MAX_GUI_VERTICES * sizeof(LinaVG::Vertex)], MAX_GUI_VERTICES * sizeof(LinaVG::Vertex)};
		m_cpuDraw.materialBuffer  = {new uint8[10000], 10000};

		m_renderDraw.guiIndexBuffer	 = {new uint8[MAX_GUI_INDICES * sizeof(LinaVG::Index)], MAX_GUI_INDICES * sizeof(LinaVG::Index)};
		m_renderDraw.guiVertexBuffer = {new uint8[MAX_GUI_VERTICES * sizeof(LinaVG::Vertex)], MAX_GUI_VERTICES * sizeof(LinaVG::Vertex)};
		m_renderDraw.materialBuffer	 = {new uint8[10000], 10000};

		// RP
		m_guiPass.Create(EditorGfxHelpers::GetGUIPassDescription());
		m_widgetManager.Initialize(&m_editor->GetApp()->GetResourceManager(), m_window, &m_lvgDrawer);
		m_lvgDrawer.GetCallbacks().draw = BIND(&SurfaceRenderer::DrawDefault, this, std::placeholders::_1);
	}

	SurfaceRenderer::~SurfaceRenderer()
	{
		delete[] m_cpuDraw.guiIndexBuffer.data();
		delete[] m_cpuDraw.guiVertexBuffer.data();
		delete[] m_cpuDraw.materialBuffer.data();
		delete[] m_renderDraw.guiIndexBuffer.data();
		delete[] m_renderDraw.guiVertexBuffer.data();
		delete[] m_renderDraw.materialBuffer.data();

		m_widgetManager.Shutdown();
		m_guiPass.Destroy();

		for (uint32 i = 0; i < FRAMES_IN_FLIGHT; i++)
		{
			auto& data = m_pfd[i];
			m_lgx->DestroyCommandStream(data.gfxStream);
			m_lgx->DestroyCommandStream(data.copyStream);
			m_lgx->DestroyUserSemaphore(data.copySemaphore.GetSemaphore());
			data.guiVertexBuffer.Destroy();
			data.guiIndexBuffer.Destroy();
		}

		m_lgx->DestroySwapchain(m_swapchain);
	}

	void SurfaceRenderer::OnWindowSizeChanged(LinaGX::Window* window, const Vector2ui& newSize)
	{
		if (window != m_window)
			return;

		const LinaGX::LGXVector2ui monitorSize = m_window->GetMonitorSize();

		const LinaGX::VSyncStyle vsync = {
			.vulkanVsync = VSYNC_VK,
			.dx12Vsync	 = VSYNC_DX,
		};

		LinaGX::SwapchainRecreateDesc desc = {
			.swapchain	  = m_swapchain,
			.width		  = newSize.x,
			.height		  = newSize.y,
			.isFullscreen = newSize.x == monitorSize.x && newSize.y == monitorSize.y,
			.vsyncStyle	  = vsync,
		};

		m_lgx->RecreateSwapchain(desc);
		m_size = newSize;
	}

	bool SurfaceRenderer::CheckVisibility()
	{
		auto ws		= m_window->GetSize();
		m_isVisible = m_window->GetIsVisible() && ws.x != 0 && ws.y != 0 && !m_window->GetIsMinimized();
		m_lgx->SetSwapchainActive(m_swapchain, m_isVisible);
		return m_isVisible;
	}

	void SurfaceRenderer::PreTick()
	{
		m_widgetManager.PreTick();
	}

	void SurfaceRenderer::Tick(float delta)
	{
		m_widgetManager.Tick(delta, m_size);

		m_drawRequests.clear();
		m_frameMaterialBufferCounter = 0;
		m_frameVertexCounter		 = 0;
		m_frameIndexCounter			 = 0;

		m_widgetManager.Draw();
		m_lvgDrawer.FlushBuffers();
		m_lvgDrawer.ResetFrame();
	}

	void SurfaceRenderer::DrawDefault(LinaVG::DrawBuffer* buf)
	{
		const uint32 frameIndex = m_lgx->GetCurrentFrameIndex();
		auto&		 pfd		= m_pfd[frameIndex];

		// Update vertex / index buffers.
		const uint32 vtxSize = static_cast<uint32>(buf->vertexBuffer.m_size);
		const uint32 idxSize = static_cast<uint32>(buf->indexBuffer.m_size);
		MEMCPY(m_cpuDraw.guiVertexBuffer.data() + m_cpuDraw.vertexCounter * sizeof(LinaVG::Vertex), (uint8*)buf->vertexBuffer.m_data, vtxSize * sizeof(LinaVG::Vertex));
		MEMCPY(m_cpuDraw.guiIndexBuffer.data() + m_cpuDraw.indexCounter * sizeof(LinaVG::Index), (uint8*)buf->indexBuffer.m_data, idxSize * sizeof(LinaVG::Index));

		// pfd.guiVertexBuffer.BufferData(m_frameVertexCounter * sizeof(LinaVG::Vertex), (uint8*)buf->vertexBuffer.m_data, vtxSize * sizeof(LinaVG::Vertex));
		// pfd.guiIndexBuffer.BufferData(m_frameIndexCounter * sizeof(LinaVG::Index), (uint8*)buf->indexBuffer.m_data, idxSize * sizeof(LinaVG::Index));

		DrawRequest drawRequest = DrawRequest{
			.startVertex	= m_cpuDraw.vertexCounter,
			.startIndex		= m_cpuDraw.indexCounter,
			.vertexCount	= vtxSize,
			.indexCount		= idxSize,
			.materialOffset = m_cpuDraw.materialBufferCounter,
			.clip			= Recti(buf->clip.x, buf->clip.y, buf->clip.z, buf->clip.w),
		};

		GUIUserData* guiUserData = nullptr;
		if (buf->userData != nullptr)
			guiUserData = static_cast<GUIUserData*>(buf->userData);

		ResourceManagerV2& rm		  = m_editor->GetApp()->GetResourceManager();
		GUIBackend&		   guiBackend = m_editor->GetApp()->GetGUIBackend();
		// Buffer&			   materialBuffer = m_guiPass.GetBuffer(frameIndex, "GUIMaterials"_hs);

		if (buf->shapeType == LinaVG::DrawBufferShapeType::Shape || buf->shapeType == LinaVG::DrawBufferShapeType::AA)
		{
			if (guiUserData)
			{
				if (guiUserData->specialType == GUISpecialType::None)
				{
					Texture* texture = buf->textureHandle == nullptr ? nullptr : static_cast<Texture*>(buf->textureHandle);

					GPUMaterialGUIDefault material = {
						.uvTilingAndOffset = buf->textureUV,
						.hasTexture		   = texture != nullptr,
						.displayChannels   = 0,
						.displayLod		   = 0,
						.singleChannel	   = texture == nullptr ? (uint32)0 : (uint32)(texture->GetMeta().format == LinaGX::Format::R8_UNORM),
						.diffuse =
							{
								.textureIndex = texture != nullptr ? texture->GetBindlessIndex() : 0,
								.samplerIndex = texture != nullptr ? m_editor->GetEditorRenderer().GetGUISampler()->GetBindlessIndex() : 0,
							},
					};

					material.displayLod		 = guiUserData->mipLevel;
					material.displayChannels = static_cast<uint32>(guiUserData->displayChannels);

					if (guiUserData->sampler != 0)
						material.diffuse.samplerIndex = rm.GetResource<TextureSampler>(guiUserData->sampler)->GetBindlessIndex();

					drawRequest.shader		   = m_guiDefault;
					drawRequest.textureID	   = texture != nullptr ? texture->GetID() : 0;
					drawRequest.texturePadding = m_cpuDraw.materialBufferCounter + offsetof(GPUMaterialGUIDefault, diffuse);

					MEMCPY(m_cpuDraw.materialBuffer.data() + m_cpuDraw.materialBufferCounter, (uint8*)&material, sizeof(GPUMaterialGUIDefault));
					m_cpuDraw.materialBufferCounter += sizeof(GPUMaterialGUIDefault);

					// materialBuffer.BufferData(m_frameMaterialBufferCounter, (uint8*)&material, sizeof(GPUMaterialGUIDefault));
					// m_frameMaterialBufferCounter += sizeof(GPUMaterialGUIDefault);
				}
				else if (guiUserData->specialType == GUISpecialType::ColorWheel)
				{
					drawRequest.shader = m_guiColorWheel;

					GPUMaterialGUIColorWheel material = {
						.wheelRadius	= 0.5f,
						.edgeSmoothness = 0.005f,
					};

					MEMCPY(m_cpuDraw.materialBuffer.data() + m_cpuDraw.materialBufferCounter, (uint8*)&material, sizeof(GPUMaterialGUIColorWheel));
					m_cpuDraw.materialBufferCounter += sizeof(GPUMaterialGUIColorWheel);

					// materialBuffer.BufferData(m_frameMaterialBufferCounter, (uint8*)&material, sizeof(GPUMaterialGUIColorWheel));
					// m_frameMaterialBufferCounter += sizeof(GPUMaterialGUIColorWheel);
				}
				else if (guiUserData->specialType == GUISpecialType::VerticalHue || guiUserData->specialType == GUISpecialType::HorizontalHue)
				{
					drawRequest.shader = m_guiHue;

					const bool				 isVertical = guiUserData->specialType == GUISpecialType::VerticalHue;
					GPUMaterialGUIHueDisplay material	= {
						  .uvContribution = Vector2(isVertical == false ? 1.0f : 0.0f, isVertical ? 1.0f : 0.0f),
					  };

					MEMCPY(m_cpuDraw.materialBuffer.data() + m_cpuDraw.materialBufferCounter, (uint8*)&material, sizeof(GPUMaterialGUIHueDisplay));
					m_cpuDraw.materialBufferCounter += sizeof(GPUMaterialGUIHueDisplay);

					// materialBuffer.BufferData(m_frameMaterialBufferCounter, (uint8*)&material, sizeof(GPUMaterialGUIHueDisplay));
					// m_frameMaterialBufferCounter += sizeof(GPUMaterialGUIHueDisplay);
				}
			}
			else
			{
				drawRequest.shader = m_guiDefault;
				Texture* texture   = buf->textureHandle == nullptr ? nullptr : static_cast<Texture*>(buf->textureHandle);

				GPUMaterialGUIDefault material = {
					.uvTilingAndOffset = buf->textureUV,
					.hasTexture		   = texture != nullptr,
					.displayChannels   = 0,
					.displayLod		   = 0,
					.singleChannel	   = texture == nullptr ? (uint32)0 : (uint32)(texture->GetMeta().format == LinaGX::Format::R8_UNORM),
					.diffuse =
						{
							.textureIndex = texture != nullptr ? texture->GetBindlessIndex() : 0,
							.samplerIndex = texture != nullptr ? m_editor->GetEditorRenderer().GetGUISampler()->GetBindlessIndex() : 0,
						},
				};

				drawRequest.textureID	   = texture != nullptr ? texture->GetID() : 0;
				drawRequest.texturePadding = m_cpuDraw.materialBufferCounter + offsetof(GPUMaterialGUIDefault, diffuse);

				MEMCPY(m_cpuDraw.materialBuffer.data() + m_cpuDraw.materialBufferCounter, (uint8*)&material, sizeof(GPUMaterialGUIDefault));
				m_cpuDraw.materialBufferCounter += sizeof(GPUMaterialGUIDefault);

				// materialBuffer.BufferData(m_frameMaterialBufferCounter, (uint8*)&material, sizeof(GPUMaterialGUIDefault));
				// m_frameMaterialBufferCounter += sizeof(GPUMaterialGUIDefault);
			}
		}
		else if (buf->shapeType == LinaVG::DrawBufferShapeType::Text)
		{
			drawRequest.shader = m_guiText;

			LinaVG::Atlas*	   atlas	= static_cast<LinaVG::Atlas*>(buf->textureHandle);
			Texture*		   texture	= guiBackend.GetFontTexture(atlas).texture;
			GPUMaterialGUIText material = {
				.diffuse =
					{
						.textureIndex = texture->GetBindlessIndex(),
						.samplerIndex = m_editor->GetEditorRenderer().GetGUITextSampler()->GetBindlessIndex(),
					},
			};

			MEMCPY(m_cpuDraw.materialBuffer.data() + m_cpuDraw.materialBufferCounter, (uint8*)&material, sizeof(GPUMaterialGUIText));
			m_cpuDraw.materialBufferCounter += sizeof(GPUMaterialGUIText);
			drawRequest.texturePadding = m_cpuDraw.materialBufferCounter;

			drawRequest.textureID = texture != nullptr ? texture->GetID() : 0;

			// materialBuffer.BufferData(m_frameMaterialBufferCounter, (uint8*)&material, sizeof(GPUMaterialGUIText));
			// m_frameMaterialBufferCounter += sizeof(GPUMaterialGUIText);
		}
		else if (buf->shapeType == LinaVG::DrawBufferShapeType::SDFText)
		{
			drawRequest.shader = m_guiSDF;

			Texture*			  texture  = guiBackend.GetFontTexture(static_cast<LinaVG::Atlas*>(buf->textureHandle)).texture;
			GPUMaterialGUISDFText material = {
				.diffuse =
					{
						.textureIndex = texture->GetBindlessIndex(),
						.samplerIndex = m_editor->GetEditorRenderer().GetGUITextSampler()->GetBindlessIndex(),
					},
				.outlineColor	  = Vector4(0, 0, 0, 0),
				.thickness		  = 0.52f,
				.softness		  = 0.025f,
				.outlineThickness = 0.0f,
				.outlineSoftness  = 0.02f,
			};

			drawRequest.textureID	   = texture != nullptr ? texture->GetID() : 0;
			drawRequest.texturePadding = m_cpuDraw.materialBufferCounter;

			if (guiUserData)
			{
				material.thickness		  = guiUserData->sdfThickness;
				material.softness		  = guiUserData->sdfSoftness;
				material.outlineThickness = guiUserData->sdfOutlineThickness;
				material.outlineSoftness  = guiUserData->sdfOutlineSoftness;
				material.outlineColor	  = guiUserData->sdfOutlineColor;
			}

			MEMCPY(m_cpuDraw.materialBuffer.data() + m_cpuDraw.materialBufferCounter, (uint8*)&material, sizeof(GPUMaterialGUISDFText));
			m_cpuDraw.materialBufferCounter += sizeof(GPUMaterialGUISDFText);

			// materialBuffer.BufferData(m_frameMaterialBufferCounter, (uint8*)&material, sizeof(GPUMaterialGUISDFText));
			// m_frameMaterialBufferCounter += sizeof(GPUMaterialGUISDFText);
		}

		m_cpuDraw.requests.emplace_back(drawRequest);

		m_cpuDraw.vertexCounter += vtxSize;
		m_cpuDraw.indexCounter += idxSize;
		// m_frameVertexCounter += vtxSize;
		// m_frameIndexCounter += idxSize;

		PROFILER_ADD_DRAWCALL(1);
		PROFILER_ADD_VERTICESINDICES(drawRequest.vertexCount, drawRequest.indexCount);
		PROFILER_ADD_TRIS(drawRequest.indexCount / 3);
	}

	void SurfaceRenderer::UpdateBuffers(uint32 frameIndex)
	{
		auto& currentFrame = m_pfd[frameIndex];

		if (m_renderDraw.requests.empty())
			return;

		// Transfers
		GPUDataGUIView view = {.proj = GfxHelpers::GetProjectionFromSize(m_window->GetSize())};
		m_guiPass.GetBuffer(frameIndex, "GUIViewData"_hs).BufferData(0, (uint8*)&view, sizeof(GPUDataGUIView));

		Buffer& materialBuffer = m_guiPass.GetBuffer(frameIndex, "GUIMaterials"_hs);
		currentFrame.guiIndexBuffer.BufferData(0, m_renderDraw.guiIndexBuffer.data(), m_renderDraw.indexCounter * sizeof(LinaVG::Index));
		currentFrame.guiVertexBuffer.BufferData(0, m_renderDraw.guiVertexBuffer.data(), m_renderDraw.vertexCounter * sizeof(LinaVG::Vertex));

		// By the time materials are recorded the texture bindless indices are subject to change.
		// We need to do this correction since we are rendering the frame N-1
		for (DrawRequest& req : m_renderDraw.requests)
		{
			if (req.textureID == 0)
				continue;

			Texture* res = m_resourceManagerV2->GetResource<Texture>(req.textureID);
			if (res == nullptr)
			{
				req._invalid = true;
				continue;
			}

			LinaTexture2DBinding* binding = reinterpret_cast<LinaTexture2DBinding*>(m_renderDraw.materialBuffer.data() + req.texturePadding);
			binding->textureIndex		  = res->GetBindlessIndex();
		}

		materialBuffer.BufferData(0, m_renderDraw.materialBuffer.data(), m_renderDraw.materialBufferCounter);
		m_uploadQueue.AddBufferRequest(&currentFrame.guiIndexBuffer);
		m_uploadQueue.AddBufferRequest(&currentFrame.guiVertexBuffer);
		m_guiPass.AddBuffersToUploadQueue(frameIndex, m_uploadQueue);

		if (m_uploadQueue.FlushAll(currentFrame.copyStream))
		{
			currentFrame.copySemaphore.Increment();
			m_lgx->CloseCommandStreams(&currentFrame.copyStream, 1);
			m_lgx->SubmitCommandStreams({
				.targetQueue	  = m_lgx->GetPrimaryQueue(LinaGX::CommandType::Transfer),
				.streams		  = &currentFrame.copyStream,
				.streamCount	  = 1,
				.useSignal		  = true,
				.signalCount	  = 1,
				.signalSemaphores = currentFrame.copySemaphore.GetSemaphorePtr(),
				.signalValues	  = currentFrame.copySemaphore.GetValuePtr(),
			});
		}
	}

	void SurfaceRenderer::SyncRender()
	{
		MEMCPY(m_renderDraw.guiIndexBuffer.data(), m_cpuDraw.guiIndexBuffer.data(), m_cpuDraw.indexCounter * sizeof(LinaVG::Index));
		MEMCPY(m_renderDraw.guiVertexBuffer.data(), m_cpuDraw.guiVertexBuffer.data(), m_cpuDraw.vertexCounter * sizeof(LinaVG::Vertex));
		MEMCPY(m_renderDraw.materialBuffer.data(), m_cpuDraw.materialBuffer.data(), m_cpuDraw.materialBufferCounter);
		m_renderDraw.indexCounter		   = m_cpuDraw.indexCounter;
		m_renderDraw.vertexCounter		   = m_cpuDraw.vertexCounter;
		m_renderDraw.materialBufferCounter = m_cpuDraw.materialBufferCounter;
		m_renderDraw.requests			   = m_cpuDraw.requests;

		m_cpuDraw.indexCounter			= 0;
		m_cpuDraw.vertexCounter			= 0;
		m_cpuDraw.materialBufferCounter = 0;
		m_cpuDraw.requests.resize(0);
	}

	LinaGX::CommandStream* SurfaceRenderer::Render(uint32 frameIndex)
	{
		LINA_ASSERT(m_isVisible, "");

		auto& currentFrame = m_pfd[frameIndex];
		currentFrame.copySemaphore.ResetModified();

		UpdateBuffers(frameIndex);

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

		LinaGX::CMDBindDescriptorSets* bindGlobal = currentFrame.gfxStream->AddCommand<LinaGX::CMDBindDescriptorSets>();
		bindGlobal->descriptorSetHandles		  = currentFrame.gfxStream->EmplaceAuxMemory<uint16>(m_editor->GetApp()->GetGfxContext().GetDescriptorSetGlobal(frameIndex));
		bindGlobal->firstSet					  = 0;
		bindGlobal->setCount					  = 1;
		bindGlobal->layoutSource				  = LinaGX::DescriptorSetsLayoutSource::CustomLayout;
		bindGlobal->customLayout				  = m_editor->GetEditorRenderer().GetPipelineLayoutGlobal();

		// Barrier to Color Attachment
		LinaGX::CMDBarrier* barrierToColor	= currentFrame.gfxStream->AddCommand<LinaGX::CMDBarrier>();
		barrierToColor->srcStageFlags		= LinaGX::PSF_TopOfPipe;
		barrierToColor->dstStageFlags		= LinaGX::PSF_ColorAttachment;
		barrierToColor->textureBarrierCount = 1;
		barrierToColor->textureBarriers		= currentFrame.gfxStream->EmplaceAuxMemorySizeOnly<LinaGX::TextureBarrier>(sizeof(LinaGX::TextureBarrier));
		barrierToColor->textureBarriers[0]	= GfxHelpers::GetTextureBarrierPresent2Color(static_cast<uint32>(m_swapchain), true);

		m_guiPass.BindDescriptors(currentFrame.gfxStream, frameIndex, m_editor->GetEditorRenderer().GetPipelineLayoutGUI());

		// Begin render pass
		m_guiPass.Begin(currentFrame.gfxStream, viewport, scissors, frameIndex);

		currentFrame.guiVertexBuffer.BindVertex(currentFrame.gfxStream, static_cast<uint32>(sizeof(LinaVG::Vertex)));
		currentFrame.guiIndexBuffer.BindIndex(currentFrame.gfxStream, LinaGX::IndexType::Uint16);

		Shader* lastBound = nullptr;

		GPUEditorGUIPushConstants constants = {};

		for (const DrawRequest& request : m_renderDraw.requests)
		{
			if (request._invalid)
				continue;

			constants.materialIndex = static_cast<uint32>(request.materialOffset / sizeof(uint32));

			if (lastBound != request.shader)
			{
				lastBound = request.shader;
				lastBound->Bind(currentFrame.gfxStream, lastBound->GetGPUHandle());
			}

			LinaGX::CMDSetScissors* sc = currentFrame.gfxStream->AddCommand<LinaGX::CMDSetScissors>();
			sc->x					   = request.clip.pos.x < 0 ? 0 : static_cast<uint32>(request.clip.pos.x);
			sc->y					   = request.clip.pos.y < 0 ? 0 : static_cast<uint32>(request.clip.pos.y);
			sc->width				   = request.clip.size.x <= 0 ? static_cast<uint32>(m_window->GetSize().x) : static_cast<uint32>(request.clip.size.x);
			sc->height				   = request.clip.size.y <= 0 ? static_cast<uint32>(m_window->GetSize().y) : static_cast<uint32>(request.clip.size.y);

			LinaGX::CMDBindConstants* pc = currentFrame.gfxStream->AddCommand<LinaGX::CMDBindConstants>();
			pc->size					 = static_cast<uint32>(sizeof(GPUEditorGUIPushConstants));
			pc->stagesSize				 = 1;
			pc->stages					 = currentFrame.gfxStream->EmplaceAuxMemory(LinaGX::ShaderStage::Fragment);
			pc->data					 = currentFrame.gfxStream->EmplaceAuxMemory<GPUEditorGUIPushConstants>((uint8*)&constants, sizeof(GPUEditorGUIPushConstants));

			LinaGX::CMDDrawIndexedInstanced* draw = currentFrame.gfxStream->AddCommand<LinaGX::CMDDrawIndexedInstanced>();
			draw->instanceCount					  = 1;
			draw->indexCountPerInstance			  = request.indexCount;
			draw->startInstanceLocation			  = 0;
			draw->startIndexLocation			  = request.startIndex;
			draw->baseVertexLocation			  = request.startVertex;

#ifdef LINA_DEBUG
			// PROFILER_ADD_DRAWCALL(request.indexCount / 3, "Surface Renderer", 0);
#endif
		}

		// End render pass
		m_guiPass.End(currentFrame.gfxStream);

		// Barrier to Present
		LinaGX::CMDBarrier* barrierToPresent  = currentFrame.gfxStream->AddCommand<LinaGX::CMDBarrier>();
		barrierToPresent->srcStageFlags		  = LinaGX::PSF_ColorAttachment;
		barrierToPresent->dstStageFlags		  = LinaGX::PSF_BottomOfPipe;
		barrierToPresent->textureBarrierCount = 1;
		barrierToPresent->textureBarriers	  = currentFrame.gfxStream->EmplaceAuxMemorySizeOnly<LinaGX::TextureBarrier>(sizeof(LinaGX::TextureBarrier));
		barrierToPresent->textureBarriers[0]  = GfxHelpers::GetTextureBarrierColor2Present(static_cast<uint32>(m_swapchain), true);

		// Close
		m_lgx->CloseCommandStreams(&currentFrame.gfxStream, 1);

		return currentFrame.gfxStream;
	}

} // namespace Lina::Editor
