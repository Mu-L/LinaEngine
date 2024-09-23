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

#include "Core/Graphics/GUI/GUIRenderer.hpp"
#include "Core/Graphics/GUI/GUIBackend.hpp"
#include "Core/Graphics/GfxManager.hpp"
#include "Core/Graphics/Resource/Shader.hpp"
#include "Core/Graphics/Pipeline/RenderPass.hpp"
#include "Core/Resources/ResourceManager.hpp"
#include "Core/Graphics/Resource/Font.hpp"
#include "Core/Graphics/Resource/Texture.hpp"
#include "Core/Graphics/Resource/TextureSampler.hpp"
#include "Core/Graphics/ResourceUploadQueue.hpp"
#include "Common/Platform/LinaGXIncl.hpp"
#include "Common/Platform/LinaVGIncl.hpp"
#include "Common/System/System.hpp"
#include "Common/Profiling/Profiler.hpp"
#include "Common/Math/Math.hpp"
#include "Common/System/SystemInfo.hpp"

namespace Lina
{

#define MAX_COPY_COMMANDS 50
#define MAX_GUI_VERTICES  120000
#define MAX_GUI_INDICES	  140000

	void GUIRenderer::Create(GUIBackend* guiBackend, ResourceManagerV2* resourceManager, TextureSampler* defaultSampler, TextureSampler* textSampler, LinaGX::Window* window)
	{
		m_guiBackend = guiBackend;
		m_window	 = window;
		m_lgx		 = GfxManager::GetLGX();

		m_defaultGUISampler = defaultSampler;
		m_textGUISampler	= textSampler;

		for (uint32 i = 0; i < FRAMES_IN_FLIGHT; i++)
		{
			auto&		 data = m_pfd[i];
			const String istr = TO_STRING(i);
			data.guiVertexBuffer.Create(LinaGX::ResourceTypeHint::TH_VertexBuffer, MAX_GUI_VERTICES * sizeof(LinaVG::Vertex), "GUIRenderer: VertexBuffer" + istr);
			data.guiIndexBuffer.Create(LinaGX::ResourceTypeHint::TH_IndexBuffer, MAX_GUI_INDICES * sizeof(LinaVG::Index), "GUIRenderer: IndexBuffer" + istr);
		}

		m_lvg.GetCallbacks().drawDefault	= BIND(&GUIRenderer::DrawDefault, this, std::placeholders::_1);
		m_lvg.GetCallbacks().drawGradient	= BIND(&GUIRenderer::DrawGradient, this, std::placeholders::_1);
		m_lvg.GetCallbacks().drawTextured	= BIND(&GUIRenderer::DrawTextured, this, std::placeholders::_1);
		m_lvg.GetCallbacks().drawSimpleText = BIND(&GUIRenderer::DrawSimpleText, this, std::placeholders::_1);
		m_lvg.GetCallbacks().drawSDFText	= BIND(&GUIRenderer::DrawSDFText, this, std::placeholders::_1);

		m_widgetManager.Initialize(resourceManager, window, &m_lvg);
	}

	void GUIRenderer::PreTick()
	{
		PROFILER_FUNCTION();
		m_widgetManager.PreTick();
	}

	void GUIRenderer::Tick(float delta, const Vector2ui& size)
	{
		PROFILER_FUNCTION();
		m_widgetManager.Tick(SystemInfo::GetDeltaTimeF(), size);
	}

	void GUIRenderer::DrawDefault(LinaVG::DrawBuffer* buf)
	{
		auto& req = AddDrawRequest(buf);
	}

	void GUIRenderer::DrawGradient(LinaVG::GradientDrawBuffer* buf)
	{
		auto& req				= AddDrawRequest(buf);
		req.materialData.color1 = buf->m_color.start;
		req.materialData.color2 = buf->m_color.end;

		if (buf->m_color.gradientType == LinaVG::GradientType::Horizontal)
		{
			req.materialData.floatPack1 = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
			req.materialData.floatPack2 = Vector4(0.0f, buf->m_isAABuffer ? 1.0f : 0.0f, buf->m_color.radialSize, static_cast<float>(buf->m_drawBufferType));
		}
		else if (buf->m_color.gradientType == LinaVG::GradientType::Vertical)
		{
			req.materialData.floatPack1 = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
			req.materialData.floatPack2 = Vector4(0.0f, buf->m_isAABuffer ? 1.0f : 0.0f, buf->m_color.radialSize, static_cast<float>(buf->m_drawBufferType));
		}
		else if (buf->m_color.gradientType == LinaVG::GradientType::Radial)
		{
			req.materialData.floatPack1 = Vector4(0.0f, 0.0f, 0.5f, 0.5f);
			req.materialData.floatPack2 = Vector4(1.0f, buf->m_isAABuffer ? 1.0f : 0.0f, buf->m_color.radialSize, static_cast<float>(buf->m_drawBufferType));
		}
		else if (buf->m_color.gradientType == LinaVG::GradientType::RadialCorner)
		{
			req.materialData.floatPack1 = Vector4(0.0f, 0.0f, 0.5f, 0.5f);
			req.materialData.floatPack2 = Vector4(1.0f, buf->m_isAABuffer ? 1.0f : 0.0f, buf->m_color.radialSize, static_cast<float>(buf->m_drawBufferType));
		}
	}

	void GUIRenderer::DrawTextured(LinaVG::TextureDrawBuffer* buf)
	{
		auto& req				= AddDrawRequest(buf);
		float drawBufferType	= static_cast<float>(buf->m_drawBufferType);
		req.materialData.color1 = buf->m_tint;
		float singleChannel		= 0.0f;

		if (buf->m_textureHandle == GUI_TEXTURE_HUE_HORIZONTAL)
		{
			drawBufferType = 5.0f; // special case :)
		}
		else if (buf->m_textureHandle == GUI_TEXTURE_HUE_VERTICAL)
		{
			drawBufferType = 6.0f; // special case :)
		}
		else if (buf->m_textureHandle == GUI_TEXTURE_COLORWHEEL)
		{
			drawBufferType = 7.0f; // special case :)
		}
		else
		{
			req.materialData.color2.x = static_cast<float>(buf->m_textureHandle - 1);
			req.materialData.color2.y = static_cast<float>(m_defaultGUISampler->GetBindlessIndex());
		}

		if (Math::Equals(req.materialData.color1.w, GUI_IS_SINGLE_CHANNEL, 0.01f))
			singleChannel = 1.0f;

		DisplayChannels displayChannels = DisplayChannels::RGBA;
		float			mipLevel		= 0.0f;

		if (buf->userData != nullptr)
		{
			GUIRendererUserData* ud = static_cast<GUIRendererUserData*>(buf->userData);
			displayChannels			= ud->displayChannels;
			mipLevel				= static_cast<float>(ud->mipLevel);
		}

		req.materialData.color2.z	= mipLevel;
		req.materialData.floatPack1 = Vector4(buf->m_textureUVTiling.x, buf->m_textureUVTiling.y, buf->m_textureUVOffset.x, buf->m_textureUVOffset.y);
		req.materialData.floatPack2 = Vector4(buf->m_isAABuffer, singleChannel, static_cast<uint32>(displayChannels), drawBufferType);
	}

	void GUIRenderer::DrawSimpleText(LinaVG::SimpleTextDrawBuffer* buf)
	{
		auto& req					  = AddDrawRequest(buf);
		auto  txt					  = m_guiBackend->GetFontTexture(buf->m_font->m_atlas).texture;
		req.materialData.floatPack2.w = static_cast<float>(buf->m_drawBufferType);
		req.materialData.color2.x	  = static_cast<float>(txt->GetBindlessIndex());
		req.materialData.color2.y	  = static_cast<float>(m_textGUISampler->GetBindlessIndex());
	}

	void GUIRenderer::DrawSDFText(LinaVG::SDFTextDrawBuffer* buf)
	{
		auto& req					= AddDrawRequest(buf);
		auto  txt					= m_guiBackend->GetFontTexture(buf->m_font->m_atlas).texture;
		req.materialData.color1		= buf->m_outlineColor;
		req.materialData.floatPack1 = Vector4(buf->m_thickness, buf->m_softness, buf->m_outlineThickness, buf->m_outlineSoftness);
		req.materialData.floatPack2 = Vector4(buf->m_flipAlpha ? 1.0f : 0.0f, 0.0f, 0.0f, static_cast<float>(buf->m_drawBufferType));
		req.materialData.color2.x	= static_cast<float>(txt->GetBindlessIndex());
		req.materialData.color2.y	= static_cast<float>(m_textGUISampler->GetBindlessIndex());
	}

	void GUIRenderer::Destroy()
	{
		m_widgetManager.Shutdown();
		for (uint32 i = 0; i < FRAMES_IN_FLIGHT; i++)
		{
			auto& data = m_pfd[i];
			data.guiVertexBuffer.Destroy();
			data.guiIndexBuffer.Destroy();
		}
	}

	GUIRenderer::DrawRequest& GUIRenderer::AddDrawRequest(LinaVG::DrawBuffer* buf)
	{
		const uint32 frame = GfxManager::GetLGX()->GetCurrentFrameIndex();
		auto&		 pfd   = m_pfd[frame];
		m_drawRequests.push_back({});
		auto& req		 = m_drawRequests.back();
		req.indexCount	 = static_cast<uint32>(buf->m_indexBuffer.m_size);
		req.vertexOffset = pfd.vertexCounter;
		req.firstIndex	 = pfd.indexCounter;
		pfd.guiVertexBuffer.BufferData(pfd.vertexCounter * sizeof(LinaVG::Vertex), (uint8*)buf->m_vertexBuffer.m_data, buf->m_vertexBuffer.m_size * sizeof(LinaVG::Vertex));
		pfd.guiIndexBuffer.BufferData(pfd.indexCounter * sizeof(LinaVG::Index), (uint8*)buf->m_indexBuffer.m_data, buf->m_indexBuffer.m_size * sizeof(LinaVG::Index));
		pfd.indexCounter += req.indexCount;
		pfd.vertexCounter += static_cast<uint32>(buf->m_vertexBuffer.m_size);
		req.materialData.clip = Vector4(buf->clipPosX, buf->clipPosY, buf->clipSizeX, buf->clipSizeY);
		return req;
	}

	const Vector<GUIRenderer::DrawRequest>& GUIRenderer::FlushGUI(uint32 frameIndex, size_t indirectBufferOffset, const Vector2ui& canvasSize)
	{
		auto& pfd = m_pfd[frameIndex];
		m_widgetManager.Draw();
		m_lvg.FlushBuffers();
		m_lvg.ResetFrame();
		pfd.vertexCounter = pfd.indexCounter = 0;
		m_indirectBufferOffset				 = indirectBufferOffset;

		for (auto& req : m_drawRequests)
			req.materialData.canvasSize = Vector2(static_cast<float>(canvasSize.x), static_cast<float>(canvasSize.y));

		return m_drawRequests;
	}

	bool GUIRenderer::CopyVertexIndex(uint32 frameIndex, LinaGX::CommandStream* copyStream)
	{
		auto& pfd		 = m_pfd[frameIndex];
		uint8 copyExists = 0;
		copyExists |= pfd.guiIndexBuffer.Copy(copyStream);
		copyExists |= pfd.guiVertexBuffer.Copy(copyStream);
		return copyExists == 0 ? false : true;
	}

	void GUIRenderer::AddBuffersToUploadQueue(uint32 frameIndex, ResourceUploadQueue& queue)
	{
		auto& pfd = m_pfd[frameIndex];
		queue.AddBufferRequest(&pfd.guiIndexBuffer);
		queue.AddBufferRequest(&pfd.guiVertexBuffer);
	}

	void GUIRenderer::Render(LinaGX::CommandStream* stream, const Buffer& indirectBuffer, uint32 frameIndex, const Vector2ui& size)
	{
		auto& pfd = m_pfd[frameIndex];
		pfd.guiVertexBuffer.BindVertex(stream, static_cast<uint32>(sizeof(LinaVG::Vertex)));
		pfd.guiIndexBuffer.BindIndex(stream, LinaGX::IndexType::Uint16);
		LinaGX::CMDDrawIndexedIndirect* draw = stream->AddCommand<LinaGX::CMDDrawIndexedIndirect>();
		draw->count							 = static_cast<uint32>(m_drawRequests.size());
		draw->indirectBuffer				 = indirectBuffer.GetGPUResource();
		draw->indirectBufferOffset			 = static_cast<uint32>(m_indirectBufferOffset);
		PROFILER_ADD_DRAWCALL(m_totalIndices / 3, "GUI Indirect", draw->count);
		m_drawRequests.clear();
	}

} // namespace Lina
